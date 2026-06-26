#include <odin/socket.h>

#ifndef _WIN32

#include <sys/socket.h>
#include <unistd.h>

namespace odin {

Socket::Socket(native_handle_type handle) noexcept
    : handle_(handle)
{
}

Socket::~Socket()
{
    close();
}

Socket::Socket(Socket &&other) noexcept
    : handle_(other.release())
{
}

Socket &Socket::operator=(Socket &&other) noexcept
{
    if (this != &other) {
        close();
        handle_ = other.release();
    }

    return *this;
}

Socket Socket::create(int domain, int type, int protocol)
{
    return Socket(::socket(domain, type, protocol));
}

bool Socket::valid() const noexcept
{
    return handle_ != invalid_handle;
}

Socket::native_handle_type Socket::native_handle() const noexcept
{
    return handle_;
}

Socket::native_handle_type Socket::release() noexcept
{
    const auto handle = handle_;
    handle_ = invalid_handle;
    return handle;
}

void Socket::reset(native_handle_type handle) noexcept
{
    if (handle_ != invalid_handle) {
        (void)::close(handle_);
    }

    handle_ = handle;
}

bool Socket::close() noexcept
{
    if (handle_ == invalid_handle) {
        return true;
    }

    const auto result = ::close(handle_);
    handle_ = invalid_handle;
    return result == 0;
}

}  // namespace odin

#else

namespace odin {

Socket::Socket(native_handle_type) noexcept {}
Socket::~Socket() = default;
Socket::Socket(Socket &&) noexcept = default;
Socket &Socket::operator=(Socket &&) noexcept = default;
Socket Socket::create(int, int, int)
{
    return Socket{};
}
bool Socket::valid() const noexcept
{
    return false;
}
Socket::native_handle_type Socket::native_handle() const noexcept
{
    return invalid_handle;
}
Socket::native_handle_type Socket::release() noexcept
{
    return invalid_handle;
}
void Socket::reset(native_handle_type) noexcept {}
bool Socket::close() noexcept
{
    return true;
}

}  // namespace odin

#endif
