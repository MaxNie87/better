#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/buffer.h"

namespace csk {

// Minimal STUN message handling for ICE connectivity checks
struct StunMessage {
    static constexpr uint16_t BINDING_REQUEST = 0x0001;
    static constexpr uint16_t BINDING_RESPONSE = 0x0101;
    static constexpr uint32_t MAGIC_COOKIE = 0x2112A442;

    uint16_t type = 0;
    uint8_t transaction_id[12] = {};

    static bool is_stun(const uint8_t *data, size_t len);
    static StunMessage parse(SpanU8 data);

    // Build a STUN Binding Success Response with XOR-MAPPED-ADDRESS
    std::vector<uint8_t> build_success_response(
        const std::string &local_pwd,
        uint32_t mapped_addr, uint16_t mapped_port) const;
};

}  // namespace csk
