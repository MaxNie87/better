#include "core/media_source.h"

#include "core/logger.h"

namespace csk {

void MediaSource::add_subscriber(std::weak_ptr<IMediaSink> sink) {
    std::lock_guard lock(mtx_);

    subscribers_.push_back(sink);

    // Send cached GOP for instant playback
    if (auto sp = sink.lock()) {
        for (auto &frame : gop_cache_.get_cached_gop()) {
            sp->on_media_frame(frame);
        }
    }

    if (subscribers_.size() == 1) {
        CSK_LOG_I("First subscriber for source {}, starting pull", id());
        on_first_subscriber();
    }
}

void MediaSource::remove_subscriber(IMediaSink *sink) {
    std::lock_guard lock(mtx_);

    subscribers_.erase(
        std::remove_if(subscribers_.begin(), subscribers_.end(),
                       [sink](auto &wp) {
                           auto sp = wp.lock();
                           return !sp || sp.get() == sink;
                       }),
        subscribers_.end());

    if (subscribers_.empty()) {
        CSK_LOG_I("No subscribers for source {}", id());
        on_no_subscribers();
    }
}

size_t MediaSource::subscriber_count() const {
    std::lock_guard lock(mtx_);
    return subscribers_.size();
}

void MediaSource::dispatch_frame(const MediaFrame &frame) {
    stats_.frames_received++;
    if (frame.data) {
        stats_.bytes_received += frame.data->size();
    }

    gop_cache_.on_frame(frame);

    std::lock_guard lock(mtx_);

    // Clean up expired weak_ptrs and dispatch
    auto it = subscribers_.begin();
    while (it != subscribers_.end()) {
        if (auto sp = it->lock()) {
            sp->on_media_frame(frame);
            ++it;
        } else {
            it = subscribers_.erase(it);
        }
    }
}

}  // namespace csk
