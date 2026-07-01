#pragma once

#include <asio.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/config.h"
#include "core/media_hub.h"
#include "core/timer.h"
#include "gb28181/ps_demuxer.h"
#include "gb28181/sip_message.h"
#include "net/udp_socket.h"

namespace csk {

struct Gb28181Device {
    std::string device_id;
    std::string transport;  // "UDP" or "TCP"
    asio::ip::udp::endpoint endpoint;
    std::chrono::steady_clock::time_point last_heartbeat;
    bool registered = false;
    std::string from_tag;
};

class Gb28181MediaSource : public MediaSource {
public:
    Gb28181MediaSource(const std::string &id, const std::string &device_id,
                       asio::io_context &io);
    ~Gb28181MediaSource();

    std::string id() const override { return id_; }
    CodecType video_codec() const override { return codec_; }
    std::string generate_sdp(const std::string &url) override;
    std::string gb28181_device_id() const override { return device_id_; }

    std::string device_id() const { return device_id_; }

    void on_rtp_data(const uint8_t *data, size_t size);
    void set_rtp_port(uint16_t port) { rtp_port_ = port; }
    uint16_t rtp_port() const { return rtp_port_; }

    void start();
    void stop();

private:
    void on_frame(const MediaFrame &frame);

    std::string id_;
    std::string device_id_;
    asio::io_context &io_;
    CodecType codec_ = CodecType::H264;
    PsDemuxer demuxer_;
    uint16_t rtp_port_ = 0;
};

class Gb28181Server : public std::enable_shared_from_this<Gb28181Server> {
public:
    Gb28181Server(asio::io_context &io, const Gb28181Config &config, MediaHub &hub);
    ~Gb28181Server();

    void start();
    void stop();

    // Invite a device to start streaming
    bool invite_stream(const std::string &device_id, const std::string &stream_id);
    void bye_stream(const std::string &stream_id);

    // Get device list for API
    std::vector<std::string> device_list() const;
    bool is_device_online(const std::string &device_id) const;

private:
    void do_receive();
    void handle_message(const std::string &data, const asio::ip::udp::endpoint &remote);
    void handle_register(const SipMessage &msg, const asio::ip::udp::endpoint &remote);
    void handle_message_body(const SipMessage &msg, const asio::ip::udp::endpoint &remote);
    void handle_keepalive(const std::string &device_id);
    void handle_invite_response(const SipMessage &msg, const asio::ip::udp::endpoint &remote);

    void send_response(const SipMessage &request, int status_code,
                       const std::string &reason,
                       const asio::ip::udp::endpoint &remote,
                       const std::string &body = "");

    std::string build_invite_sdp(const std::string &stream_id, uint16_t rtp_port);
    std::string generate_call_id();
    std::string generate_tag();
    std::string generate_branch();
    std::string local_ip();

    void check_heartbeats();

    asio::io_context &io_;
    Gb28181Config config_;
    MediaHub &hub_;

    asio::ip::udp::socket sip_socket_;
    std::array<uint8_t, 65536> recv_buf_;
    asio::ip::udp::endpoint recv_ep_;

    // RTP receive sockets for incoming streams
    RtpPortPool rtp_port_pool_;
    std::unordered_map<uint16_t, std::shared_ptr<UdpSocket>> rtp_sockets_;

    mutable std::mutex devices_mutex_;
    std::unordered_map<std::string, Gb28181Device> devices_;

    std::mutex streams_mutex_;
    std::unordered_map<std::string, std::shared_ptr<Gb28181MediaSource>> streams_;
    // Track pending invites: call_id -> stream_id
    std::unordered_map<std::string, std::string> pending_invites_;

    std::shared_ptr<Timer> heartbeat_timer_;
    uint32_t cseq_ = 1;
};

}  // namespace csk
