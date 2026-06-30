#pragma once

#include <mutex>
#include <vector>

#include "core/media_frame.h"

namespace csk {

class GopCache {
public:
    void on_frame(const MediaFrame &frame);
    std::vector<MediaFrame> get_cached_gop() const;
    void clear();
    size_t frame_count() const;

private:
    mutable std::mutex mtx_;
    std::vector<MediaFrame> frames_;
};

}  // namespace csk
