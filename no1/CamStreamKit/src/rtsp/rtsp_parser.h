#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace csk {

struct RtspRequest {
    std::string method;
    std::string url;
    uint32_t cseq = 0;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct RtspResponse {
    uint16_t status_code = 200;
    std::string reason = "OK";
    uint32_t cseq = 0;
    std::map<std::string, std::string> headers;
    std::string body;

    std::string serialize() const;
};

enum class TransportMode { TCP, UDP };

struct RtspTransport {
    TransportMode mode = TransportMode::TCP;
    uint8_t rtp_channel = 0;
    uint8_t rtcp_channel = 1;
    uint16_t client_rtp_port = 0;
    uint16_t client_rtcp_port = 0;
    uint16_t server_rtp_port = 0;
    uint16_t server_rtcp_port = 0;
};

class RtspParser {
public:
    // Feed raw TCP data, returns completed requests
    std::vector<RtspRequest> feed(const std::string &data);

    static RtspTransport parse_transport(const std::string &transport_header);
    static std::string build_transport_response(const RtspTransport &transport,
                                                 const std::string &session_id);

    // Parse RTSP URL: rtsp://user:pass@host:port/path
    struct UrlInfo {
        std::string scheme;
        std::string username;
        std::string password;
        std::string host;
        uint16_t port = 554;
        std::string path;
    };
    static std::optional<UrlInfo> parse_url(const std::string &url);

private:
    std::optional<RtspRequest> try_parse();
    std::string buffer_;
};

}  // namespace csk
