#pragma once

#include <cstdint>

namespace odin {

class Socket {
public:
    using native_handle_type = int;

    static constexpr native_handle_type invalid_handle = -1;

    Socket() noexcept = default;
    explicit Socket(native_handle_type handle) noexcept;
    ~Socket();

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;

    Socket(Socket &&other) noexcept;
    Socket &operator=(Socket &&other) noexcept;

    static Socket create(int domain, int type, int protocol = 0);

    bool valid() const noexcept;
    native_handle_type native_handle() const noexcept;
    native_handle_type release() noexcept;
    void reset(native_handle_type handle = invalid_handle) noexcept;
    bool close() noexcept;

private:
    native_handle_type handle_{invalid_handle};
};

}  // namespace odin
