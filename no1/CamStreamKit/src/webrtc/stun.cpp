#include "webrtc/stun.h"

#include <openssl/hmac.h>

#include <cstring>

namespace csk {

bool StunMessage::is_stun(const uint8_t *data, size_t len) {
    if (len < 20) return false;
    // First two bits must be 0, and magic cookie must match
    if ((data[0] & 0xC0) != 0) return false;
    uint32_t cookie = (uint32_t)((data[4] << 24) | (data[5] << 16) |
                                  (data[6] << 8) | data[7]);
    return cookie == MAGIC_COOKIE;
}

StunMessage StunMessage::parse(SpanU8 data) {
    StunMessage msg;
    msg.type = (uint16_t)((data[0] << 8) | data[1]);
    std::memcpy(msg.transaction_id, data.data() + 8, 12);
    return msg;
}

std::vector<uint8_t> StunMessage::build_success_response(
    const std::string &local_pwd,
    uint32_t mapped_addr, uint16_t mapped_port) const {

    std::vector<uint8_t> resp;
    resp.resize(20);

    // Type: Binding Success Response
    resp[0] = 0x01; resp[1] = 0x01;
    // Magic cookie
    resp[4] = 0x21; resp[5] = 0x12; resp[6] = 0xA4; resp[7] = 0x42;
    // Transaction ID
    std::memcpy(resp.data() + 8, transaction_id, 12);

    // XOR-MAPPED-ADDRESS attribute (type=0x0020)
    uint16_t xport = mapped_port ^ 0x2112;
    uint32_t xaddr = mapped_addr ^ MAGIC_COOKIE;

    uint8_t xma[12] = {};
    xma[0] = 0x00; xma[1] = 0x20;  // Attribute type
    xma[2] = 0x00; xma[3] = 0x08;  // Length = 8
    xma[4] = 0x00;                   // Reserved
    xma[5] = 0x01;                   // Family: IPv4
    xma[6] = (uint8_t)(xport >> 8);
    xma[7] = (uint8_t)(xport & 0xFF);
    xma[8] = (uint8_t)(xaddr >> 24);
    xma[9] = (uint8_t)(xaddr >> 16);
    xma[10] = (uint8_t)(xaddr >> 8);
    xma[11] = (uint8_t)(xaddr & 0xFF);

    resp.insert(resp.end(), xma, xma + 12);

    // MESSAGE-INTEGRITY (HMAC-SHA1 over message up to this attr)
    // First set message length (excluding 20-byte header):
    // current attrs (12) + MESSAGE-INTEGRITY attr header(4) + hmac(20) = 36
    uint16_t msg_len_for_integrity = (uint16_t)(resp.size() - 20 + 24);
    resp[2] = (uint8_t)(msg_len_for_integrity >> 8);
    resp[3] = (uint8_t)(msg_len_for_integrity & 0xFF);

    unsigned int hmac_len = 0;
    uint8_t hmac_out[20];
    HMAC(EVP_sha1(), local_pwd.data(), (int)local_pwd.size(),
         resp.data(), resp.size(), hmac_out, &hmac_len);

    // MESSAGE-INTEGRITY attribute (type=0x0008, length=20)
    uint8_t mi_hdr[4] = {0x00, 0x08, 0x00, 0x14};
    resp.insert(resp.end(), mi_hdr, mi_hdr + 4);
    resp.insert(resp.end(), hmac_out, hmac_out + 20);

    // FINGERPRINT attribute (type=0x8028, length=4)
    uint16_t final_msg_len = (uint16_t)(resp.size() - 20 + 8);
    resp[2] = (uint8_t)(final_msg_len >> 8);
    resp[3] = (uint8_t)(final_msg_len & 0xFF);

    // CRC32 XOR 0x5354554E
    uint32_t crc = 0;
    // Simple CRC32 using zlib-style polynomial
    crc = ~crc;
    for (size_t i = 0; i < resp.size(); i++) {
        crc ^= resp[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    crc = ~crc;
    crc ^= 0x5354554E;

    uint8_t fp_attr[8] = {0x80, 0x28, 0x00, 0x04,
                          (uint8_t)(crc >> 24), (uint8_t)(crc >> 16),
                          (uint8_t)(crc >> 8), (uint8_t)(crc & 0xFF)};
    resp.insert(resp.end(), fp_attr, fp_attr + 8);

    // Update final message length
    uint16_t total_attrs_len = (uint16_t)(resp.size() - 20);
    resp[2] = (uint8_t)(total_attrs_len >> 8);
    resp[3] = (uint8_t)(total_attrs_len & 0xFF);

    return resp;
}

}  // namespace csk
