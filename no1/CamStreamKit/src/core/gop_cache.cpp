#include "core/gop_cache.h"

namespace csk {

void GopCache::on_frame(const MediaFrame &frame) {
    std::lock_guard lock(mtx_);

    if (frame.is_key_frame()) {
        frames_.clear();
    }
    frames_.push_back(frame);
}

std::vector<MediaFrame> GopCache::get_cached_gop() const {
    std::lock_guard lock(mtx_);
    return frames_;
}

void GopCache::clear() {
    std::lock_guard lock(mtx_);
    frames_.clear();
}

size_t GopCache::frame_count() const {
    std::lock_guard lock(mtx_);
    return frames_.size();
}

}  // namespace csk
