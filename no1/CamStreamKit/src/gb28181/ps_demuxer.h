#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "core/media_frame.h"

namespace csk {

class PsDemuxer {
public:
    using FrameCallback = std::function<void(const MediaFrame &)>;

    void set_callback(FrameCallback cb) { callback_ = std::move(cb); }
    void input(const uint8_t *data, size_t size);
    void reset();

private:
    bool parse_ps_header(const uint8_t *data, size_t size, size_t &offset);
    bool parse_pes_packet(const uint8_t *data, size_t size, size_t &offset);
    void flush_frame();

    FrameCallback callback_;
    std::vector<uint8_t> frame_buf_;
    CodecType codec_ = CodecType::H264;
    uint32_t timestamp_ = 0;
    bool has_keyframe_ = false;
    std::vector<uint8_t> pending_data_;
};

}  // namespace csk
