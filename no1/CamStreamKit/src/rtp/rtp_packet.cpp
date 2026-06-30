#include "rtp/rtp_packet.h"

#include "core/buffer.h"

namespace csk {

std::optional<RtpPacket> RtpPacket::parse(SpanU8 data) {
    if (data.size() < 12) return std::nullopt;

    BufferReader reader(data);
    RtpPacket pkt;

    uint8_t byte0 = reader.read_u8();
    pkt.header_.version = (byte0 >> 6) & 0x03;
    pkt.header_.padding = (byte0 >> 5) & 0x01;
    pkt.header_.extension = (byte0 >> 4) & 0x01;
    pkt.header_.csrc_count = byte0 & 0x0F;

    if (pkt.header_.version != 2) return std::nullopt;

    uint8_t byte1 = reader.read_u8();
    pkt.header_.marker = (byte1 >> 7) & 0x01;
    pkt.header_.payload_type = byte1 & 0x7F;

    pkt.header_.sequence = reader.read_u16_be();
    pkt.header_.timestamp = reader.read_u32_be();
    pkt.header_.ssrc = reader.read_u32_be();

    // Skip CSRC list
    size_t csrc_bytes = pkt.header_.csrc_count * 4;
    if (reader.remaining() < csrc_bytes) return std::nullopt;
    reader.skip(csrc_bytes);

    // Skip extension header if present
    if (pkt.header_.extension) {
        if (reader.remaining() < 4) return std::nullopt;
        reader.skip(2);  // extension profile
        uint16_t ext_len = reader.read_u16_be();
        size_t ext_bytes = ext_len * 4;
        if (reader.remaining() < ext_bytes) return std::nullopt;
        reader.skip(ext_bytes);
    }

    // Remaining is payload
    size_t payload_size = reader.remaining();
    if (pkt.header_.padding && payload_size > 0) {
        uint8_t pad_len = data[data.size() - 1];
        if (pad_len > payload_size) return std::nullopt;
        payload_size -= pad_len;
    }

    auto payload_span = reader.read_bytes(payload_size);
    pkt.payload_.assign(payload_span.begin(), payload_span.end());

    return pkt;
}

std::vector<uint8_t> RtpPacket::serialize() const {
    BufferWriter writer(12 + payload_.size());

    uint8_t byte0 = (header_.version << 6) | (header_.padding << 5) |
                    (header_.extension << 4) | (header_.csrc_count & 0x0F);
    writer.write_u8(byte0);

    uint8_t byte1 = (header_.marker << 7) | (header_.payload_type & 0x7F);
    writer.write_u8(byte1);

    writer.write_u16_be(header_.sequence);
    writer.write_u32_be(header_.timestamp);
    writer.write_u32_be(header_.ssrc);

    writer.write_bytes(SpanU8(payload_.data(), payload_.size()));

    return writer.take();
}

std::optional<RtcpSenderReport> RtcpSenderReport::parse(SpanU8 data) {
    if (data.size() < 28) return std::nullopt;

    BufferReader reader(data);

    uint8_t byte0 = reader.read_u8();
    uint8_t version = (byte0 >> 6) & 0x03;
    if (version != 2) return std::nullopt;

    uint8_t pt = reader.read_u8();
    if (pt != 200) return std::nullopt;  // SR packet type

    reader.skip(2);  // length

    RtcpSenderReport sr;
    sr.ssrc = reader.read_u32_be();

    uint32_t ntp_hi = reader.read_u32_be();
    uint32_t ntp_lo = reader.read_u32_be();
    sr.ntp_timestamp = (static_cast<uint64_t>(ntp_hi) << 32) | ntp_lo;

    sr.rtp_timestamp = reader.read_u32_be();
    sr.packet_count = reader.read_u32_be();
    sr.octet_count = reader.read_u32_be();

    return sr;
}

}  // namespace csk
