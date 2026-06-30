#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/buffer.h"

namespace csk {

struct H265NalUnit {
    uint8_t type = 0;       // 6 bits: NAL unit type
    uint8_t layer_id = 0;
    uint8_t temporal_id = 0;
    std::vector<uint8_t> data;  // includes 2-byte NAL header

    bool is_vps() const { return type == 32; }
    bool is_sps() const { return type == 33; }
    bool is_pps() const { return type == 34; }
    bool is_idr() const { return type == 19 || type == 20; }
    bool is_trail() const { return type <= 9; }
    bool is_key_frame() const { return is_idr(); }
};

class H265Parser {
public:
    std::vector<H265NalUnit> parse(SpanU8 data);
    std::optional<H265NalUnit> depacketize_rtp(SpanU8 payload);
    void reset_fu_state() { fu_buffer_.clear(); }

private:
    std::vector<uint8_t> fu_buffer_;
    uint16_t fu_nal_header_ = 0;
};

}  // namespace csk
