#pragma once

#include <asio.hpp>
#include <array>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "core/buffer.h"
#include "core/logger.h"

namespace csk {

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(asio::ip::tcp::socket socket)
        : socket_(std::move(socket)),
          strand_(static_cast<asio::io_context &>(socket_.get_executor().context())) {}

    virtual ~Session() = default;

    virtual void start() = 0;
    virtual void on_data(SpanU8 data) = 0;

    void send(const std::string &data) {
        send(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    }

    void send(const uint8_t *data, size_t size) {
        auto buf = std::make_shared<std::vector<uint8_t>>(data, data + size);
        asio::post(strand_, [this, self = shared_from_this(), buf]() {
            bool writing = !write_queue_.empty();
            write_queue_.push_back(buf);
            if (!writing) {
                do_write();
            }
        });
    }

    void close() {
        asio::post(strand_, [this, self = shared_from_this()]() {
            asio::error_code ec;
            socket_.close(ec);
        });
    }

    asio::ip::tcp::endpoint remote_endpoint() const {
        asio::error_code ec;
        return socket_.remote_endpoint(ec);
    }

protected:
    void do_read() {
        auto self = shared_from_this();
        socket_.async_read_some(
            asio::buffer(read_buf_),
            asio::bind_executor(strand_, [this, self](asio::error_code ec, size_t bytes) {
                if (!ec) {
                    on_data(SpanU8(read_buf_.data(), bytes));
                    do_read();
                } else {
                    on_disconnect();
                }
            }));
    }

    virtual void on_disconnect() {}

    asio::ip::tcp::socket socket_;
    asio::io_context::strand strand_;

private:
    void do_write() {
        auto self = shared_from_this();
        asio::async_write(
            socket_, asio::buffer(*write_queue_.front()),
            asio::bind_executor(strand_, [this, self](asio::error_code ec, size_t) {
                if (!ec) {
                    write_queue_.pop_front();
                    if (!write_queue_.empty()) {
                        do_write();
                    }
                }
            }));
    }

    std::array<uint8_t, 8192> read_buf_;
    std::deque<std::shared_ptr<std::vector<uint8_t>>> write_queue_;
};

class TcpServer {
public:
    using SessionFactory = std::function<std::shared_ptr<Session>(asio::ip::tcp::socket)>;

    TcpServer(asio::io_context &io, uint16_t port, SessionFactory factory)
        : acceptor_(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          factory_(std::move(factory)) {}

    void start() {
        CSK_LOG_I("TCP server listening on port {}", acceptor_.local_endpoint().port());
        do_accept();
    }

    void stop() {
        asio::error_code ec;
        acceptor_.close(ec);
        std::lock_guard lock(mtx_);
        for (auto &session : sessions_) {
            session->close();
        }
        sessions_.clear();
    }

    size_t session_count() const {
        std::lock_guard lock(mtx_);
        return sessions_.size();
    }

private:
    void do_accept() {
        acceptor_.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                auto session = factory_(std::move(socket));
                {
                    std::lock_guard lock(mtx_);
                    sessions_.insert(session);
                }
                session->start();
            }
            if (acceptor_.is_open()) {
                do_accept();
            }
        });
    }

    asio::ip::tcp::acceptor acceptor_;
    SessionFactory factory_;
    mutable std::mutex mtx_;
    std::set<std::shared_ptr<Session>> sessions_;
};

}  // namespace csk
