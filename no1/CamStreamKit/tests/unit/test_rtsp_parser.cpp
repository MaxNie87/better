#include <catch2/catch_test_macros.hpp>

#include "rtsp/rtsp_parser.h"

using namespace csk;

TEST_CASE("RTSP parser parse OPTIONS request", "[rtsp]") {
    RtspParser parser;
    std::string data =
        "OPTIONS rtsp://192.168.1.100:554/stream1 RTSP/1.0\r\n"
        "CSeq: 1\r\n"
        "User-Agent: VLC\r\n"
        "\r\n";

    auto requests = parser.feed(data);
    REQUIRE(requests.size() == 1);
    REQUIRE(requests[0].method == "OPTIONS");
    REQUIRE(requests[0].url == "rtsp://192.168.1.100:554/stream1");
    REQUIRE(requests[0].cseq == 1);
}

TEST_CASE("RTSP parser parse DESCRIBE with body", "[rtsp]") {
    RtspParser parser;
    std::string data =
        "DESCRIBE rtsp://localhost/stream1 RTSP/1.0\r\n"
        "CSeq: 2\r\n"
        "Accept: application/sdp\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";

    auto requests = parser.feed(data);
    REQUIRE(requests.size() == 1);
    REQUIRE(requests[0].method == "DESCRIBE");
    REQUIRE(requests[0].cseq == 2);
    REQUIRE(requests[0].body == "hello");
}

TEST_CASE("RTSP parser handles incomplete data", "[rtsp]") {
    RtspParser parser;

    auto r1 = parser.feed("OPTIONS rtsp://x RTSP/1.0\r\n");
    REQUIRE(r1.empty());

    auto r2 = parser.feed("CSeq: 1\r\n\r\n");
    REQUIRE(r2.size() == 1);
    REQUIRE(r2[0].method == "OPTIONS");
}

TEST_CASE("RTSP URL parser", "[rtsp]") {
    auto info = RtspParser::parse_url("rtsp://admin:123456@192.168.1.100:554/stream1");
    REQUIRE(info.has_value());
    REQUIRE(info->scheme == "rtsp");
    REQUIRE(info->username == "admin");
    REQUIRE(info->password == "123456");
    REQUIRE(info->host == "192.168.1.100");
    REQUIRE(info->port == 554);
    REQUIRE(info->path == "/stream1");
}

TEST_CASE("RTSP URL parser without auth", "[rtsp]") {
    auto info = RtspParser::parse_url("rtsp://192.168.1.100/stream1");
    REQUIRE(info.has_value());
    REQUIRE(info->username.empty());
    REQUIRE(info->host == "192.168.1.100");
    REQUIRE(info->port == 554);
    REQUIRE(info->path == "/stream1");
}

TEST_CASE("RTSP transport parser TCP", "[rtsp]") {
    auto transport = RtspParser::parse_transport(
        "RTP/AVP/TCP;unicast;interleaved=0-1");
    REQUIRE(transport.mode == TransportMode::TCP);
    REQUIRE(transport.rtp_channel == 0);
    REQUIRE(transport.rtcp_channel == 1);
}

TEST_CASE("RTSP transport parser UDP", "[rtsp]") {
    auto transport = RtspParser::parse_transport(
        "RTP/AVP;unicast;client_port=5000-5001");
    REQUIRE(transport.mode == TransportMode::UDP);
    REQUIRE(transport.client_rtp_port == 5000);
    REQUIRE(transport.client_rtcp_port == 5001);
}

TEST_CASE("RTSP response serialize", "[rtsp]") {
    RtspResponse resp;
    resp.status_code = 200;
    resp.reason = "OK";
    resp.cseq = 1;
    resp.headers["Public"] = "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN";

    auto str = resp.serialize();
    REQUIRE(str.find("RTSP/1.0 200 OK") != std::string::npos);
    REQUIRE(str.find("CSeq: 1") != std::string::npos);
    REQUIRE(str.find("Public: OPTIONS") != std::string::npos);
}
