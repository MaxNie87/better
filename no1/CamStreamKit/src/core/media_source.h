#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "core/gop_cache.h"
#include "core/media_frame.h"

namespace csk {

class IMediaSink {
public:
    virtual ~IMediaSink() = default;
    virtual void on_media_frame(const MediaFrame &frame) = 0;
};

class MediaSource : public std::enable_shared_from_this<MediaSource> {
public:
    virtual ~MediaSource() = default;

    virtual std::string id() const = 0;
    virtual CodecType video_codec() const = 0;
    virtual std::string generate_sdp(const std::string &url) = 0;
    virtual std::string status() const { return status_; }
    virtual std::string gb28181_device_id() const { return ""; }

    void add_subscriber(std::weak_ptr<IMediaSink> sink);
    void remove_subscriber(IMediaSink *sink);
    size_t subscriber_count() const;

    // Stream statistics
    struct Stats {
        std::atomic<uint64_t> bytes_received{0};
        std::atomic<uint64_t> frames_received{0};
        std::atomic<uint32_t> current_bitrate_kbps{0};
        std::atomic<uint32_t> current_fps{0};
        std::chrono::steady_clock::time_point start_time;
    };

    const Stats &stats() const { return stats_; }

protected:
    void dispatch_frame(const MediaFrame &frame);

    virtual void on_first_subscriber() {}
    virtual void on_no_subscribers() {}

    std::string status_ = "idle";
    Stats stats_;

private:
    mutable std::mutex mtx_;
    std::vector<std::weak_ptr<IMediaSink>> subscribers_;
    GopCache gop_cache_;
};

}  // namespace csk
