#include <catch2/catch_test_macros.hpp>

#include "codec/h264_parser.h"

using namespace csk;

TEST_CASE("H264 parse Annex-B stream", "[h264]") {
    // Construct a simple Annex-B stream with SPS + PPS + IDR
    std::vector<uint8_t> stream;

    // SPS (type 7)
    stream.insert(stream.end(), {0x00, 0x00, 0x00, 0x01});
    stream.push_back(0x67);  // NAL header: type=7 (SPS)
    stream.insert(stream.end(), {0x42, 0xC0, 0x1F});  // profile/level

    // PPS (type 8)
    stream.insert(stream.end(), {0x00, 0x00, 0x00, 0x01});
    stream.push_back(0x68);  // NAL header: type=8 (PPS)
    stream.insert(stream.end(), {0xCE, 0x38, 0x80});

    // IDR (type 5)
    stream.insert(stream.end(), {0x00, 0x00, 0x00, 0x01});
    stream.push_back(0x65);  // NAL header: type=5 (IDR)
    stream.insert(stream.end(), 50, 0xAA);  // IDR data

    H264Parser parser;
    auto nals = parser.parse(stream);

    REQUIRE(nals.size() == 3);
    REQUIRE(nals[0].is_sps());
    REQUIRE(nals[1].is_pps());
    REQUIRE(nals[2].is_idr());
}

TEST_CASE("H264 depacketize single NAL", "[h264]") {
    H264Parser parser;

    std::vector<uint8_t> payload = {0x65, 0x01, 0x02, 0x03};  // IDR NAL
    auto nal = parser.depacketize_rtp(payload);

    REQUIRE(nal.has_value());
    REQUIRE(nal->type == 5);  // IDR
    REQUIRE(nal->is_idr());
}

TEST_CASE("H264 depacketize FU-A", "[h264]") {
    H264Parser parser;

    // FU-A Start: fu_indicator=0x7C (type=28, nri=3), fu_header=0x85 (S=1, type=5)
    std::vector<uint8_t> frag1 = {0x7C, 0x85, 0x01, 0x02, 0x03};
    auto r1 = parser.depacketize_rtp(frag1);
    REQUIRE(!r1.has_value());  // Not complete yet

    // FU-A Middle: fu_indicator=0x7C, fu_header=0x05 (S=0, E=0, type=5)
    std::vector<uint8_t> frag2 = {0x7C, 0x05, 0x04, 0x05, 0x06};
    auto r2 = parser.depacketize_rtp(frag2);
    REQUIRE(!r2.has_value());

    // FU-A End: fu_indicator=0x7C, fu_header=0x45 (E=1, type=5)
    std::vector<uint8_t> frag3 = {0x7C, 0x45, 0x07, 0x08, 0x09};
    auto r3 = parser.depacketize_rtp(frag3);
    REQUIRE(r3.has_value());
    REQUIRE(r3->type == 5);
    REQUIRE(r3->data.size() == 10);  // 1 (nal header) + 3 + 3 + 3
}
