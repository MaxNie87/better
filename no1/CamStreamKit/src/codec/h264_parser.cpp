#include "codec/h264_parser.h"

namespace csk {

std::vector<NalUnit> H264Parser::parse(SpanU8 data) {
    std::vector<NalUnit> nals;
    if (data.size() < 4) return nals;

    // Find start codes (00 00 00 01 or 00 00 01)
    std::vector<size_t> start_positions;
    for (size_t i = 0; i + 3 < data.size(); ++i) {
        if (data[i] == 0 && data[i + 1] == 0) {
            if (data[i + 2] == 1) {
                start_positions.push_back(i + 3);
                i += 2;
            } else if (i + 3 < data.size() && data[i + 2] == 0 && data[i + 3] == 1) {
                start_positions.push_back(i + 4);
                i += 3;
            }
        }
    }

    for (size_t idx = 0; idx < start_positions.size(); ++idx) {
        size_t start = start_positions[idx];
        size_t end = (idx + 1 < start_positions.size())
                         ? start_positions[idx + 1]
                         : data.size();

        // Remove trailing zeros (previous start code prefix)
        while (end > start && end > 2) {
            if (data[end - 1] == 0 && data[end - 2] == 0 &&
                (end - start > 2 && data[end - 3] == 0)) {
                end -= 1;
            } else {
                break;
            }
        }
        // Adjust end for next start code
        if (idx + 1 < start_positions.size()) {
            size_t next_start = start_positions[idx + 1];
            // Walk back to remove 00 00 00 01 or 00 00 01 prefix
            if (next_start >= 4 && data[next_start - 4] == 0 && data[next_start - 3] == 0 &&
                data[next_start - 2] == 0 && data[next_start - 1] == 1) {
                end = next_start - 4;
            } else if (next_start >= 3 && data[next_start - 3] == 0 &&
                       data[next_start - 2] == 0 && data[next_start - 1] == 1) {
                end = next_start - 3;
            }
        }

        if (end <= start) continue;

        NalUnit nal;
        nal.data.assign(data.begin() + start, data.begin() + end);
        if (!nal.data.empty()) {
            nal.type = nal.data[0] & 0x1F;
            nal.ref_idc = (nal.data[0] >> 5) & 0x03;
        }
        nals.push_back(std::move(nal));
    }

    return nals;
}

std::optional<NalUnit> H264Parser::depacketize_rtp(SpanU8 payload) {
    if (payload.empty()) return std::nullopt;

    uint8_t nal_type = payload[0] & 0x1F;

    if (nal_type >= 1 && nal_type <= 23) {
        // Single NAL unit
        NalUnit nal;
        nal.data.assign(payload.begin(), payload.end());
        nal.type = nal_type;
        nal.ref_idc = (payload[0] >> 5) & 0x03;
        return nal;
    }

    if (nal_type == 28) {
        // FU-A
        if (payload.size() < 2) return std::nullopt;

        uint8_t fu_header = payload[1];
        bool start = (fu_header & 0x80) != 0;
        bool end = (fu_header & 0x40) != 0;
        uint8_t frag_type = fu_header & 0x1F;

        if (start) {
            fu_buffer_.clear();
            fu_nal_header_ = (payload[0] & 0xE0) | frag_type;
            fu_buffer_.push_back(fu_nal_header_);
        }

        if (fu_buffer_.empty()) return std::nullopt;  // Missing start fragment

        fu_buffer_.insert(fu_buffer_.end(), payload.begin() + 2, payload.end());

        if (end) {
            NalUnit nal;
            nal.data = std::move(fu_buffer_);
            nal.type = frag_type;
            nal.ref_idc = (fu_nal_header_ >> 5) & 0x03;
            fu_buffer_.clear();
            return nal;
        }

        return std::nullopt;  // Fragment not complete yet
    }

    if (nal_type == 24) {
        // STAP-A: return the first NAL unit
        if (payload.size() < 4) return std::nullopt;
        size_t offset = 1;
        uint16_t nal_size = (static_cast<uint16_t>(payload[offset]) << 8) | payload[offset + 1];
        offset += 2;
        if (offset + nal_size > payload.size()) return std::nullopt;

        NalUnit nal;
        nal.data.assign(payload.begin() + offset, payload.begin() + offset + nal_size);
        if (!nal.data.empty()) {
            nal.type = nal.data[0] & 0x1F;
            nal.ref_idc = (nal.data[0] >> 5) & 0x03;
        }
        return nal;
    }

    return std::nullopt;
}

std::optional<H264SpsInfo> H264Parser::parse_sps(SpanU8 sps_data) {
    if (sps_data.size() < 4) return std::nullopt;

    // Simplified SPS parsing: just extract profile and level
    // Full SPS parsing requires exp-golomb decoding
    H264SpsInfo info;
    size_t offset = (sps_data[0] & 0x1F) == 7 ? 1 : 0;  // Skip NAL header if present

    if (offset + 3 >= sps_data.size()) return std::nullopt;

    info.profile_idc = sps_data[offset];
    info.level_idc = sps_data[offset + 2];

    // Width/height require full exp-golomb parsing of SPS
    // Placeholder: set to 0, caller should use FFmpeg for accurate parsing
    info.width = 0;
    info.height = 0;

    return info;
}

}  // namespace csk
