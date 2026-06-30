#include "rtsp/rtsp_client.h"

#include <sstream>

#include "core/logger.h"

namespace csk {

RtspClient::RtspClient(asio::io_context &io, Config config)
    : io_(io), config_(std::move(config)), socket_(io) {
    auto url_opt = RtspParser::parse_url(config_.url);
    if (url_opt) {
        url_info_ = *url_opt;
    }
}

RtspClient::~RtspClient() { stop(); }

void RtspClient::connect() {
    do_connect();
}

void RtspClient::do_connect() {
    asio::ip::tcp::resolver resolver(io_);
    auto endpoints = resolver.resolve(url_info_.host, std::to_string(url_info_.port));

    asio::async_connect(socket_, endpoints,
                        [this, self = shared_from_this()](asio::error_code ec, auto) {
                            if (!ec) {
                                connected_ = true;
                                CSK_LOG_I("RTSP connected to {}", config_.url);
                                send_options();
                                do_read();
                            } else {
                                CSK_LOG_E("RTSP connect failed: {}", ec.message());
                            }
                        });
}

void RtspClient::send_options() {
    std::ostringstream ss;
    ss << "OPTIONS " << config_.url << " RTSP/1.0\r\n"
       << "CSeq: " << ++cseq_ << "\r\n"
       << "User-Agent: CamStreamKit/1.0\r\n"
       << "\r\n";

    auto data = std::make_shared<std::string>(ss.str());
    asio::async_write(socket_, asio::buffer(*data),
                      [data](asio::error_code, size_t) {});
}

void RtspClient::send_describe() {
    std::ostringstream ss;
    ss << "DESCRIBE " << config_.url << " RTSP/1.0\r\n"
       << "CSeq: " << ++cseq_ << "\r\n"
       << "Accept: application/sdp\r\n"
       << "User-Agent: CamStreamKit/1.0\r\n"
       << "\r\n";

    auto data = std::make_shared<std::string>(ss.str());
    asio::async_write(socket_, asio::buffer(*data),
                      [data](asio::error_code, size_t) {});
}

void RtspClient::send_setup() {
    std::string control_url = config_.url;
    if (!content_base_.empty()) {
        control_url = content_base_ + "trackID=0";
    }

    std::ostringstream ss;
    ss << "SETUP " << control_url << " RTSP/1.0\r\n"
       << "CSeq: " << ++cseq_ << "\r\n";

    if (config_.transport == TransportMode::TCP) {
        ss << "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n";
    } else {
        ss << "Transport: RTP/AVP;unicast;client_port=5000-5001\r\n";
    }

    if (!session_id_.empty()) {
        ss << "Session: " << session_id_ << "\r\n";
    }
    ss << "User-Agent: CamStreamKit/1.0\r\n"
       << "\r\n";

    auto data = std::make_shared<std::string>(ss.str());
    asio::async_write(socket_, asio::buffer(*data),
                      [data](asio::error_code, size_t) {});
}

void RtspClient::send_play() {
    std::ostringstream ss;
    ss << "PLAY " << config_.url << " RTSP/1.0\r\n"
       << "CSeq: " << ++cseq_ << "\r\n"
       << "Session: " << session_id_ << "\r\n"
       << "Range: npt=0.000-\r\n"
       << "User-Agent: CamStreamKit/1.0\r\n"
       << "\r\n";

    auto data = std::make_shared<std::string>(ss.str());
    asio::async_write(socket_, asio::buffer(*data),
                      [data](asio::error_code, size_t) {});
}

void RtspClient::do_read() {
    socket_.async_read_some(
        asio::buffer(read_buf_),
        [this, self = shared_from_this()](asio::error_code ec, size_t bytes) {
            if (ec) {
                CSK_LOG_W("RTSP read error: {}", ec.message());
                connected_ = false;
                return;
            }

            recv_buffer_.append(reinterpret_cast<const char *>(read_buf_.data()), bytes);

            // Process interleaved RTP data ($ + channel + length + data)
            while (recv_buffer_.size() >= 4 && recv_buffer_[0] == '$') {
                uint8_t channel = static_cast<uint8_t>(recv_buffer_[1]);
                uint16_t length = (static_cast<uint16_t>(
                                       static_cast<uint8_t>(recv_buffer_[2])) << 8) |
                                  static_cast<uint8_t>(recv_buffer_[3]);

                if (recv_buffer_.size() < 4u + length) break;

                handle_interleaved_rtp(
                    channel,
                    SpanU8(reinterpret_cast<const uint8_t *>(recv_buffer_.data() + 4), length));
                recv_buffer_.erase(0, 4 + length);
            }

            // Process RTSP responses
            if (!recv_buffer_.empty() && recv_buffer_[0] != '$') {
                handle_response(recv_buffer_);
            }

            do_read();
        });
}

void RtspClient::handle_response(const std::string &data) {
    auto header_end = recv_buffer_.find("\r\n\r\n");
    if (header_end == std::string::npos) return;

    // Determine content-length
    size_t content_length = 0;
    auto cl_pos = recv_buffer_.find("Content-Length:");
    if (cl_pos != std::string::npos && cl_pos < header_end) {
        auto val_start = cl_pos + 15;
        while (val_start < recv_buffer_.size() && recv_buffer_[val_start] == ' ') val_start++;
        auto val_end = recv_buffer_.find("\r\n", val_start);
        content_length = std::stoul(recv_buffer_.substr(val_start, val_end - val_start));
    }

    size_t total = header_end + 4 + content_length;
    if (recv_buffer_.size() < total) return;

    std::string response = recv_buffer_.substr(0, total);
    recv_buffer_.erase(0, total);

    // Extract status code
    auto first_line_end = response.find("\r\n");
    std::string first_line = response.substr(0, first_line_end);

    // Parse session header
    auto session_pos = response.find("Session:");
    if (session_pos != std::string::npos) {
        auto val_start = session_pos + 8;
        while (val_start < response.size() && response[val_start] == ' ') val_start++;
        auto val_end = response.find_first_of(";\r\n", val_start);
        session_id_ = response.substr(val_start, val_end - val_start);
    }

    // Parse Content-Base
    auto cb_pos = response.find("Content-Base:");
    if (cb_pos != std::string::npos) {
        auto val_start = cb_pos + 13;
        while (val_start < response.size() && response[val_start] == ' ') val_start++;
        auto val_end = response.find("\r\n", val_start);
        content_base_ = response.substr(val_start, val_end - val_start);
    }

    // State machine: OPTIONS -> DESCRIBE -> SETUP -> PLAY
    if (first_line.find("200") != std::string::npos) {
        if (response.find("Public:") != std::string::npos) {
            send_describe();
        } else if (response.find("application/sdp") != std::string::npos) {
            send_setup();
        } else if (!session_id_.empty() && !playing_) {
            send_play();
            playing_ = true;
            CSK_LOG_I("RTSP playing: {}", config_.url);
        }
    }
}

void RtspClient::handle_interleaved_rtp(uint8_t channel, SpanU8 data) {
    if (channel == 0 && data.size() >= 12) {
        on_rtp_packet(data);
    }
}

void RtspClient::on_rtp_packet(SpanU8 data) {
    auto pkt = RtpPacket::parse(data);
    if (!pkt) return;

    auto nal = h264_parser_.depacketize_rtp(pkt->payload());
    if (!nal) return;

    MediaFrame frame;
    frame.codec = CodecType::H264;
    frame.timestamp = pkt->header().timestamp;
    frame.type = nal->is_key_frame() ? MediaFrame::VIDEO_KEY : MediaFrame::VIDEO_P;

    // Prepend start code for Annex-B format
    std::vector<uint8_t> annex_b;
    annex_b.reserve(4 + nal->data.size());
    annex_b.push_back(0x00);
    annex_b.push_back(0x00);
    annex_b.push_back(0x00);
    annex_b.push_back(0x01);
    annex_b.insert(annex_b.end(), nal->data.begin(), nal->data.end());

    frame.data = make_buffer(std::move(annex_b));

    if (frame_cb_) {
        frame_cb_(frame);
    }
}

void RtspClient::play() {
    if (connected_ && !playing_) {
        send_play();
    }
}

void RtspClient::stop() {
    playing_ = false;
    connected_ = false;
    asio::error_code ec;
    socket_.close(ec);
}

// --- RtspMediaSource ---

RtspMediaSource::RtspMediaSource(const std::string &id, const RtspClient::Config &config,
                                 asio::io_context &io)
    : id_(id), io_(io), client_config_(config), reconnect_(1000, 30000) {}

RtspMediaSource::~RtspMediaSource() { stop(); }

void RtspMediaSource::start() { start_pull(); }

void RtspMediaSource::stop() {
    pulling_ = false;
    if (client_) {
        client_->stop();
        client_.reset();
    }
    if (idle_timer_) idle_timer_->cancel();
    if (reconnect_timer_) reconnect_timer_->cancel();
    status_ = "offline";
}

std::string RtspMediaSource::generate_sdp(const std::string &url) {
    std::string sprop;
    // TODO: encode sps/pps to base64 for SDP
    return SdpParser::build_video_sdp("0", "0.0.0.0",
                                       codec_ == CodecType::H265 ? "H265" : "H264",
                                       96, sprop);
}

void RtspMediaSource::on_first_subscriber() {
    if (!pulling_) {
        start_pull();
    }
    if (idle_timer_) {
        idle_timer_->cancel();
        idle_timer_.reset();
    }
}

void RtspMediaSource::on_no_subscribers() {
    idle_timer_ = std::make_shared<asio::steady_timer>(io_);
    idle_timer_->expires_after(std::chrono::seconds(10));
    idle_timer_->async_wait([this](asio::error_code ec) {
        if (!ec && subscriber_count() == 0) {
            CSK_LOG_I("Source {} idle timeout, stopping pull", id_);
            stop_pull();
        }
    });
}

void RtspMediaSource::start_pull() {
    pulling_ = true;
    status_ = "connecting";

    client_ = std::make_shared<RtspClient>(io_, client_config_);
    client_->set_frame_callback([this](const MediaFrame &frame) {
        on_frame(frame);
    });
    client_->connect();

    reconnect_.reset();
    status_ = "online";
}

void RtspMediaSource::stop_pull() {
    pulling_ = false;
    if (client_) {
        client_->stop();
        client_.reset();
    }
    status_ = "idle";
}

void RtspMediaSource::schedule_reconnect() {
    if (!pulling_) return;
    status_ = "reconnecting";
    int delay = reconnect_.next_delay_ms();
    CSK_LOG_W("Source {} disconnected, retry in {}ms", id_, delay);

    reconnect_timer_ = std::make_shared<asio::steady_timer>(io_);
    reconnect_timer_->expires_after(std::chrono::milliseconds(delay));
    reconnect_timer_->async_wait([this](asio::error_code ec) {
        if (!ec && pulling_) {
            start_pull();
        }
    });
}

void RtspMediaSource::on_frame(const MediaFrame &frame) {
    // Extract SPS/PPS for SDP generation
    if (frame.data && frame.codec == CodecType::H264) {
        auto span = frame.data->span();
        H264Parser parser;
        auto nals = parser.parse(span);
        for (auto &nal : nals) {
            if (nal.is_sps()) sps_ = nal.data;
            if (nal.is_pps()) pps_ = nal.data;
        }
    }
    dispatch_frame(frame);
}

}  // namespace csk
