#include "core/media_hub.h"

#include "core/logger.h"

namespace csk {

MediaHub &MediaHub::instance() {
    static MediaHub hub;
    return hub;
}

void MediaHub::add_source(std::shared_ptr<MediaSource> source) {
    std::unique_lock lock(mtx_);
    auto id = source->id();
    sources_[id] = std::move(source);
    CSK_LOG_I("MediaHub: added source '{}'", id);
}

void MediaHub::remove_source(const std::string &id) {
    std::unique_lock lock(mtx_);
    auto it = sources_.find(id);
    if (it != sources_.end()) {
        sources_.erase(it);
        CSK_LOG_I("MediaHub: removed source '{}'", id);
    }
}

std::shared_ptr<MediaSource> MediaHub::find(const std::string &id) const {
    std::shared_lock lock(mtx_);
    auto it = sources_.find(id);
    if (it != sources_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<StreamInfo> MediaHub::list_all() const {
    std::shared_lock lock(mtx_);
    std::vector<StreamInfo> result;
    result.reserve(sources_.size());

    for (auto &[id, source] : sources_) {
        StreamInfo info;
        info.id = id;
        info.codec = source->video_codec();
        info.subscriber_count = source->subscriber_count();
        info.status = source->status();
        info.bitrate_kbps = source->stats().current_bitrate_kbps;
        info.fps = source->stats().current_fps;
        info.gb28181_device_id = source->gb28181_device_id();
        result.push_back(std::move(info));
    }

    return result;
}

size_t MediaHub::source_count() const {
    std::shared_lock lock(mtx_);
    return sources_.size();
}

void MediaHub::stop_all() {
    std::unique_lock lock(mtx_);
    sources_.clear();
    CSK_LOG_I("MediaHub: stopped all sources");
}

}  // namespace csk
