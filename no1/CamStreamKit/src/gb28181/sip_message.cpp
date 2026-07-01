#include "gb28181/sip_message.h"

#include <algorithm>
#include <sstream>

namespace csk {

std::string SipMessage::header(const std::string &name) const {
    auto it = headers.find(name);
    if (it != headers.end()) return it->second;
    // Case-insensitive fallback
    for (auto &[k, v] : headers) {
        std::string lower_k = k, lower_name = name;
        std::transform(lower_k.begin(), lower_k.end(), lower_k.begin(), ::tolower);
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (lower_k == lower_name) return v;
    }
    return "";
}

void SipMessage::set_header(const std::string &name, const std::string &value) {
    headers[name] = value;
}

SipMessage SipMessage::parse(const std::string &data) {
    SipMessage msg;
    std::istringstream stream(data);
    std::string first_line;
    std::getline(stream, first_line);
    if (!first_line.empty() && first_line.back() == '\r') first_line.pop_back();

    if (first_line.find("SIP/2.0") == 0) {
        msg.type = RESPONSE;
        auto sp1 = first_line.find(' ');
        auto sp2 = first_line.find(' ', sp1 + 1);
        if (sp1 != std::string::npos) {
            msg.status_code = std::stoi(first_line.substr(sp1 + 1, sp2 - sp1 - 1));
            if (sp2 != std::string::npos) {
                msg.reason_phrase = first_line.substr(sp2 + 1);
            }
        }
    } else {
        msg.type = REQUEST;
        auto sp1 = first_line.find(' ');
        auto sp2 = first_line.find(' ', sp1 + 1);
        if (sp1 != std::string::npos) {
            msg.method = first_line.substr(0, sp1);
            if (sp2 != std::string::npos) {
                msg.request_uri = first_line.substr(sp1 + 1, sp2 - sp1 - 1);
            }
        }
    }

    // Parse headers
    std::string line;
    size_t content_length = 0;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;

        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            while (!value.empty() && value[0] == ' ') value.erase(0, 1);
            msg.headers[key] = value;

            std::string lower_key = key;
            std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
            if (lower_key == "content-length") {
                content_length = std::stoul(value);
            }
        }
    }

    // Read body
    if (content_length > 0) {
        std::string body_data;
        body_data.resize(content_length);
        stream.read(&body_data[0], content_length);
        msg.body = body_data;
    } else {
        // Read remaining as body
        std::string remaining;
        std::getline(stream, remaining, '\0');
        if (!remaining.empty()) msg.body = remaining;
    }

    return msg;
}

std::string SipMessage::serialize() const {
    std::ostringstream ss;

    if (type == RESPONSE) {
        ss << "SIP/2.0 " << status_code << " " << reason_phrase << "\r\n";
    } else {
        ss << method << " " << request_uri << " SIP/2.0\r\n";
    }

    for (auto &[key, value] : headers) {
        ss << key << ": " << value << "\r\n";
    }

    if (!body.empty()) {
        ss << "Content-Length: " << body.size() << "\r\n";
    } else {
        ss << "Content-Length: 0\r\n";
    }

    ss << "\r\n";
    ss << body;
    return ss.str();
}

std::string SipMessage::extract_user_from_uri(const std::string &uri) {
    // Extract user from "sip:user@host" or "<sip:user@host>"
    auto sip_pos = uri.find("sip:");
    if (sip_pos == std::string::npos) return "";
    auto start = sip_pos + 4;
    auto at_pos = uri.find('@', start);
    if (at_pos == std::string::npos) {
        auto end = uri.find_first_of(">; ", start);
        return uri.substr(start, end - start);
    }
    return uri.substr(start, at_pos - start);
}

std::string SipMessage::extract_tag(const std::string &header_value) {
    auto pos = header_value.find("tag=");
    if (pos == std::string::npos) return "";
    pos += 4;
    auto end = header_value.find_first_of(";> ", pos);
    return header_value.substr(pos, end - pos);
}

std::string SipMessage::extract_branch(const std::string &via) {
    auto pos = via.find("branch=");
    if (pos == std::string::npos) return "";
    pos += 7;
    auto end = via.find_first_of(";, ", pos);
    return via.substr(pos, end - pos);
}

}  // namespace csk
