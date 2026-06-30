#include "rtsp/rtsp_server.h"

#include <random>
#include <sstream>

#include "codec/h264_parser.h"
#include "core/logger.h"

namespace csk {

static std::string generate_session_id() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist;
    std::ostringstream ss;
    ss << std::hex << dist(rng);
    return ss.str();
}

// --- RtspServerSession ---

RtspServerSession::RtspServerSession(asio::ip::tcp::socket socket, MediaHub &hub)
    : Session(std::move(socket)), hub_(hub) {
    session_id_ = generate_session_id();
}

RtspServerSession::~RtspServerSession() {
    if (subscribed_source_) {
        subscribed_source_->remove_subscriber(this);
    }
}

void RtspServerSession::start() {
    CSK_LOG_I("RTSP session started from {}", remote_endpoint().address().to_string());
    do_read();
}

void RtspServerSession::on_data(SpanU8 data) {
    std::string str(reinterpret_cast<const char *>(data.data()), data.size());

    // Skip interleaved RTP data (client RTCP reports)
    if (!str.empty() && str[0] == '$') return;

    auto requests = parser_.feed(str);
    for (auto &req : requests) {
        if (req.method == "OPTIONS") handle_options(req);
        else if (req.method == "DESCRIBE") handle_describe(req);
        else if (req.method == "SETUP") handle_setup(req);
        else if (req.method == "PLAY") handle_play(req);
        else if (req.method == "TEARDOWN") handle_teardown(req);
    }
}

void RtspServerSession::handle_options(const RtspRequest &req) {
    RtspResponse resp;
    resp.status_code = 200;
    resp.reason = "OK";
    resp.cseq = req.cseq;
    resp.headers["Public"] = "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN";
    send_response(resp);
}

void RtspServerSession::handle_describe(const RtspRequest &req) {
    // Extract stream path from URL: rtsp://host:port/stream/cam1 -> cam1
    auto url_info = RtspParser::parse_url(req.url);
    if (!url_info) {
        RtspResponse resp;
        resp.status_code = 400;
        resp.reason = "Bad Request";
        resp.cseq = req.cseq;
        send_response(resp);
        return;
    }

    std::string path = url_info->path;
    // Strip /stream/ prefix if present
    if (path.find("/stream/") == 0) {
        stream_path_ = path.substr(8);
    } else if (path.size() > 1) {
        stream_path_ = path.substr(1);
    }

    auto source = hub_.find(stream_path_);
    if (!source) {
        RtspResponse resp;
        resp.status_code = 404;
        resp.reason = "Not Found";
        resp.cseq = req.cseq;
        send_response(resp);
        return;
    }

    std::string sdp = source->generate_sdp(req.url);

    RtspResponse resp;
    resp.status_code = 200;
    resp.reason = "OK";
    resp.cseq = req.cseq;
    resp.headers["Content-Type"] = "application/sdp";
    resp.headers["Content-Base"] = req.url + "/";
    resp.body = sdp;
    send_response(resp);
}

void RtspServerSession::handle_setup(const RtspRequest &req) {
    auto it = req.headers.find("Transport");
    if (it == req.headers.end()) {
        RtspResponse resp;
        resp.status_code = 461;
        resp.reason = "Unsupported Transport";
        resp.cseq = req.cseq;
        send_response(resp);
        return;
    }

    transport_ = RtspParser::parse_transport(it->second);

    RtspResponse resp;
    resp.status_code = 200;
    resp.reason = "OK";
    resp.cseq = req.cseq;
    resp.headers["Session"] = session_id_ + ";timeout=60";
    resp.headers["Transport"] = RtspParser::build_transport_response(transport_, session_id_);
    send_response(resp);
}

void RtspServerSession::handle_play(const RtspRequest &req) {
    auto source = hub_.find(stream_path_);
    if (!source) {
        RtspResponse resp;
        resp.status_code = 404;
        resp.reason = "Not Found";
        resp.cseq = req.cseq;
        send_response(resp);
        return;
    }

    subscribed_source_ = source;
    source->add_subscriber(
        std::dynamic_pointer_cast<IMediaSink>(shared_from_this()));

    RtspResponse resp;
    resp.status_code = 200;
    resp.reason = "OK";
    resp.cseq = req.cseq;
    resp.headers["Session"] = session_id_;
    resp.headers["Range"] = "npt=0.000-";
    send_response(resp);

    CSK_LOG_I("RTSP PLAY: stream={}", stream_path_);
}

void RtspServerSession::handle_teardown(const RtspRequest &req) {
    if (subscribed_source_) {
        subscribed_source_->remove_subscriber(this);
        subscribed_source_.reset();
    }

    RtspResponse resp;
    resp.status_code = 200;
    resp.reason = "OK";
    resp.cseq = req.cseq;
    send_response(resp);

    close();
}

void RtspServerSession::on_media_frame(const MediaFrame &frame) {
    if (!frame.data || frame.data->empty()) return;

    // Parse NAL units and packetize to RTP
    H264Parser parser;
    auto nals = parser.parse(frame.data->span());

    for (auto &nal : nals) {
        auto packets = packetizer_.packetize(nal, frame.timestamp);
        for (auto &pkt : packets) {
            if (transport_.mode == TransportMode::TCP) {
                send_rtp_tcp(pkt);
            }
        }
    }
}

void RtspServerSession::send_response(const RtspResponse &resp) {
    send(resp.serialize());
}

void RtspServerSession::send_rtp_tcp(const RtpPacket &pkt) {
    auto rtp_data = pkt.serialize();
    uint16_t len = static_cast<uint16_t>(rtp_data.size());

    std::vector<uint8_t> interleaved;
    interleaved.reserve(4 + rtp_data.size());
    interleaved.push_back('$');
    interleaved.push_back(transport_.rtp_channel);
    interleaved.push_back(static_cast<uint8_t>(len >> 8));
    interleaved.push_back(static_cast<uint8_t>(len & 0xFF));
    interleaved.insert(interleaved.end(), rtp_data.begin(), rtp_data.end());

    send(interleaved.data(), interleaved.size());
}

void RtspServerSession::on_disconnect() {
    if (subscribed_source_) {
        subscribed_source_->remove_subscriber(this);
        subscribed_source_.reset();
    }
    CSK_LOG_I("RTSP session disconnected");
}

// --- RtspServer ---

RtspServer::RtspServer(asio::io_context &io, uint16_t port, MediaHub &hub)
    : hub_(hub) {
    tcp_server_ = std::make_unique<TcpServer>(
        io, port, [this](asio::ip::tcp::socket socket) -> std::shared_ptr<Session> {
            return std::make_shared<RtspServerSession>(std::move(socket), hub_);
        });
}

void RtspServer::start() {
    tcp_server_->start();
    CSK_LOG_I("RTSP Server started");
}

void RtspServer::stop() {
    tcp_server_->stop();
    CSK_LOG_I("RTSP Server stopped");
}

size_t RtspServer::session_count() const {
    return tcp_server_->session_count();
}

}  // namespace csk
