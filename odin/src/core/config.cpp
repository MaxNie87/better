#include <odin/config.h>

#include <cctype>
#include <fstream>
#include <string>
#include <utility>

namespace odin {
namespace {

std::string trim_left(std::string value)
{
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.erase(value.begin());
    }

    return value;
}

std::string trim_right(std::string value)
{
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }

    return value;
}

}  // namespace

std::string Config::trim(std::string value)
{
    return trim_right(trim_left(std::move(value)));
}

bool Config::load(const std::filesystem::path &path)
{
    std::ifstream input(path);
    if (!input) {
        return false;
    }

    values_.clear();

    std::string line;
    while (std::getline(input, line)) {
        line = trim(std::move(line));
        if (line.empty() || line.front() == '#' || line.front() == ';' ||
            line.front() == '[') {
            continue;
        }

        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }

        auto key = trim(line.substr(0, separator));
        auto value = trim(line.substr(separator + 1));
        if (!key.empty()) {
            values_[std::move(key)] = std::move(value);
        }
    }

    return true;
}

bool Config::save(const std::filesystem::path &path) const
{
    std::ofstream output(path);
    if (!output) {
        return false;
    }

    for (const auto &[key, value] : values_) {
        output << key << " = " << value << '\n';
    }

    return true;
}

void Config::set(std::string key, std::string value)
{
    values_[std::move(key)] = std::move(value);
}

bool Config::contains(const std::string &key) const
{
    return values_.find(key) != values_.end();
}

std::string Config::get(const std::string &key, std::string default_value) const
{
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return default_value;
    }

    return it->second;
}

const std::unordered_map<std::string, std::string> &Config::values() const noexcept
{
    return values_;
}

}  // namespace odin
