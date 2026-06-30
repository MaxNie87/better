#pragma once

#include <cstdint>
#include <memory>

#include "core/buffer.h"

namespace csk {

enum class CodecType {
    H264,
    H265,
    AAC,
    G711A,
    G711U,
    OPUS,
    UNKNOWN
};

struct MediaFrame {
    enum Type { VIDEO_KEY, VIDEO_P, AUDIO };

    Type type = VIDEO_P;
    CodecType codec = CodecType::H264;
    uint32_t timestamp = 0;  // 90kHz timebase for video
    uint64_t dts = 0;
    uint64_t pts = 0;
    BufferPtr data;

    bool is_video() const { return type == VIDEO_KEY || type == VIDEO_P; }
    bool is_key_frame() const { return type == VIDEO_KEY; }
    bool is_audio() const { return type == AUDIO; }
};

}  // namespace csk
