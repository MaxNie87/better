#pragma once

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/media_source.h"

namespace csk {

struct StreamInfo {
    std::string id;
    std::string source_url;
    CodecType codec = CodecType::UNKNOWN;
    uint16_t width = 0;
    uint16_t height = 0;
    size_t subscriber_count = 0;
    std::string status;
    uint32_t bitrate_kbps = 0;
    uint32_t fps = 0;
};

class MediaHub {
public:
    static MediaHub &instance();

    void add_source(std::shared_ptr<MediaSource> source);
    void remove_source(const std::string &id);
    std::shared_ptr<MediaSource> find(const std::string &id) const;
    std::vector<StreamInfo> list_all() const;
    size_t source_count() const;

    void stop_all();

private:
    MediaHub() = default;
    mutable std::shared_mutex mtx_;
    std::unordered_map<std::string, std::shared_ptr<MediaSource>> sources_;
};

}  // namespace csk
