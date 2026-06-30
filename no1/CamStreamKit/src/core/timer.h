#pragma once

#include <asio.hpp>
#include <chrono>
#include <functional>
#include <memory>

namespace csk {

class Timer : public std::enable_shared_from_this<Timer> {
public:
    using Callback = std::function<void()>;

    Timer(asio::io_context &io) : timer_(io) {}

    void start_once(std::chrono::milliseconds delay, Callback cb) {
        cb_ = std::move(cb);
        timer_.expires_after(delay);
        timer_.async_wait([self = shared_from_this()](const asio::error_code &ec) {
            if (!ec && self->cb_) {
                self->cb_();
            }
        });
    }

    void start_periodic(std::chrono::milliseconds interval, Callback cb) {
        cb_ = std::move(cb);
        interval_ = interval;
        schedule_next();
    }

    void cancel() {
        cancelled_ = true;
        timer_.cancel();
    }

private:
    void schedule_next() {
        timer_.expires_after(interval_);
        timer_.async_wait([self = shared_from_this()](const asio::error_code &ec) {
            if (!ec && !self->cancelled_ && self->cb_) {
                self->cb_();
                self->schedule_next();
            }
        });
    }

    asio::steady_timer timer_;
    Callback cb_;
    std::chrono::milliseconds interval_{1000};
    bool cancelled_ = false;
};

class ReconnectPolicy {
public:
    ReconnectPolicy(int base_ms = 1000, int max_ms = 30000)
        : base_ms_(base_ms), max_ms_(max_ms), current_ms_(base_ms) {}

    int next_delay_ms() {
        int delay = current_ms_;
        current_ms_ = std::min(current_ms_ * 2, max_ms_);
        return delay;
    }

    void reset() { current_ms_ = base_ms_; }

private:
    int base_ms_;
    int max_ms_;
    int current_ms_;
};

}  // namespace csk
