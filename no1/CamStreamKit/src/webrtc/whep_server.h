#pragma once

#include <asio.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/media_hub.h"
#include "webrtc/webrtc_session.h"

namespace csk {

struct WhepOffer {
    std::string sdp;
};

struct WhepAnswer {
    std::string sdp;
    std::string resource_id;
};

class WhepServer : public std::enable_shared_from_this<WhepServer> {
public:
    WhepServer(asio::io_context &io, MediaHub &hub, uint16_t udp_port = 0);
    ~WhepServer();

    void start();
    void stop();

    // Called from HTTP handler: POST /whep/{stream_id}
    // Returns SDP answer or empty on failure
    WhepAnswer handle_offer(const std::string &stream_id, const std::string &offer_sdp);

    // Called from HTTP handler: DELETE /whep/resource/{id}
    bool handle_delete(const std::string &resource_id);

    uint16_t udp_port() const { return udp_port_; }

private:
    void do_receive();
    std::string generate_id();
    std::string build_answer_sdp(const WebRtcSessionConfig &config, uint16_t port,
                                 const std::string &host_ip, int h264_pt,
                                 const std::string &h264_fmtp);
    std::string parse_remote_ufrag(const std::string &sdp);
    std::string parse_remote_pwd(const std::string &sdp);
    std::string detect_local_ip();
    static int find_h264_pt(const std::string &offer_sdp, std::string &fmtp_out);

    asio::io_context &io_;
    MediaHub &hub_;
    asio::ip::udp::socket socket_;
    uint16_t udp_port_ = 0;

    std::array<uint8_t, 2048> recv_buf_;
    asio::ip::udp::endpoint recv_ep_;

    std::mutex sessions_mutex_;
    std::unordered_map<std::string, std::shared_ptr<WebRtcSession>> sessions_;  // by resource_id
    std::unordered_map<std::string, std::shared_ptr<WebRtcSession>> by_ufrag_;   // by local_ufrag
};

}  // namespace csk
