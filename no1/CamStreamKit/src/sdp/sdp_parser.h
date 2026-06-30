#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace csk {

struct SdpMedia {
    std::string type;          // "video" / "audio"
    uint16_t port = 0;
    std::string protocol;      // "RTP/AVP"
    uint8_t payload_type = 96;
    std::string codec;         // "H264" / "H265"
    uint32_t clock_rate = 90000;
    std::string sprop_params;  // base64 SPS/PPS
    std::string control;       // track control URL
    std::map<std::string, std::string> attributes;
};

struct SdpSession {
    std::string origin;
    std::string session_name;
    std::string connection;
    std::vector<SdpMedia> media;
};

class SdpParser {
public:
    std::optional<SdpSession> parse(const std::string &sdp);
    static std::string build_video_sdp(const std::string &session_id,
                                       const std::string &address,
                                       const std::string &codec,
                                       uint8_t payload_type,
                                       const std::string &sprop_params,
                                       const std::string &control = "trackID=0");
};

}  // namespace csk
