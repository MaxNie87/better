#pragma once

#include <asio.hpp>
#include <memory>
#include <string>

#include "core/media_hub.h"
#include "core/media_source.h"
#include "codec/h264_parser.h"
#include "net/tcp_server.h"
#include "rtp/rtp_packetizer.h"
#include "rtsp/rtsp_parser.h"

namespace csk {

class RtspServerSession : public Session, public IMediaSink {
public:
    RtspServerSession(asio::ip::tcp::socket socket, MediaHub &hub);
    ~RtspServerSession();

    void start() override;
    void on_data(SpanU8 data) override;
    void on_media_frame(const MediaFrame &frame) override;

private:
    void handle_options(const RtspRequest &req);
    void handle_describe(const RtspRequest &req);
    void handle_setup(const RtspRequest &req);
    void handle_play(const RtspRequest &req);
    void handle_teardown(const RtspRequest &req);

    void send_response(const RtspResponse &resp);
    void send_rtp_tcp(const RtpPacket &pkt);

    void on_disconnect() override;

    MediaHub &hub_;
    RtspParser parser_;
    RtpPacketizer packetizer_;
    RtspTransport transport_;

    std::string session_id_;
    std::string stream_path_;
    std::shared_ptr<MediaSource> subscribed_source_;
};

class RtspServer {
public:
    RtspServer(asio::io_context &io, uint16_t port, MediaHub &hub);

    void start();
    void stop();
    size_t session_count() const;

private:
    std::unique_ptr<TcpServer> tcp_server_;
    MediaHub &hub_;
};

}  // namespace csk
