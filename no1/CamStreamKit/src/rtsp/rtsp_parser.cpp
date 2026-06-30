#include "rtsp/rtsp_parser.h"

#include <sstream>

namespace csk {

std::vector<RtspRequest> RtspParser::feed(const std::string &data) {
    buffer_ += data;
    std::vector<RtspRequest> requests;

    while (true) {
        auto req = try_parse();
        if (!req) break;
        requests.push_back(std::move(*req));
    }

    return requests;
}

std::optional<RtspRequest> RtspParser::try_parse() {
    // Find header/body separator
    auto header_end = buffer_.find("\r\n\r\n");
    if (header_end == std::string::npos) return std::nullopt;

    std::string header_section = buffer_.substr(0, header_end);

    // Parse Content-Length to determine body size
    size_t content_length = 0;
    auto cl_pos = header_section.find("Content-Length:");
    if (cl_pos == std::string::npos) cl_pos = header_section.find("content-length:");
    if (cl_pos != std::string::npos) {
        auto val_start = cl_pos + 15;
        while (val_start < header_section.size() && header_section[val_start] == ' ')
            val_start++;
        auto val_end = header_section.find("\r\n", val_start);
        if (val_end == std::string::npos) val_end = header_section.size();
        content_length = std::stoul(header_section.substr(val_start, val_end - val_start));
    }

    size_t total_size = header_end + 4 + content_length;
    if (buffer_.size() < total_size) return std::nullopt;

    // Parse the request
    RtspRequest req;
    std::istringstream stream(header_section);
    std::string line;

    // Request line: METHOD URL RTSP/1.0
    if (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream reqline(line);
        std::string version;
        reqline >> req.method >> req.url >> version;
    }

    // Headers
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            while (!value.empty() && value[0] == ' ') value.erase(0, 1);
            req.headers[key] = value;
        }
    }

    // CSeq
    if (req.headers.count("CSeq")) {
        req.cseq = std::stoul(req.headers["CSeq"]);
    }

    // Body
    if (content_length > 0) {
        req.body = buffer_.substr(header_end + 4, content_length);
    }

    buffer_.erase(0, total_size);
    return req;
}

std::string RtspResponse::serialize() const {
    std::ostringstream ss;
    ss << "RTSP/1.0 " << status_code << " " << reason << "\r\n";
    ss << "CSeq: " << cseq << "\r\n";
    for (auto &[key, value] : headers) {
        ss << key << ": " << value << "\r\n";
    }
    if (!body.empty()) {
        ss << "Content-Length: " << body.size() << "\r\n";
    }
    ss << "\r\n";
    if (!body.empty()) {
        ss << body;
    }
    return ss.str();
}

RtspTransport RtspParser::parse_transport(const std::string &transport_header) {
    RtspTransport transport;

    if (transport_header.find("RTP/AVP/TCP") != std::string::npos) {
        transport.mode = TransportMode::TCP;
        auto interleaved_pos = transport_header.find("interleaved=");
        if (interleaved_pos != std::string::npos) {
            auto val = transport_header.substr(interleaved_pos + 12);
            auto dash = val.find('-');
            if (dash != std::string::npos) {
                transport.rtp_channel = static_cast<uint8_t>(std::stoi(val.substr(0, dash)));
                auto end = val.find(';', dash);
                transport.rtcp_channel = static_cast<uint8_t>(
                    std::stoi(val.substr(dash + 1, end != std::string::npos ? end - dash - 1 : std::string::npos)));
            }
        }
    } else {
        transport.mode = TransportMode::UDP;
        auto cp_pos = transport_header.find("client_port=");
        if (cp_pos != std::string::npos) {
            auto val = transport_header.substr(cp_pos + 12);
            auto dash = val.find('-');
            if (dash != std::string::npos) {
                transport.client_rtp_port = static_cast<uint16_t>(std::stoi(val.substr(0, dash)));
                auto end = val.find(';', dash);
                transport.client_rtcp_port = static_cast<uint16_t>(
                    std::stoi(val.substr(dash + 1, end != std::string::npos ? end - dash - 1 : std::string::npos)));
            }
        }
    }

    return transport;
}

std::string RtspParser::build_transport_response(const RtspTransport &transport,
                                                  const std::string &session_id) {
    std::ostringstream ss;
    if (transport.mode == TransportMode::TCP) {
        ss << "RTP/AVP/TCP;unicast;interleaved=" << static_cast<int>(transport.rtp_channel)
           << "-" << static_cast<int>(transport.rtcp_channel);
    } else {
        ss << "RTP/AVP;unicast;client_port=" << transport.client_rtp_port << "-"
           << transport.client_rtcp_port << ";server_port=" << transport.server_rtp_port
           << "-" << transport.server_rtcp_port;
    }
    return ss.str();
}

std::optional<RtspParser::UrlInfo> RtspParser::parse_url(const std::string &url) {
    UrlInfo info;

    auto scheme_end = url.find("://");
    if (scheme_end == std::string::npos) return std::nullopt;
    info.scheme = url.substr(0, scheme_end);

    auto authority_start = scheme_end + 3;
    auto path_start = url.find('/', authority_start);
    std::string authority;
    if (path_start != std::string::npos) {
        authority = url.substr(authority_start, path_start - authority_start);
        info.path = url.substr(path_start);
    } else {
        authority = url.substr(authority_start);
        info.path = "/";
    }

    // Check for user:pass@
    auto at_pos = authority.find('@');
    std::string host_port;
    if (at_pos != std::string::npos) {
        auto userpass = authority.substr(0, at_pos);
        auto colon = userpass.find(':');
        if (colon != std::string::npos) {
            info.username = userpass.substr(0, colon);
            info.password = userpass.substr(colon + 1);
        } else {
            info.username = userpass;
        }
        host_port = authority.substr(at_pos + 1);
    } else {
        host_port = authority;
    }

    auto colon = host_port.find(':');
    if (colon != std::string::npos) {
        info.host = host_port.substr(0, colon);
        info.port = static_cast<uint16_t>(std::stoi(host_port.substr(colon + 1)));
    } else {
        info.host = host_port;
    }

    return info;
}

}  // namespace csk
