#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/buffer.h"

namespace csk {

struct RtpHeader {
    uint8_t version = 2;
    bool padding = false;
    bool extension = false;
    uint8_t csrc_count = 0;
    bool marker = false;
    uint8_t payload_type = 96;
    uint16_t sequence = 0;
    uint32_t timestamp = 0;
    uint32_t ssrc = 0;
};

class RtpPacket {
public:
    RtpPacket() = default;

    static std::optional<RtpPacket> parse(SpanU8 data);
    std::vector<uint8_t> serialize() const;

    const RtpHeader &header() const { return header_; }
    RtpHeader &header() { return header_; }

    SpanU8 payload() const { return {payload_.data(), payload_.size()}; }
    void set_payload(SpanU8 data) {
        payload_.assign(data.begin(), data.end());
    }
    void set_payload(std::vector<uint8_t> data) { payload_ = std::move(data); }

    size_t total_size() const { return 12 + payload_.size(); }

private:
    RtpHeader header_;
    std::vector<uint8_t> payload_;
};

struct RtcpSenderReport {
    uint32_t ssrc = 0;
    uint64_t ntp_timestamp = 0;
    uint32_t rtp_timestamp = 0;
    uint32_t packet_count = 0;
    uint32_t octet_count = 0;

    static std::optional<RtcpSenderReport> parse(SpanU8 data);
};

}  // namespace csk
