#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/buffer.h"

namespace csk {

struct NalUnit {
    uint8_t type = 0;      // lower 5 bits of NAL header
    uint8_t ref_idc = 0;   // bits 5-6
    std::vector<uint8_t> data;  // includes NAL header byte

    bool is_sps() const { return type == 7; }
    bool is_pps() const { return type == 8; }
    bool is_idr() const { return type == 5; }
    bool is_slice() const { return type == 1; }
    bool is_sei() const { return type == 6; }
    bool is_key_frame() const { return is_idr(); }
};

struct H264SpsInfo {
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t profile_idc = 0;
    uint8_t level_idc = 0;
};

class H264Parser {
public:
    // Parse NAL units from Annex-B byte stream (00 00 00 01 or 00 00 01 separated)
    std::vector<NalUnit> parse(SpanU8 data);

    // Depacketize single RTP payload into NAL unit (handles FU-A reassembly)
    std::optional<NalUnit> depacketize_rtp(SpanU8 payload);

    // Reset FU-A reassembly state
    void reset_fu_state() { fu_buffer_.clear(); }

    // Parse basic SPS info (width/height)
    static std::optional<H264SpsInfo> parse_sps(SpanU8 sps_data);

private:
    std::vector<uint8_t> fu_buffer_;
    uint8_t fu_nal_header_ = 0;
};

}  // namespace csk
