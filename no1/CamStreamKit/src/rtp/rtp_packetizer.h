#pragma once

#include <cstdint>
#include <vector>

#include "codec/h264_parser.h"
#include "rtp/rtp_packet.h"

namespace csk {

class RtpPacketizer {
public:
    RtpPacketizer(uint8_t payload_type = 96, uint32_t ssrc = 0);

    std::vector<RtpPacket> packetize(const NalUnit &nal, uint32_t timestamp);

    void set_ssrc(uint32_t ssrc) { ssrc_ = ssrc; }
    uint16_t current_seq() const { return seq_; }

private:
    std::vector<RtpPacket> pack_single(const NalUnit &nal, uint32_t ts);
    std::vector<RtpPacket> pack_fu_a(const NalUnit &nal, uint32_t ts);

    RtpPacket make_packet(uint32_t ts, bool marker);

    uint16_t seq_ = 0;
    uint32_t ssrc_;
    uint8_t pt_;
    static constexpr size_t MAX_RTP_PAYLOAD = 1400;
};

}  // namespace csk
