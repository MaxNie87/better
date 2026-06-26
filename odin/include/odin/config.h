#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace odin {

class Config {
public:
    bool load(const std::filesystem::path &path);
    bool save(const std::filesystem::path &path) const;

    void set(std::string key, std::string value);
    bool contains(const std::string &key) const;
    std::string get(const std::string &key, std::string default_value = {}) const;

    const std::unordered_map<std::string, std::string> &values() const noexcept;

private:
    static std::string trim(std::string value);

    std::unordered_map<std::string, std::string> values_;
};

}  // namespace odin
