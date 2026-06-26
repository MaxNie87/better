#include <odin/file_handle.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace odin {

FileHandle::FileHandle(std::FILE *file) noexcept
    : file_(file)
{
}

FileHandle::~FileHandle()
{
    reset();
}

FileHandle::FileHandle(FileHandle &&other) noexcept
    : file_(other.release())
{
}

FileHandle &FileHandle::operator=(FileHandle &&other) noexcept
{
    if (this != &other) {
        reset(other.release());
    }

    return *this;
}

FileHandle FileHandle::open(const std::filesystem::path &path, const char *mode)
{
    return FileHandle(std::fopen(path.string().c_str(), mode));
}

bool FileHandle::valid() const noexcept
{
    return file_ != nullptr;
}

std::FILE *FileHandle::get() const noexcept
{
    return file_;
}

std::FILE *FileHandle::release() noexcept
{
    auto *file = file_;
    file_ = nullptr;
    return file;
}

void FileHandle::reset(std::FILE *file) noexcept
{
    if (file_ != nullptr) {
        std::fclose(file_);
    }

    file_ = file;
}

std::size_t FileHandle::read(void *buffer, std::size_t bytes)
{
    if (!valid() || buffer == nullptr || bytes == 0) {
        return 0;
    }

    return std::fread(buffer, 1, bytes, file_);
}

std::size_t FileHandle::write(const void *buffer, std::size_t bytes)
{
    if (!valid() || buffer == nullptr || bytes == 0) {
        return 0;
    }

    return std::fwrite(buffer, 1, bytes, file_);
}

bool FileHandle::flush()
{
    return valid() && std::fflush(file_) == 0;
}

std::string FileHandle::read_all()
{
    if (!valid()) {
        return {};
    }

    const auto current = std::ftell(file_);
    if (current < 0) {
        return {};
    }

    if (std::fseek(file_, 0, SEEK_END) != 0) {
        return {};
    }

    const auto end = std::ftell(file_);
    if (end < 0) {
        (void)std::fseek(file_, current, SEEK_SET);
        return {};
    }

    if (std::fseek(file_, 0, SEEK_SET) != 0) {
        return {};
    }

    std::string data(static_cast<std::size_t>(end), '\0');
    const auto read = std::fread(data.data(), 1, data.size(), file_);
    data.resize(read);

    (void)std::fseek(file_, current, SEEK_SET);
    return data;
}

}  // namespace odin
