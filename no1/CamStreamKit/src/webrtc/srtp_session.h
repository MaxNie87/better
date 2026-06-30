#pragma once

#include <openssl/hmac.h>

#include <cstdint>
#include <vector>

namespace csk {

// Simplified SRTP encryption using AES-128-CM + HMAC-SHA1-80
class SrtpSession {
public:
    bool init(const uint8_t *key_material, size_t len);

    // Protect RTP packet in-place, appends auth tag
    bool protect_rtp(std::vector<uint8_t> &packet);

private:
    void derive_session_keys(const uint8_t *master_key, const uint8_t *master_salt);
    void aes_cm_encrypt(const uint8_t *key, const uint8_t *iv,
                        uint8_t *data, size_t len);

    uint8_t session_key_[16] = {};
    uint8_t session_salt_[14] = {};
    uint8_t auth_key_[20] = {};
    uint32_t roc_ = 0;
    bool initialized_ = false;
};

}  // namespace csk
