#pragma once

#include <asio.hpp>
#include <openssl/ssl.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "codec/h264_parser.h"
#include "core/media_frame.h"
#include "core/media_source.h"
#include "rtp/rtp_packetizer.h"
#include "webrtc/srtp_session.h"

namespace csk {

struct WebRtcSessionConfig {
    std::string stream_id;
    std::string local_ufrag;
    std::string local_pwd;
    std::string remote_ufrag;
    std::string remote_pwd;
    uint32_t ssrc = 0;
};

enum class WebRtcState {
    NEW,
    CHECKING,
    CONNECTED,
    DTLS_HANDSHAKING,
    READY,
    CLOSED
};

class WebRtcSession : public IMediaSink, public std::enable_shared_from_this<WebRtcSession> {
public:
    WebRtcSession(asio::io_context &io, const WebRtcSessionConfig &config);
    ~WebRtcSession();

    void start(asio::ip::udp::endpoint remote_ep);
    void stop();

    // Called when UDP data arrives for this session
    void on_udp_data(const uint8_t *data, size_t len, const asio::ip::udp::endpoint &from);

    // IMediaSink
    void on_media_frame(const MediaFrame &frame) override;

    WebRtcState state() const { return state_; }
    const std::string &local_ufrag() const { return config_.local_ufrag; }
    const std::string &remote_ufrag() const { return config_.remote_ufrag; }
    const std::string &stream_id() const { return config_.stream_id; }

    void set_udp_sender(std::function<void(const uint8_t *, size_t,
                                           const asio::ip::udp::endpoint &)> sender) {
        udp_send_ = std::move(sender);
    }

private:
    void handle_stun(const uint8_t *data, size_t len, const asio::ip::udp::endpoint &from);
    void start_dtls();
    void do_dtls_handshake();
    void on_dtls_connected();
    void send_srtp_packet(std::vector<uint8_t> &rtp_packet);

    WebRtcSessionConfig config_;
    WebRtcState state_ = WebRtcState::NEW;
    asio::io_context &io_;
    asio::ip::udp::endpoint remote_ep_;

    // DTLS
    SSL *ssl_ = nullptr;
    BIO *read_bio_ = nullptr;
    BIO *write_bio_ = nullptr;

    // SRTP
    SrtpSession srtp_;

    // RTP / H264
    RtpPacketizer packetizer_;
    H264Parser h264_parser_;
    uint32_t timestamp_ = 0;

    std::function<void(const uint8_t *, size_t, const asio::ip::udp::endpoint &)> udp_send_;
};

}  // namespace csk
