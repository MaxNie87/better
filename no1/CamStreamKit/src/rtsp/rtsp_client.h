#pragma once

#include <asio.hpp>
#include <functional>
#include <memory>
#include <string>

#include "codec/h264_parser.h"
#include "core/media_frame.h"
#include "core/media_source.h"
#include "core/timer.h"
#include "rtp/rtp_packet.h"
#include "rtsp/rtsp_parser.h"
#include "sdp/sdp_parser.h"

namespace csk {

class RtspClient : public std::enable_shared_from_this<RtspClient> {
public:
    struct Config {
        std::string url;
        TransportMode transport = TransportMode::TCP;
        int timeout_ms = 10000;
    };

    using FrameCallback = std::function<void(const MediaFrame &frame)>;

    RtspClient(asio::io_context &io, Config config);
    ~RtspClient();

    void set_frame_callback(FrameCallback cb) { frame_cb_ = std::move(cb); }

    void connect();
    void play();
    void stop();

    bool is_connected() const { return connected_; }
    const Config &config() const { return config_; }

private:
    void do_connect();
    void send_options();
    void send_describe();
    void send_setup();
    void send_play();

    void do_read();
    void handle_response(const std::string &data);
    void handle_interleaved_rtp(uint8_t channel, SpanU8 data);

    void on_rtp_packet(SpanU8 payload);

    asio::io_context &io_;
    Config config_;
    asio::ip::tcp::socket socket_;
    FrameCallback frame_cb_;
    RtspParser parser_;
    H264Parser h264_parser_;

    std::string session_id_;
    std::string content_base_;
    uint32_t cseq_ = 0;
    bool connected_ = false;
    bool playing_ = false;

    std::array<uint8_t, 65536> read_buf_;
    std::string recv_buffer_;

    RtspParser::UrlInfo url_info_;
};

// A MediaSource that pulls from an RTSP camera
class RtspMediaSource : public MediaSource {
public:
    RtspMediaSource(const std::string &id, const RtspClient::Config &config,
                    asio::io_context &io);
    ~RtspMediaSource();

    std::string id() const override { return id_; }
    CodecType video_codec() const override { return codec_; }
    std::string generate_sdp(const std::string &url) override;

    void start();
    void stop();

protected:
    void on_first_subscriber() override;
    void on_no_subscribers() override;

private:
    void start_pull();
    void stop_pull();
    void schedule_reconnect();
    void on_frame(const MediaFrame &frame);

    std::string id_;
    asio::io_context &io_;
    RtspClient::Config client_config_;
    std::shared_ptr<RtspClient> client_;
    CodecType codec_ = CodecType::H264;
    ReconnectPolicy reconnect_;

    std::vector<uint8_t> sps_;
    std::vector<uint8_t> pps_;

    std::shared_ptr<asio::steady_timer> idle_timer_;
    std::shared_ptr<asio::steady_timer> reconnect_timer_;
    bool pulling_ = false;
};

}  // namespace csk
