#include "webrtc/webrtc_session.h"

#include <openssl/err.h>

#include <cstring>
#include <random>

#include "core/logger.h"
#include "webrtc/dtls_context.h"
#include "webrtc/stun.h"

namespace csk {

WebRtcSession::WebRtcSession(asio::io_context &io, const WebRtcSessionConfig &config)
    : config_(config), io_(io), packetizer_(config.payload_type, config.ssrc) {
    timestamp_ = 0;
}

WebRtcSession::~WebRtcSession() {
    stop();
}

void WebRtcSession::start(asio::ip::udp::endpoint remote_ep) {
    remote_ep_ = remote_ep;
    state_ = WebRtcState::CHECKING;
    CSK_LOG_I("WebRTC session started for stream: {}", config_.stream_id);
}

void WebRtcSession::stop() {
    if (state_ == WebRtcState::CLOSED) return;
    state_ = WebRtcState::CLOSED;
    if (ssl_) {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = nullptr;
    }
    CSK_LOG_I("WebRTC session closed for stream: {}", config_.stream_id);
}

void WebRtcSession::on_udp_data(const uint8_t *data, size_t len,
                                 const asio::ip::udp::endpoint &from) {
    if (len == 0) return;

    if (StunMessage::is_stun(data, len)) {
        handle_stun(data, len, from);
    } else if (len > 0 && (data[0] >= 20 && data[0] <= 63)) {
        // DTLS
        if (ssl_ && read_bio_) {
            BIO_write(read_bio_, data, (int)len);
            if (state_ == WebRtcState::DTLS_HANDSHAKING) {
                do_dtls_handshake();
            }
        }
    }
}

void WebRtcSession::handle_stun(const uint8_t *data, size_t len,
                                 const asio::ip::udp::endpoint &from) {
    auto msg = StunMessage::parse(SpanU8(data, len));

    if (msg.type == StunMessage::BINDING_REQUEST) {
        remote_ep_ = from;

        uint32_t addr = from.address().to_v4().to_uint();
        uint16_t port = from.port();

        auto resp = msg.build_success_response(config_.local_pwd, addr, port);
        if (udp_send_) {
            udp_send_(resp.data(), resp.size(), from);
        }

        if (state_ == WebRtcState::CHECKING) {
            state_ = WebRtcState::CONNECTED;
            CSK_LOG_I("ICE connected from {}:{}", from.address().to_string(), from.port());
            start_dtls();
        }
    }
}

void WebRtcSession::start_dtls() {
    auto &ctx = DtlsContext::instance();
    ssl_ = SSL_new(ctx.ssl_ctx());

    read_bio_ = BIO_new(BIO_s_mem());
    write_bio_ = BIO_new(BIO_s_mem());
    BIO_set_mem_eof_return(read_bio_, -1);
    BIO_set_mem_eof_return(write_bio_, -1);

    SSL_set_bio(ssl_, read_bio_, write_bio_);
    SSL_set_accept_state(ssl_);

    state_ = WebRtcState::DTLS_HANDSHAKING;
    do_dtls_handshake();
}

void WebRtcSession::do_dtls_handshake() {
    int ret = SSL_do_handshake(ssl_);

    // Flush any outgoing data
    char buf[4096];
    int pending;
    while ((pending = BIO_read(write_bio_, buf, sizeof(buf))) > 0) {
        if (udp_send_) {
            udp_send_(reinterpret_cast<uint8_t *>(buf), (size_t)pending, remote_ep_);
        }
    }

    if (ret == 1) {
        on_dtls_connected();
    } else {
        int err = SSL_get_error(ssl_, ret);
        if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE) {
            CSK_LOG_E("DTLS handshake failed: {}", ERR_error_string(ERR_get_error(), nullptr));
            stop();
        }
    }
}

void WebRtcSession::on_dtls_connected() {
    CSK_LOG_I("DTLS handshake complete for stream: {}", config_.stream_id);

    // Extract SRTP keying material
    uint8_t material[60];
    static const char *label = "EXTRACTOR-dtls_srtp";
    if (SSL_export_keying_material(ssl_, material, sizeof(material),
                                    label, strlen(label), nullptr, 0, 0) != 1) {
        CSK_LOG_E("Failed to export SRTP keying material");
        stop();
        return;
    }

    if (!srtp_.init(material, sizeof(material))) {
        CSK_LOG_E("Failed to init SRTP session");
        stop();
        return;
    }

    state_ = WebRtcState::READY;
    CSK_LOG_I("WebRTC session READY for stream: {}", config_.stream_id);
}

void WebRtcSession::on_media_frame(const MediaFrame &frame) {
    if (state_ != WebRtcState::READY) return;
    if (!frame.data || frame.data->empty()) return;

    auto nals = h264_parser_.parse(SpanU8(frame.data->data(), frame.data->size()));
    if (nals.empty()) return;

    // Always cache SPS/PPS regardless of keyframe state
    for (auto &nal : nals) {
        if (nal.is_sps()) cached_sps_ = nal.data;
        else if (nal.is_pps()) cached_pps_ = nal.data;
    }

    // Skip parameter-set-only frames (SPS/PPS) - don't send them standalone.
    // They'll be sent together with their IDR.
    bool has_slice = false;
    for (auto &nal : nals) {
        if (nal.is_idr() || nal.is_slice()) { has_slice = true; break; }
    }
    if (!has_slice) return;

    // Wait for IDR before sending anything
    if (!got_keyframe_) {
        bool has_idr = false;
        for (auto &nal : nals) {
            if (nal.is_idr()) { has_idr = true; break; }
        }
        if (!has_idr) return;
        got_keyframe_ = true;
        CSK_LOG_I("WebRTC[{}]: Got first keyframe, SPS={}B PPS={}B, sending IDR",
                  config_.stream_id, cached_sps_.size(), cached_pps_.size());

        // Prepend cached SPS/PPS before the IDR
        std::vector<NalUnit> send_nals;
        if (!cached_sps_.empty()) {
            NalUnit sps;
            sps.data = cached_sps_;
            sps.type = 7;
            send_nals.push_back(std::move(sps));
        }
        if (!cached_pps_.empty()) {
            NalUnit pps;
            pps.data = cached_pps_;
            pps.type = 8;
            send_nals.push_back(std::move(pps));
        }
        for (auto &nal : nals) {
            send_nals.push_back(std::move(nal));
        }

        // Use source timestamp directly (already 90kHz from RTSP)
        timestamp_ = frame.timestamp;
        send_nals_as_rtp(send_nals);
        return;
    }

    // For subsequent IDR frames, also prepend SPS/PPS
    bool has_idr = false;
    for (auto &nal : nals) {
        if (nal.is_idr()) { has_idr = true; break; }
    }

    // Use source timestamp (90kHz). If source ts is 0 or non-monotonic, generate our own.
    if (frame.timestamp > 0) {
        timestamp_ = frame.timestamp;
    } else {
        timestamp_ += 3000;
    }

    if (has_idr) {
        std::vector<NalUnit> send_nals;
        if (!cached_sps_.empty()) {
            NalUnit sps;
            sps.data = cached_sps_;
            sps.type = 7;
            send_nals.push_back(std::move(sps));
        }
        if (!cached_pps_.empty()) {
            NalUnit pps;
            pps.data = cached_pps_;
            pps.type = 8;
            send_nals.push_back(std::move(pps));
        }
        for (auto &nal : nals) {
            send_nals.push_back(std::move(nal));
        }
        send_nals_as_rtp(send_nals);
    } else {
        send_nals_as_rtp(nals);
    }
}

void WebRtcSession::send_nals_as_rtp(const std::vector<NalUnit> &nals) {
    static uint64_t frame_count = 0;
    if (++frame_count <= 5 || frame_count % 100 == 0) {
        std::string nal_info;
        for (auto &n : nals) {
            nal_info += " type=" + std::to_string(n.type) + "(" + std::to_string(n.data.size()) + "B)";
        }
        CSK_LOG_I("WebRTC[{}]: send frame#{} ts={} nals:[{}]",
                  config_.stream_id, frame_count, timestamp_, nal_info);
    }
    for (size_t i = 0; i < nals.size(); ++i) {
        bool is_last_nal = (i == nals.size() - 1);
        auto packets = packetizer_.packetize(nals[i], timestamp_);

        // Only set marker on the very last packet of the last NAL in this access unit
        if (!packets.empty() && !is_last_nal) {
            packets.back().header().marker = false;
        }

        for (auto &pkt : packets) {
            auto serialized = pkt.serialize();
            send_srtp_packet(serialized);
        }
    }
}

void WebRtcSession::send_srtp_packet(std::vector<uint8_t> &rtp_packet) {
    if (!srtp_.protect_rtp(rtp_packet)) return;
    if (udp_send_) {
        udp_send_(rtp_packet.data(), rtp_packet.size(), remote_ep_);
    }
}

}  // namespace csk
