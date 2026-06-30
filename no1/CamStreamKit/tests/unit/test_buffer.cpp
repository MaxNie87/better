#include <catch2/catch_test_macros.hpp>

#include "core/buffer.h"

using namespace csk;

TEST_CASE("Buffer basic operations", "[buffer]") {
    Buffer buf(10);
    REQUIRE(buf.size() == 10);
    REQUIRE(!buf.empty());

    buf.clear();
    REQUIRE(buf.empty());
}

TEST_CASE("Buffer from data", "[buffer]") {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    Buffer buf(data, 4);
    REQUIRE(buf.size() == 4);
    REQUIRE(buf[0] == 0x01);
    REQUIRE(buf[3] == 0x04);
}

TEST_CASE("Buffer append", "[buffer]") {
    Buffer buf;
    uint8_t data[] = {0xAA, 0xBB};
    buf.append(data, 2);
    REQUIRE(buf.size() == 2);
    REQUIRE(buf[0] == 0xAA);

    buf.append(data, 2);
    REQUIRE(buf.size() == 4);
}

TEST_CASE("BufferReader read integers", "[buffer]") {
    uint8_t data[] = {0x80, 0xE0, 0x00, 0x01, 0x00, 0x00, 0x00, 0xA0};
    BufferReader reader({data, 8});

    REQUIRE(reader.read_u8() == 0x80);
    REQUIRE(reader.read_u8() == 0xE0);
    REQUIRE(reader.read_u16_be() == 0x0001);
    REQUIRE(reader.read_u32_be() == 0x000000A0);
    REQUIRE(reader.remaining() == 0);
}

TEST_CASE("BufferReader throws on underflow", "[buffer]") {
    uint8_t data[] = {0x01};
    BufferReader reader({data, 1});

    REQUIRE_NOTHROW(reader.read_u8());
    REQUIRE_THROWS_AS(reader.read_u8(), std::out_of_range);
}

TEST_CASE("BufferWriter write integers", "[buffer]") {
    BufferWriter writer;
    writer.write_u8(0x80);
    writer.write_u16_be(0x1234);
    writer.write_u32_be(0xDEADBEEF);

    auto data = writer.take();
    REQUIRE(data.size() == 7);
    REQUIRE(data[0] == 0x80);
    REQUIRE(data[1] == 0x12);
    REQUIRE(data[2] == 0x34);
    REQUIRE(data[3] == 0xDE);
    REQUIRE(data[4] == 0xAD);
    REQUIRE(data[5] == 0xBE);
    REQUIRE(data[6] == 0xEF);
}
