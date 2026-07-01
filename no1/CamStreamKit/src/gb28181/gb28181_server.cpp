#include "gb28181/gb28181_server.h"

#include <chrono>
#include <random>
#include <sstream>

#include "core/logger.h"

namespace csk {

// --- Gb28181MediaSource ---

Gb28181MediaSource::Gb28181MediaSource(const std::string &id,
                                       const std::string &device_id,
                                       asio::io_context &io)
    : id_(id), device_id_(device_id), io_(io) {
    demuxer_.set_callback([this](const MediaFrame &f) { on_frame(f); });
}

Gb28181MediaSource::~Gb28181MediaSource() { stop(); }

std::string Gb28181MediaSource::generate_sdp(const std::string &) {
    std::ostringstream ss;
    ss << "v=0\r\n"
       << "o=- 0 0 IN IP4 0.0.0.0\r\n"
       << "s=" << id_ << "\r\n"
       << "c=IN IP4 0.0.0.0\r\n"
       << "t=0 0\r\n"
       << "m=video 0 RTP/AVP 96\r\n"
       << "a=rtpmap:96 " << (codec_ == CodecType::H265 ? "H265" : "H264") << "/90000\r\n";
    return ss.str();
}

void Gb28181MediaSource::on_rtp_data(const uint8_t *data, size_t size) {
    if (size < 12) return;

    // Skip RTP header
    uint8_t cc = data[0] & 0x0F;
    size_t header_size = 12 + cc * 4;

    bool has_extension = (data[0] >> 4) & 0x01;
    if (has_extension && header_size + 4 <= size) {
        uint16_t ext_len = (uint16_t(data[header_size + 2]) << 8) | data[header_size + 3];
        header_size += 4 + ext_len * 4;
    }

    if (header_size >= size) return;

    const uint8_t *payload = data + header_size;
    size_t payload_size = size - header_size;

    stats_.bytes_received += payload_size;
    demuxer_.input(payload, payload_size);
}

void Gb28181MediaSource::on_frame(const MediaFrame &frame) {
    if (status_ == "idle" || status_ == "connecting") {
        status_ = "online";
        codec_ = frame.codec;
    }
    stats_.frames_received++;
    dispatch_frame(frame);
}

void Gb28181MediaSource::start() {
    status_ = "connecting";
    stats_.start_time = std::chrono::steady_clock::now();
}

void Gb28181MediaSource::stop() {
    status_ = "offline";
    demuxer_.reset();
}

// --- Gb28181Server ---

Gb28181Server::Gb28181Server(asio::io_context &io, const Gb28181Config &config,
                             MediaHub &hub)
    : io_(io),
      config_(config),
      hub_(hub),
      sip_socket_(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), config.sip_port)),
      rtp_port_pool_(20000, 21000) {}

Gb28181Server::~Gb28181Server() { stop(); }

void Gb28181Server::start() {
    CSK_LOG_I("GB28181 SIP server started on port {} (ID: {})", config_.sip_port,
              config_.server_id);
    do_receive();

    heartbeat_timer_ = std::make_shared<Timer>(io_);
    heartbeat_timer_->start_periodic(std::chrono::seconds(30),
                                     [this]() { check_heartbeats(); });
}

void Gb28181Server::stop() {
    if (heartbeat_timer_) heartbeat_timer_->cancel();
    asio::error_code ec;
    sip_socket_.close(ec);

    std::lock_guard<std::mutex> lk(streams_mutex_);
    for (auto &[id, source] : streams_) {
        source->stop();
    }
    streams_.clear();

    for (auto &[port, sock] : rtp_sockets_) {
        sock->close();
    }
    rtp_sockets_.clear();
}

void Gb28181Server::do_receive() {
    sip_socket_.async_receive_from(
        asio::buffer(recv_buf_), recv_ep_,
        [this](asio::error_code ec, size_t bytes) {
            if (ec) return;
            std::string data(reinterpret_cast<char *>(recv_buf_.data()), bytes);
            handle_message(data, recv_ep_);
            do_receive();
        });
}

void Gb28181Server::handle_message(const std::string &data,
                                   const asio::ip::udp::endpoint &remote) {
    auto msg = SipMessage::parse(data);

    if (msg.type == SipMessage::RESPONSE) {
        handle_invite_response(msg, remote);
        return;
    }

    if (msg.method == "REGISTER") {
        handle_register(msg, remote);
    } else if (msg.method == "MESSAGE") {
        handle_message_body(msg, remote);
        send_response(msg, 200, "OK", remote);
    } else if (msg.method == "ACK") {
        // ACK for our INVITE 200 OK - nothing to do
    } else if (msg.method == "BYE") {
        send_response(msg, 200, "OK", remote);
    } else {
        send_response(msg, 405, "Method Not Allowed", remote);
    }
}

void Gb28181Server::handle_register(const SipMessage &msg,
                                    const asio::ip::udp::endpoint &remote) {
    std::string device_id = SipMessage::extract_user_from_uri(msg.from());

    int expires = 3600;
    if (!msg.expires().empty()) {
        expires = std::stoi(msg.expires());
    }

    {
        std::lock_guard<std::mutex> lk(devices_mutex_);
        if (expires == 0) {
            // Unregister
            devices_.erase(device_id);
            CSK_LOG_I("GB28181: device {} unregistered", device_id);
        } else {
            auto &dev = devices_[device_id];
            dev.device_id = device_id;
            dev.endpoint = remote;
            dev.registered = true;
            dev.last_heartbeat = std::chrono::steady_clock::now();
            dev.from_tag = SipMessage::extract_tag(msg.from());
            CSK_LOG_I("GB28181: device {} registered from {}:{}", device_id,
                      remote.address().to_string(), remote.port());
        }
    }

    send_response(msg, 200, "OK", remote);
}

void Gb28181Server::handle_message_body(const SipMessage &msg,
                                        const asio::ip::udp::endpoint &) {
    if (msg.content_type().find("Application/MANSCDP+xml") != std::string::npos ||
        msg.content_type().find("application/manscdp+xml") != std::string::npos ||
        msg.content_type().find("Application/MANSCDP") != std::string::npos) {
        // Parse XML body for Keepalive/Catalog etc
        if (msg.body.find("<CmdType>Keepalive</CmdType>") != std::string::npos) {
            // Extract DeviceID from XML
            auto pos = msg.body.find("<DeviceID>");
            if (pos != std::string::npos) {
                pos += 10;
                auto end = msg.body.find("</DeviceID>", pos);
                if (end != std::string::npos) {
                    std::string device_id = msg.body.substr(pos, end - pos);
                    handle_keepalive(device_id);
                }
            }
        }
    }
}

void Gb28181Server::handle_keepalive(const std::string &device_id) {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    auto it = devices_.find(device_id);
    if (it != devices_.end()) {
        it->second.last_heartbeat = std::chrono::steady_clock::now();
    }
}

void Gb28181Server::handle_invite_response(const SipMessage &msg,
                                           const asio::ip::udp::endpoint &remote) {
    if (msg.status_code == 200) {
        std::string call_id = msg.call_id();
        std::string stream_id;

        {
            std::lock_guard<std::mutex> lk(streams_mutex_);
            auto it = pending_invites_.find(call_id);
            if (it != pending_invites_.end()) {
                stream_id = it->second;
                pending_invites_.erase(it);
            }
        }

        if (!stream_id.empty()) {
            CSK_LOG_I("GB28181: INVITE accepted for stream {}", stream_id);

            // Send ACK
            SipMessage ack;
            ack.type = SipMessage::REQUEST;
            ack.method = "ACK";
            ack.request_uri = "sip:" + config_.server_id + "@" +
                              remote.address().to_string() + ":" +
                              std::to_string(remote.port());
            ack.set_header("Via",
                           "SIP/2.0/UDP " + local_ip() + ":" +
                               std::to_string(config_.sip_port) +
                               ";branch=" + generate_branch());
            ack.set_header("From", msg.to());
            ack.set_header("To", msg.from());
            ack.set_header("Call-ID", call_id);
            ack.set_header("CSeq", std::to_string(cseq_++) + " ACK");
            ack.set_header("Max-Forwards", "70");

            auto ack_data = ack.serialize();
            sip_socket_.async_send_to(
                asio::buffer(ack_data), remote,
                [](asio::error_code, size_t) {});
        }
    } else if (msg.status_code >= 400) {
        std::string call_id = msg.call_id();
        std::lock_guard<std::mutex> lk(streams_mutex_);
        auto it = pending_invites_.find(call_id);
        if (it != pending_invites_.end()) {
            CSK_LOG_W("GB28181: INVITE rejected ({}) for stream {}", msg.status_code,
                      it->second);
            pending_invites_.erase(it);
        }
    }
}

bool Gb28181Server::invite_stream(const std::string &device_id,
                                  const std::string &stream_id) {
    Gb28181Device device;
    {
        std::lock_guard<std::mutex> lk(devices_mutex_);
        auto it = devices_.find(device_id);
        if (it == devices_.end() || !it->second.registered) {
            CSK_LOG_W("GB28181: device {} not registered", device_id);
            return false;
        }
        device = it->second;
    }

    // Allocate RTP port
    auto port_opt = rtp_port_pool_.allocate();
    if (!port_opt) {
        CSK_LOG_E("GB28181: no available RTP ports");
        return false;
    }
    uint16_t rtp_port = *port_opt;

    // Create media source
    auto source = std::make_shared<Gb28181MediaSource>(stream_id, device_id, io_);
    source->set_rtp_port(rtp_port);
    source->start();

    // Create RTP receive socket
    auto rtp_socket = std::make_shared<UdpSocket>(io_, rtp_port);
    rtp_socket->start_receive(
        [source](SpanU8 data, const asio::ip::udp::endpoint &) {
            source->on_rtp_data(data.data(), data.size());
        });

    {
        std::lock_guard<std::mutex> lk(streams_mutex_);
        streams_[stream_id] = source;
        rtp_sockets_[rtp_port] = rtp_socket;
    }

    hub_.add_source(source);

    // Send INVITE
    std::string call_id = generate_call_id();
    std::string sdp = build_invite_sdp(stream_id, rtp_port);

    SipMessage invite;
    invite.type = SipMessage::REQUEST;
    invite.method = "INVITE";
    invite.request_uri = "sip:" + device_id + "@" +
                         device.endpoint.address().to_string() + ":" +
                         std::to_string(device.endpoint.port());
    invite.set_header("Via",
                      "SIP/2.0/UDP " + local_ip() + ":" +
                          std::to_string(config_.sip_port) +
                          ";rport;branch=" + generate_branch());
    invite.set_header("From",
                      "<sip:" + config_.server_id + "@" + config_.domain +
                          ">;tag=" + generate_tag());
    invite.set_header("To", "<sip:" + device_id + "@" + config_.domain + ">");
    invite.set_header("Call-ID", call_id);
    invite.set_header("CSeq", std::to_string(cseq_++) + " INVITE");
    invite.set_header("Contact",
                      "<sip:" + config_.server_id + "@" + local_ip() + ":" +
                          std::to_string(config_.sip_port) + ">");
    invite.set_header("Content-Type", "APPLICATION/SDP");
    invite.set_header("Max-Forwards", "70");
    invite.set_header("Subject",
                      stream_id + ":" + config_.server_id + "," +
                          device_id + ":" + std::to_string(0));
    invite.body = sdp;

    {
        std::lock_guard<std::mutex> lk(streams_mutex_);
        pending_invites_[call_id] = stream_id;
    }

    auto invite_data = invite.serialize();
    sip_socket_.async_send_to(
        asio::buffer(invite_data), device.endpoint,
        [stream_id](asio::error_code ec, size_t) {
            if (ec) {
                CSK_LOG_W("GB28181: failed to send INVITE for {}: {}", stream_id,
                          ec.message());
            }
        });

    CSK_LOG_I("GB28181: sent INVITE to {} for stream {} (RTP port {})", device_id,
              stream_id, rtp_port);
    return true;
}

void Gb28181Server::bye_stream(const std::string &stream_id) {
    std::shared_ptr<Gb28181MediaSource> source;
    {
        std::lock_guard<std::mutex> lk(streams_mutex_);
        auto it = streams_.find(stream_id);
        if (it == streams_.end()) return;
        source = it->second;
        streams_.erase(it);

        uint16_t port = source->rtp_port();
        auto socket_it = rtp_sockets_.find(port);
        if (socket_it != rtp_sockets_.end()) {
            socket_it->second->close();
            rtp_sockets_.erase(socket_it);
        }
        rtp_port_pool_.release(port);
    }

    hub_.remove_source(stream_id);
    source->stop();
    CSK_LOG_I("GB28181: stream {} stopped", stream_id);
}

std::vector<std::string> Gb28181Server::device_list() const {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    std::vector<std::string> result;
    for (auto &[id, dev] : devices_) {
        if (dev.registered) result.push_back(id);
    }
    return result;
}

bool Gb28181Server::is_device_online(const std::string &device_id) const {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    auto it = devices_.find(device_id);
    return it != devices_.end() && it->second.registered;
}

void Gb28181Server::send_response(const SipMessage &request, int status_code,
                                  const std::string &reason,
                                  const asio::ip::udp::endpoint &remote,
                                  const std::string &body) {
    SipMessage resp;
    resp.type = SipMessage::RESPONSE;
    resp.status_code = status_code;
    resp.reason_phrase = reason;

    resp.set_header("Via", request.via());
    resp.set_header("From", request.from());

    std::string to_hdr = request.to();
    if (SipMessage::extract_tag(to_hdr).empty()) {
        to_hdr += ";tag=" + generate_tag();
    }
    resp.set_header("To", to_hdr);
    resp.set_header("Call-ID", request.call_id());
    resp.set_header("CSeq", request.cseq());
    resp.set_header("User-Agent", "CamStreamKit/1.0");

    if (!body.empty()) {
        resp.body = body;
        resp.set_header("Content-Type", "APPLICATION/SDP");
    }

    auto data = resp.serialize();
    sip_socket_.async_send_to(asio::buffer(data), remote,
                              [](asio::error_code, size_t) {});
}

std::string Gb28181Server::build_invite_sdp(const std::string &stream_id,
                                            uint16_t rtp_port) {
    std::ostringstream ss;
    std::string ip = local_ip();

    // SSRC: use stream_id hash for predictability
    uint32_t ssrc = 0;
    for (char c : stream_id) ssrc = ssrc * 31 + c;
    ssrc = (ssrc % 9000000) + 1000000;  // 7-digit number

    char ssrc_str[16];
    snprintf(ssrc_str, sizeof(ssrc_str), "%07u", ssrc);

    ss << "v=0\r\n"
       << "o=" << config_.server_id << " 0 0 IN IP4 " << ip << "\r\n"
       << "s=Play\r\n"
       << "c=IN IP4 " << ip << "\r\n"
       << "t=0 0\r\n"
       << "m=video " << rtp_port << " RTP/AVP 96\r\n"
       << "a=recvonly\r\n"
       << "a=rtpmap:96 PS/90000\r\n"
       << "y=" << ssrc_str << "\r\n";

    return ss.str();
}

std::string Gb28181Server::generate_call_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(0, 15);
    const char hex[] = "0123456789abcdef";
    std::string id;
    for (int i = 0; i < 32; i++) id += hex[dist(gen)];
    return id;
}

std::string Gb28181Server::generate_tag() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(0, 35);
    const char chars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    std::string tag;
    for (int i = 0; i < 8; i++) tag += chars[dist(gen)];
    return tag;
}

std::string Gb28181Server::generate_branch() {
    return "z9hG4bK" + generate_call_id().substr(0, 16);
}

std::string Gb28181Server::local_ip() {
    try {
        asio::ip::udp::socket tmp(io_, asio::ip::udp::v4());
        tmp.connect(asio::ip::udp::endpoint(asio::ip::make_address("8.8.8.8"), 80));
        std::string ip = tmp.local_endpoint().address().to_string();
        tmp.close();
        return ip;
    } catch (...) {
        return "0.0.0.0";
    }
}

void Gb28181Server::check_heartbeats() {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lk(devices_mutex_);
    for (auto it = devices_.begin(); it != devices_.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           now - it->second.last_heartbeat)
                           .count();
        if (elapsed > config_.heartbeat_timeout_s) {
            CSK_LOG_W("GB28181: device {} heartbeat timeout, removing",
                      it->second.device_id);
            it = devices_.erase(it);
        } else {
            ++it;
        }
    }
}

}  // namespace csk
