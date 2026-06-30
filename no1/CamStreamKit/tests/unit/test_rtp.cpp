#include <catch2/catch_test_macros.hpp>

#include "rtp/rtp_packet.h"
#include "rtp/rtp_packetizer.h"

using namespace csk;

TEST_CASE("RTP packet parse", "[rtp]") {
    // Construct a minimal RTP packet: V=2, PT=96, Seq=1, TS=160, SSRC=0x12345678
    uint8_t data[] = {
        0x80, 0x60,              // V=2, P=0, X=0, CC=0, M=0, PT=96
        0x00, 0x01,              // Seq=1
        0x00, 0x00, 0x00, 0xA0, // Timestamp=160
        0x12, 0x34, 0x56, 0x78, // SSRC
        0xAA, 0xBB, 0xCC        // Payload
    };

    auto pkt = RtpPacket::parse({data, sizeof(data)});
    REQUIRE(pkt.has_value());
    REQUIRE(pkt->header().version == 2);
    REQUIRE(pkt->header().payload_type == 96);
    REQUIRE(pkt->header().sequence == 1);
    REQUIRE(pkt->header().timestamp == 160);
    REQUIRE(pkt->header().ssrc == 0x12345678);
    REQUIRE(pkt->header().marker == false);
    REQUIRE(pkt->payload().size() == 3);
}

TEST_CASE("RTP packet serialize roundtrip", "[rtp]") {
    RtpPacket pkt;
    pkt.header().payload_type = 96;
    pkt.header().sequence = 42;
    pkt.header().timestamp = 90000;
    pkt.header().ssrc = 0xAABBCCDD;
    pkt.header().marker = true;

    std::vector<uint8_t> payload = {0x01, 0x02, 0x03, 0x04, 0x05};
    pkt.set_payload(payload);

    auto serialized = pkt.serialize();
    auto parsed = RtpPacket::parse(serialized);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->header().payload_type == 96);
    REQUIRE(parsed->header().sequence == 42);
    REQUIRE(parsed->header().timestamp == 90000);
    REQUIRE(parsed->header().ssrc == 0xAABBCCDD);
    REQUIRE(parsed->header().marker == true);
    REQUIRE(parsed->payload().size() == 5);
}

TEST_CASE("RTP packetizer single NAL", "[rtp]") {
    RtpPacketizer packetizer(96, 0x11223344);

    NalUnit nal;
    nal.type = 5;  // IDR
    nal.data.resize(100, 0xAA);
    nal.data[0] = 0x65;  // IDR NAL header

    auto packets = packetizer.packetize(nal, 90000);
    REQUIRE(packets.size() == 1);
    REQUIRE(packets[0].header().marker == true);
    REQUIRE(packets[0].header().ssrc == 0x11223344);
    REQUIRE(packets[0].payload().size() == 100);
}

TEST_CASE("RTP packetizer FU-A fragmentation", "[rtp]") {
    RtpPacketizer packetizer(96, 0x11223344);

    NalUnit nal;
    nal.type = 5;  // IDR
    nal.data.resize(5000, 0xBB);
    nal.data[0] = 0x65;  // IDR NAL header

    auto packets = packetizer.packetize(nal, 90000);
    REQUIRE(packets.size() > 1);

    // Only last packet should have marker bit set
    for (size_t i = 0; i < packets.size() - 1; ++i) {
        REQUIRE(packets[i].header().marker == false);
    }
    REQUIRE(packets.back().header().marker == true);

    // Sequence numbers should be consecutive
    for (size_t i = 1; i < packets.size(); ++i) {
        REQUIRE(packets[i].header().sequence == packets[i - 1].header().sequence + 1);
    }
}
