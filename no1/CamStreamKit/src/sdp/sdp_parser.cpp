#include "sdp/sdp_parser.h"

#include <sstream>

namespace csk {

std::optional<SdpSession> SdpParser::parse(const std::string &sdp) {
    SdpSession session;
    SdpMedia *current_media = nullptr;

    std::istringstream stream(sdp);
    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.size() < 2 || line[1] != '=') continue;

        char type = line[0];
        std::string value = line.substr(2);

        switch (type) {
            case 'o':
                session.origin = value;
                break;
            case 's':
                session.session_name = value;
                break;
            case 'c':
                if (!current_media) session.connection = value;
                break;
            case 'm': {
                session.media.emplace_back();
                current_media = &session.media.back();

                // Parse: m=video 0 RTP/AVP 96
                std::istringstream ms(value);
                std::string pt_str;
                ms >> current_media->type >> current_media->port
                   >> current_media->protocol >> pt_str;
                if (!pt_str.empty()) {
                    current_media->payload_type = static_cast<uint8_t>(std::stoi(pt_str));
                }
                break;
            }
            case 'a': {
                if (!current_media) break;
                auto eq_pos = value.find(':');
                std::string attr_name = (eq_pos != std::string::npos)
                                            ? value.substr(0, eq_pos)
                                            : value;
                std::string attr_value = (eq_pos != std::string::npos)
                                             ? value.substr(eq_pos + 1)
                                             : "";

                current_media->attributes[attr_name] = attr_value;

                if (attr_name == "rtpmap") {
                    // e.g. "96 H264/90000"
                    auto space = attr_value.find(' ');
                    if (space != std::string::npos) {
                        auto codec_str = attr_value.substr(space + 1);
                        auto slash = codec_str.find('/');
                        if (slash != std::string::npos) {
                            current_media->codec = codec_str.substr(0, slash);
                            current_media->clock_rate =
                                std::stoul(codec_str.substr(slash + 1));
                        } else {
                            current_media->codec = codec_str;
                        }
                    }
                } else if (attr_name == "fmtp") {
                    auto space = attr_value.find(' ');
                    if (space != std::string::npos) {
                        auto params = attr_value.substr(space + 1);
                        auto sprop_pos = params.find("sprop-parameter-sets=");
                        if (sprop_pos != std::string::npos) {
                            auto start = sprop_pos + 21;
                            auto end = params.find(';', start);
                            current_media->sprop_params =
                                (end != std::string::npos) ? params.substr(start, end - start)
                                                          : params.substr(start);
                        }
                    }
                } else if (attr_name == "control") {
                    current_media->control = attr_value;
                }
                break;
            }
            default:
                break;
        }
    }

    if (session.media.empty()) return std::nullopt;
    return session;
}

std::string SdpParser::build_video_sdp(const std::string &session_id,
                                        const std::string &address,
                                        const std::string &codec,
                                        uint8_t payload_type,
                                        const std::string &sprop_params,
                                        const std::string &control) {
    std::ostringstream ss;
    ss << "v=0\r\n"
       << "o=- " << session_id << " 1 IN IP4 " << address << "\r\n"
       << "s=CamStreamKit\r\n"
       << "c=IN IP4 " << address << "\r\n"
       << "t=0 0\r\n"
       << "m=video 0 RTP/AVP " << static_cast<int>(payload_type) << "\r\n"
       << "a=rtpmap:" << static_cast<int>(payload_type) << " " << codec << "/90000\r\n";

    if (codec == "H264" && !sprop_params.empty()) {
        ss << "a=fmtp:" << static_cast<int>(payload_type)
           << " packetization-mode=1;sprop-parameter-sets=" << sprop_params << "\r\n";
    }

    ss << "a=control:" << control << "\r\n";
    return ss.str();
}

}  // namespace csk
