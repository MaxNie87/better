#include "gb28181/ps_demuxer.h"

#include "core/logger.h"

namespace csk {

namespace {
constexpr uint32_t PS_START_CODE = 0x000001BA;
constexpr uint32_t PS_SYSTEM_HEADER = 0x000001BB;
constexpr uint32_t PS_MAP_START = 0x000001BC;
constexpr uint32_t PES_VIDEO_MIN = 0x000001E0;
constexpr uint32_t PES_VIDEO_MAX = 0x000001EF;
constexpr uint32_t PES_AUDIO_MIN = 0x000001C0;
constexpr uint32_t PES_AUDIO_MAX = 0x000001DF;

uint32_t read_u32(const uint8_t *p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

uint16_t read_u16(const uint8_t *p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}
}  // namespace

void PsDemuxer::input(const uint8_t *data, size_t size) {
    pending_data_.insert(pending_data_.end(), data, data + size);

    size_t offset = 0;
    while (offset + 4 <= pending_data_.size()) {
        uint32_t start_code = read_u32(pending_data_.data() + offset);

        if (start_code == PS_START_CODE) {
            if (!parse_ps_header(pending_data_.data(), pending_data_.size(), offset))
                break;
        } else if (start_code == PS_SYSTEM_HEADER) {
            if (offset + 6 > pending_data_.size()) break;
            uint16_t len = read_u16(pending_data_.data() + offset + 4);
            if (offset + 6 + len > pending_data_.size()) break;
            offset += 6 + len;
        } else if (start_code == PS_MAP_START) {
            if (offset + 6 > pending_data_.size()) break;
            uint16_t len = read_u16(pending_data_.data() + offset + 4);
            if (offset + 6 + len > pending_data_.size()) break;

            // Parse PSM to detect codec type
            size_t psm_offset = offset + 6;
            size_t psm_end = psm_offset + len;
            if (psm_end <= pending_data_.size() && len >= 6) {
                // Skip program_stream_info
                uint16_t ps_info_len = read_u16(pending_data_.data() + psm_offset + 2);
                size_t es_map_offset = psm_offset + 4 + ps_info_len;
                if (es_map_offset + 2 <= psm_end) {
                    uint16_t es_map_len = read_u16(pending_data_.data() + es_map_offset);
                    size_t es_offset = es_map_offset + 2;
                    while (es_offset + 4 <= es_map_offset + 2 + es_map_len &&
                           es_offset + 4 <= psm_end) {
                        uint8_t stream_type = pending_data_[es_offset];
                        uint16_t es_info_len = read_u16(pending_data_.data() + es_offset + 2);
                        if (stream_type == 0x1B) codec_ = CodecType::H264;
                        else if (stream_type == 0x24) codec_ = CodecType::H265;
                        es_offset += 4 + es_info_len;
                    }
                }
            }
            offset += 6 + len;
        } else if ((start_code & 0xFFFFFF00) == 0x00000100) {
            if (!parse_pes_packet(pending_data_.data(), pending_data_.size(), offset))
                break;
        } else {
            offset++;
        }
    }

    if (offset > 0) {
        pending_data_.erase(pending_data_.begin(), pending_data_.begin() + offset);
    }
}

bool PsDemuxer::parse_ps_header(const uint8_t *data, size_t size, size_t &offset) {
    if (offset + 14 > size) return false;

    flush_frame();

    // PS pack header is at least 14 bytes
    uint8_t stuff_len = data[offset + 13] & 0x07;
    size_t header_len = 14 + stuff_len;
    if (offset + header_len > size) return false;

    // Extract SCR for timestamp
    uint64_t scr_base =
        ((uint64_t)(data[offset + 4] & 0x38) << 27) |
        ((uint64_t)(data[offset + 4] & 0x03) << 28) |
        ((uint64_t)data[offset + 5] << 20) |
        ((uint64_t)(data[offset + 6] & 0xF8) << 12) |
        ((uint64_t)(data[offset + 6] & 0x03) << 13) |
        ((uint64_t)data[offset + 7] << 5) |
        ((uint64_t)(data[offset + 8] & 0xF8) >> 3);

    timestamp_ = static_cast<uint32_t>(scr_base & 0xFFFFFFFF);
    offset += header_len;
    return true;
}

bool PsDemuxer::parse_pes_packet(const uint8_t *data, size_t size, size_t &offset) {
    if (offset + 6 > size) return false;

    uint32_t stream_id = read_u32(data + offset) & 0xFF;
    uint16_t pes_length = read_u16(data + offset + 4);

    size_t packet_size = 6 + pes_length;
    if (pes_length == 0) {
        // Variable length - skip for now
        offset += 6;
        return true;
    }

    if (offset + packet_size > size) return false;

    uint32_t full_id = 0x00000100 | stream_id;

    if (full_id >= PES_VIDEO_MIN && full_id <= PES_VIDEO_MAX) {
        // Video PES
        if (pes_length >= 3) {
            uint8_t pes_header_len = data[offset + 8];
            size_t payload_offset = 9 + pes_header_len;
            if (payload_offset < packet_size) {
                size_t payload_len = packet_size - payload_offset;
                const uint8_t *payload = data + offset + payload_offset;

                // Extract PTS if available
                uint8_t pts_flag = (data[offset + 7] >> 6) & 0x03;
                if (pts_flag >= 2 && pes_header_len >= 5) {
                    uint64_t pts =
                        ((uint64_t)(data[offset + 9] & 0x0E) << 29) |
                        ((uint64_t)data[offset + 10] << 22) |
                        ((uint64_t)(data[offset + 11] & 0xFE) << 14) |
                        ((uint64_t)data[offset + 12] << 7) |
                        ((uint64_t)(data[offset + 13] & 0xFE) >> 1);
                    timestamp_ = static_cast<uint32_t>(pts & 0xFFFFFFFF);
                }

                frame_buf_.insert(frame_buf_.end(), payload, payload + payload_len);

                // Detect keyframe by scanning for IDR NAL
                for (size_t i = 0; i + 4 < payload_len; ++i) {
                    if (payload[i] == 0 && payload[i + 1] == 0 &&
                        payload[i + 2] == 0 && payload[i + 3] == 1) {
                        uint8_t nal_type;
                        if (codec_ == CodecType::H264) {
                            nal_type = payload[i + 4] & 0x1F;
                            if (nal_type == 5 || nal_type == 7) has_keyframe_ = true;
                        } else if (codec_ == CodecType::H265) {
                            nal_type = (payload[i + 4] >> 1) & 0x3F;
                            if (nal_type >= 16 && nal_type <= 21) has_keyframe_ = true;
                            if (nal_type == 32 || nal_type == 33 || nal_type == 34) has_keyframe_ = true;
                        }
                    }
                }
            }
        }
    }
    // Audio PES is ignored for now

    offset += packet_size;
    return true;
}

void PsDemuxer::flush_frame() {
    if (frame_buf_.empty() || !callback_) return;

    MediaFrame frame;
    frame.codec = codec_;
    frame.timestamp = timestamp_;
    frame.pts = timestamp_;
    frame.dts = timestamp_;
    frame.type = has_keyframe_ ? MediaFrame::VIDEO_KEY : MediaFrame::VIDEO_P;
    frame.data = make_buffer(std::move(frame_buf_));

    callback_(frame);

    frame_buf_.clear();
    has_keyframe_ = false;
}

void PsDemuxer::reset() {
    frame_buf_.clear();
    pending_data_.clear();
    has_keyframe_ = false;
    timestamp_ = 0;
}

}  // namespace csk
