#pragma once

#include <asio.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <set>

#include "core/buffer.h"
#include "core/logger.h"

namespace csk {

class UdpSocket : public std::enable_shared_from_this<UdpSocket> {
public:
    using ReceiveCallback =
        std::function<void(SpanU8, const asio::ip::udp::endpoint &)>;

    UdpSocket(asio::io_context &io, uint16_t port)
        : socket_(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {}

    UdpSocket(asio::io_context &io, const asio::ip::udp::endpoint &endpoint)
        : socket_(io, endpoint) {}

    void start_receive(ReceiveCallback cb) {
        receive_cb_ = std::move(cb);
        do_receive();
    }

    void send_to(SpanU8 data, const asio::ip::udp::endpoint &ep) {
        socket_.async_send_to(
            asio::buffer(data.data(), data.size()), ep,
            [](asio::error_code ec, size_t) {
                if (ec) {
                    CSK_LOG_W("UDP send error: {}", ec.message());
                }
            });
    }

    void send_to(const uint8_t *data, size_t size, const asio::ip::udp::endpoint &ep) {
        send_to(SpanU8(data, size), ep);
    }

    uint16_t local_port() const { return socket_.local_endpoint().port(); }

    void close() {
        asio::error_code ec;
        socket_.close(ec);
    }

private:
    void do_receive() {
        socket_.async_receive_from(
            asio::buffer(recv_buf_), remote_ep_,
            [this, self = shared_from_this()](asio::error_code ec, size_t bytes) {
                if (!ec && receive_cb_) {
                    receive_cb_(SpanU8(recv_buf_.data(), bytes), remote_ep_);
                }
                if (socket_.is_open()) {
                    do_receive();
                }
            });
    }

    asio::ip::udp::socket socket_;
    ReceiveCallback receive_cb_;
    asio::ip::udp::endpoint remote_ep_;
    std::array<uint8_t, 65536> recv_buf_;
};

class RtpPortPool {
public:
    RtpPortPool(uint16_t start = 30000, uint16_t end = 31000)
        : start_(start), end_(end), next_(start) {}

    std::optional<uint16_t> allocate() {
        std::lock_guard lock(mtx_);
        for (uint16_t i = 0; i < (end_ - start_); i += 2) {
            uint16_t port = next_;
            next_ += 2;
            if (next_ >= end_) next_ = start_;
            if (used_.find(port) == used_.end()) {
                used_.insert(port);
                return port;
            }
        }
        return std::nullopt;
    }

    void release(uint16_t port) {
        std::lock_guard lock(mtx_);
        used_.erase(port);
    }

private:
    uint16_t start_;
    uint16_t end_;
    uint16_t next_;
    std::mutex mtx_;
    std::set<uint16_t> used_;
};

}  // namespace csk
