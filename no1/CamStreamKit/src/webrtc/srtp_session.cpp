#include "webrtc/srtp_session.h"

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <cstring>

namespace csk {

// SRTP key derivation as per RFC 3711 Section 4.3
static void kdf(const uint8_t *master_key, const uint8_t *master_salt,
                uint8_t label, uint8_t *out, size_t out_len) {
    uint8_t x[14] = {};
    std::memcpy(x, master_salt, 14);
    x[7] ^= label;

    uint8_t iv[16] = {};
    std::memcpy(iv, x, 14);

    // AES-CM with master_key to derive session key
    AES_KEY aes_key;
    AES_set_encrypt_key(master_key, 128, &aes_key);

    uint8_t block[16];
    for (size_t i = 0; i < out_len; i += 16) {
        iv[14] = (uint8_t)((i / 16) >> 8);
        iv[15] = (uint8_t)(i / 16);
        AES_encrypt(iv, block, &aes_key);
        size_t copy_len = std::min<size_t>(16, out_len - i);
        std::memcpy(out + i, block, copy_len);
    }
}

bool SrtpSession::init(const uint8_t *key_material, size_t len) {
    // DTLS-SRTP exports 2*(16 key + 14 salt) = 60 bytes
    if (len < 60) return false;

    // For server sending: use server write key/salt
    const uint8_t *client_key = key_material;
    const uint8_t *server_key = key_material + 16;
    const uint8_t *client_salt = key_material + 32;
    const uint8_t *server_salt = key_material + 32 + 14;

    derive_session_keys(server_key, server_salt);
    (void)client_key;
    (void)client_salt;

    initialized_ = true;
    return true;
}

void SrtpSession::derive_session_keys(const uint8_t *master_key, const uint8_t *master_salt) {
    kdf(master_key, master_salt, 0x00, session_key_, 16);
    kdf(master_key, master_salt, 0x01, auth_key_, 20);
    kdf(master_key, master_salt, 0x02, session_salt_, 14);
}

void SrtpSession::aes_cm_encrypt(const uint8_t *key, const uint8_t *iv,
                                  uint8_t *data, size_t len) {
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);

    uint8_t counter[16];
    std::memcpy(counter, iv, 16);

    for (size_t i = 0; i < len; i += 16) {
        uint8_t keystream[16];
        AES_encrypt(counter, keystream, &aes_key);

        size_t block_len = std::min<size_t>(16, len - i);
        for (size_t j = 0; j < block_len; j++) {
            data[i + j] ^= keystream[j];
        }

        // Increment counter
        for (int k = 15; k >= 0; k--) {
            if (++counter[k] != 0) break;
        }
    }
}

bool SrtpSession::protect_rtp(std::vector<uint8_t> &packet) {
    if (!initialized_ || packet.size() < 12) return false;

    uint16_t seq = (uint16_t)((packet[2] << 8) | packet[3]);
    uint32_t ssrc = (uint32_t)((packet[8] << 24) | (packet[9] << 16) |
                               (packet[10] << 8) | packet[11]);

    // Compute IV for AES-CM (RFC 3711 Section 4.1.1)
    // IV layout: [0:3]=0 | [4:7]=SSRC | [8:11]=ROC | [12:13]=SEQ | [14:15]=0
    uint8_t iv[16] = {};
    iv[4] = (ssrc >> 24) & 0xFF;
    iv[5] = (ssrc >> 16) & 0xFF;
    iv[6] = (ssrc >> 8) & 0xFF;
    iv[7] = ssrc & 0xFF;
    iv[8] = (roc_ >> 24) & 0xFF;
    iv[9] = (roc_ >> 16) & 0xFF;
    iv[10] = (roc_ >> 8) & 0xFF;
    iv[11] = roc_ & 0xFF;
    iv[12] = (seq >> 8) & 0xFF;
    iv[13] = seq & 0xFF;

    for (int i = 0; i < 14; i++) iv[i] ^= session_salt_[i];

    // Encrypt payload (skip 12-byte RTP header + CSRC + extensions)
    uint8_t cc = packet[0] & 0x0F;
    size_t header_len = 12 + cc * 4;
    bool has_ext = (packet[0] & 0x10) != 0;
    if (has_ext && packet.size() > header_len + 4) {
        uint16_t ext_len = (uint16_t)((packet[header_len + 2] << 8) | packet[header_len + 3]);
        header_len += 4 + ext_len * 4;
    }

    if (header_len < packet.size()) {
        aes_cm_encrypt(session_key_, iv, packet.data() + header_len,
                       packet.size() - header_len);
    }

    // HMAC-SHA1 auth tag (10 bytes / 80 bits)
    // Auth covers: RTP header + encrypted payload + ROC
    uint8_t roc_be[4] = {(uint8_t)(roc_ >> 24), (uint8_t)(roc_ >> 16),
                         (uint8_t)(roc_ >> 8), (uint8_t)roc_};
    unsigned int hmac_len = 0;
    uint8_t hmac_out[20];

    HMAC_CTX *hmac_ctx = HMAC_CTX_new();
    HMAC_Init_ex(hmac_ctx, auth_key_, 20, EVP_sha1(), nullptr);
    HMAC_Update(hmac_ctx, packet.data(), packet.size());
    HMAC_Update(hmac_ctx, roc_be, 4);
    HMAC_Final(hmac_ctx, hmac_out, &hmac_len);
    HMAC_CTX_free(hmac_ctx);

    // Append 10-byte tag
    packet.insert(packet.end(), hmac_out, hmac_out + 10);
    return true;
}

}  // namespace csk
