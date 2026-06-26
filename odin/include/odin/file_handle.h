#pragma once

#include <cstdio>
#include <cstddef>
#include <filesystem>
#include <string>

namespace odin {

class FileHandle {
public:
    FileHandle() noexcept = default;
    explicit FileHandle(std::FILE *file) noexcept;
    ~FileHandle();

    FileHandle(const FileHandle &) = delete;
    FileHandle &operator=(const FileHandle &) = delete;

    FileHandle(FileHandle &&other) noexcept;
    FileHandle &operator=(FileHandle &&other) noexcept;

    static FileHandle open(const std::filesystem::path &path, const char *mode);

    bool valid() const noexcept;
    std::FILE *get() const noexcept;
    std::FILE *release() noexcept;
    void reset(std::FILE *file = nullptr) noexcept;

    std::size_t read(void *buffer, std::size_t bytes);
    std::size_t write(const void *buffer, std::size_t bytes);
    bool flush();
    std::string read_all();

private:
    std::FILE *file_{nullptr};
};

}  // namespace odin
