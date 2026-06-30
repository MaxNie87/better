#include "codec/h265_parser.h"

namespace csk {

std::vector<H265NalUnit> H265Parser::parse(SpanU8 data) {
    std::vector<H265NalUnit> nals;
    if (data.size() < 5) return nals;

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

        // Trim trailing start code bytes
        if (idx + 1 < start_positions.size()) {
            size_t next_start = start_positions[idx + 1];
            if (next_start >= 4 && data[next_start - 4] == 0 && data[next_start - 3] == 0 &&
                data[next_start - 2] == 0 && data[next_start - 1] == 1) {
                end = next_start - 4;
            } else if (next_start >= 3 && data[next_start - 3] == 0 &&
                       data[next_start - 2] == 0 && data[next_start - 1] == 1) {
                end = next_start - 3;
            }
        }

        if (end <= start + 1) continue;

        H265NalUnit nal;
        nal.data.assign(data.begin() + start, data.begin() + end);
        if (nal.data.size() >= 2) {
            // H.265 NAL header is 2 bytes: [forbidden(1) type(6) layer_id(6) temporal_id(3)]
            nal.type = (nal.data[0] >> 1) & 0x3F;
            nal.layer_id = ((nal.data[0] & 0x01) << 5) | ((nal.data[1] >> 3) & 0x1F);
            nal.temporal_id = (nal.data[1] & 0x07) - 1;
        }
        nals.push_back(std::move(nal));
    }

    return nals;
}

std::optional<H265NalUnit> H265Parser::depacketize_rtp(SpanU8 payload) {
    if (payload.size() < 2) return std::nullopt;

    uint8_t nal_type = (payload[0] >> 1) & 0x3F;

    if (nal_type < 48) {
        // Single NAL unit
        H265NalUnit nal;
        nal.data.assign(payload.begin(), payload.end());
        nal.type = nal_type;
        nal.layer_id = ((payload[0] & 0x01) << 5) | ((payload[1] >> 3) & 0x1F);
        nal.temporal_id = (payload[1] & 0x07) - 1;
        return nal;
    }

    if (nal_type == 49) {
        // FU (Fragmentation Unit)
        if (payload.size() < 3) return std::nullopt;

        uint8_t fu_header = payload[2];
        bool start = (fu_header & 0x80) != 0;
        bool end = (fu_header & 0x40) != 0;
        uint8_t frag_type = fu_header & 0x3F;

        if (start) {
            fu_buffer_.clear();
            // Reconstruct NAL header
            uint8_t byte0 = (payload[0] & 0x81) | (frag_type << 1);
            uint8_t byte1 = payload[1];
            fu_buffer_.push_back(byte0);
            fu_buffer_.push_back(byte1);
            fu_nal_header_ = (static_cast<uint16_t>(byte0) << 8) | byte1;
        }

        if (fu_buffer_.empty()) return std::nullopt;

        fu_buffer_.insert(fu_buffer_.end(), payload.begin() + 3, payload.end());

        if (end) {
            H265NalUnit nal;
            nal.data = std::move(fu_buffer_);
            nal.type = frag_type;
            fu_buffer_.clear();
            return nal;
        }

        return std::nullopt;
    }

    return std::nullopt;
}

}  // namespace csk
