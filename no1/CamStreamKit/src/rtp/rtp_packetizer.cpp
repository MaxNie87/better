#include "rtp/rtp_packetizer.h"

namespace csk {

RtpPacketizer::RtpPacketizer(uint8_t payload_type, uint32_t ssrc)
    : ssrc_(ssrc), pt_(payload_type) {}

RtpPacket RtpPacketizer::make_packet(uint32_t ts, bool marker) {
    RtpPacket pkt;
    pkt.header().payload_type = pt_;
    pkt.header().ssrc = ssrc_;
    pkt.header().timestamp = ts;
    pkt.header().sequence = seq_++;
    pkt.header().marker = marker;
    return pkt;
}

std::vector<RtpPacket> RtpPacketizer::packetize(const NalUnit &nal, uint32_t timestamp) {
    if (nal.data.size() <= MAX_RTP_PAYLOAD) {
        return pack_single(nal, timestamp);
    }
    return pack_fu_a(nal, timestamp);
}

std::vector<RtpPacket> RtpPacketizer::pack_single(const NalUnit &nal, uint32_t ts) {
    auto pkt = make_packet(ts, true);
    pkt.set_payload(nal.data);
    return {pkt};
}

std::vector<RtpPacket> RtpPacketizer::pack_fu_a(const NalUnit &nal, uint32_t ts) {
    std::vector<RtpPacket> packets;

    uint8_t nal_header = nal.data[0];
    uint8_t fu_indicator = (nal_header & 0xE0) | 28;  // FU-A type = 28
    uint8_t nal_type = nal_header & 0x1F;

    const uint8_t *payload = nal.data.data() + 1;
    size_t remaining = nal.data.size() - 1;
    bool first = true;

    while (remaining > 0) {
        size_t chunk_size = std::min(remaining, MAX_RTP_PAYLOAD - 2);
        bool last = (remaining == chunk_size);

        auto pkt = make_packet(ts, last);

        std::vector<uint8_t> fu_payload;
        fu_payload.reserve(2 + chunk_size);
        fu_payload.push_back(fu_indicator);

        uint8_t fu_header = nal_type;
        if (first) fu_header |= 0x80;  // Start bit
        if (last) fu_header |= 0x40;   // End bit
        fu_payload.push_back(fu_header);

        fu_payload.insert(fu_payload.end(), payload, payload + chunk_size);
        pkt.set_payload(std::move(fu_payload));

        packets.push_back(std::move(pkt));

        payload += chunk_size;
        remaining -= chunk_size;
        first = false;
    }

    return packets;
}

}  // namespace csk
