#include "webrtc/whep_server.h"

#include <random>
#include <sstream>

#include "core/logger.h"
#include "webrtc/dtls_context.h"
#include "webrtc/stun.h"

namespace csk {

WhepServer::WhepServer(asio::io_context &io, MediaHub &hub, uint16_t udp_port)
    : io_(io), hub_(hub), socket_(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), udp_port)) {
    udp_port_ = socket_.local_endpoint().port();
}

WhepServer::~WhepServer() { stop(); }

void WhepServer::start() {
    do_receive();
    CSK_LOG_I("WHEP/WebRTC UDP listening on port {}", udp_port_);
}

void WhepServer::stop() {
    asio::error_code ec;
    socket_.close(ec);
    std::lock_guard<std::mutex> lk(sessions_mutex_);
    for (auto &[id, s] : sessions_) s->stop();
    sessions_.clear();
    by_ufrag_.clear();
}

void WhepServer::do_receive() {
    socket_.async_receive_from(
        asio::buffer(recv_buf_), recv_ep_,
        [this](asio::error_code ec, size_t bytes) {
            if (ec) return;

            // Route by STUN username (local_ufrag:remote_ufrag)
            const uint8_t *data = recv_buf_.data();

            std::shared_ptr<WebRtcSession> session;
            if (StunMessage::is_stun(data, bytes) && bytes > 20) {
                // Extract USERNAME attribute to find session
                // Simple approach: try all sessions (small count expected)
                std::lock_guard<std::mutex> lk(sessions_mutex_);
                for (auto &[ufrag, s] : by_ufrag_) {
                    session = s;
                    break;  // For ICE-lite, first candidate that matches
                }
                // Better: parse USERNAME from STUN
                // USERNAME format: local_ufrag:remote_ufrag
                size_t pos = 20;
                while (pos + 4 <= bytes) {
                    uint16_t attr_type = (uint16_t)((data[pos] << 8) | data[pos + 1]);
                    uint16_t attr_len = (uint16_t)((data[pos + 2] << 8) | data[pos + 3]);
                    if (attr_type == 0x0006 && attr_len > 0) {  // USERNAME
                        std::string username((char *)data + pos + 4, attr_len);
                        auto colon = username.find(':');
                        if (colon != std::string::npos) {
                            std::string local_ufrag = username.substr(0, colon);
                            auto it = by_ufrag_.find(local_ufrag);
                            if (it != by_ufrag_.end()) {
                                session = it->second;
                            }
                        }
                        break;
                    }
                    pos += 4 + attr_len;
                    pos += (4 - (attr_len % 4)) % 4;  // padding
                }
            } else {
                // DTLS/RTP - route by remote endpoint
                std::lock_guard<std::mutex> lk(sessions_mutex_);
                for (auto &[id, s] : sessions_) {
                    session = s;  // Single session routing for now
                    break;
                }
            }

            if (session) {
                session->on_udp_data(data, bytes, recv_ep_);
            }

            do_receive();
        });
}

WhepAnswer WhepServer::handle_offer(const std::string &stream_id, const std::string &offer_sdp) {
    auto source = hub_.find(stream_id);
    if (!source) {
        CSK_LOG_W("WHEP: stream not found: {}", stream_id);
        return {};
    }

    WebRtcSessionConfig config;
    config.stream_id = stream_id;
    config.local_ufrag = generate_id().substr(0, 8);
    config.local_pwd = generate_id() + generate_id();
    config.remote_ufrag = parse_remote_ufrag(offer_sdp);
    config.remote_pwd = parse_remote_pwd(offer_sdp);

    std::random_device rd;
    config.ssrc = rd();

    auto session = std::make_shared<WebRtcSession>(io_, config);
    session->set_udp_sender([this](const uint8_t *data, size_t len,
                                   const asio::ip::udp::endpoint &ep) {
        socket_.async_send_to(asio::buffer(data, len), ep,
                              [](asio::error_code, size_t) {});
    });

    std::string resource_id = generate_id();

    {
        std::lock_guard<std::mutex> lk(sessions_mutex_);
        sessions_[resource_id] = session;
        by_ufrag_[config.local_ufrag] = session;
    }

    // Subscribe to media source
    std::shared_ptr<IMediaSink> sink_ptr = std::static_pointer_cast<IMediaSink>(session);
    source->add_subscriber(sink_ptr);
    session->start(asio::ip::udp::endpoint());

    std::string answer = build_answer_sdp(config, udp_port_);

    WhepAnswer result;
    result.sdp = answer;
    result.resource_id = resource_id;

    CSK_LOG_I("WHEP: created session {} for stream {}", resource_id, stream_id);
    return result;
}

bool WhepServer::handle_delete(const std::string &resource_id) {
    std::lock_guard<std::mutex> lk(sessions_mutex_);
    auto it = sessions_.find(resource_id);
    if (it == sessions_.end()) return false;

    auto session = it->second;
    by_ufrag_.erase(session->local_ufrag());

    // Unsubscribe from media source
    auto source = hub_.find(session->stream_id());
    if (source) source->remove_subscriber(static_cast<IMediaSink *>(session.get()));

    session->stop();
    sessions_.erase(it);
    return true;
}

std::string WhepServer::generate_id() {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(chars) - 2);
    std::string id;
    for (int i = 0; i < 12; i++) id += chars[dist(gen)];
    return id;
}

std::string WhepServer::build_answer_sdp(const WebRtcSessionConfig &config, uint16_t port) {
    auto &dtls_ctx = DtlsContext::instance();
    std::ostringstream ss;

    ss << "v=0\r\n"
       << "o=- 0 0 IN IP4 0.0.0.0\r\n"
       << "s=-\r\n"
       << "t=0 0\r\n"
       << "a=group:BUNDLE 0\r\n"
       << "a=ice-lite\r\n"
       << "m=video " << port << " UDP/TLS/RTP/SAVPF 96\r\n"
       << "c=IN IP4 0.0.0.0\r\n"
       << "a=rtpmap:96 H264/90000\r\n"
       << "a=fmtp:96 packetization-mode=1\r\n"
       << "a=sendonly\r\n"
       << "a=mid:0\r\n"
       << "a=ice-ufrag:" << config.local_ufrag << "\r\n"
       << "a=ice-pwd:" << config.local_pwd << "\r\n"
       << "a=fingerprint:sha-256 " << dtls_ctx.fingerprint() << "\r\n"
       << "a=setup:passive\r\n"
       << "a=rtcp-mux\r\n"
       << "a=ssrc:" << config.ssrc << " cname:camstreamkit\r\n";

    return ss.str();
}

std::string WhepServer::parse_remote_ufrag(const std::string &sdp) {
    auto pos = sdp.find("a=ice-ufrag:");
    if (pos == std::string::npos) return "";
    pos += 12;
    auto end = sdp.find("\r\n", pos);
    if (end == std::string::npos) end = sdp.find("\n", pos);
    return sdp.substr(pos, end - pos);
}

std::string WhepServer::parse_remote_pwd(const std::string &sdp) {
    auto pos = sdp.find("a=ice-pwd:");
    if (pos == std::string::npos) return "";
    pos += 10;
    auto end = sdp.find("\r\n", pos);
    if (end == std::string::npos) end = sdp.find("\n", pos);
    return sdp.substr(pos, end - pos);
}

}  // namespace csk
