#pragma once

#include <map>
#include <string>
#include <vector>

namespace csk {

struct SipMessage {
    enum Type { REQUEST, RESPONSE };

    Type type = REQUEST;

    // Request line
    std::string method;
    std::string request_uri;

    // Status line (for responses)
    int status_code = 0;
    std::string reason_phrase;

    // Headers
    std::map<std::string, std::string> headers;
    std::string body;

    std::string via() const { return header("Via"); }
    std::string from() const { return header("From"); }
    std::string to() const { return header("To"); }
    std::string call_id() const { return header("Call-ID"); }
    std::string cseq() const { return header("CSeq"); }
    std::string contact() const { return header("Contact"); }
    std::string content_type() const { return header("Content-Type"); }
    std::string expires() const { return header("Expires"); }

    std::string header(const std::string &name) const;
    void set_header(const std::string &name, const std::string &value);

    static SipMessage parse(const std::string &data);
    std::string serialize() const;

    static std::string extract_user_from_uri(const std::string &uri);
    static std::string extract_tag(const std::string &header_value);
    static std::string extract_branch(const std::string &via);
};

}  // namespace csk
