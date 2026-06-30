#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <string>

#define CSK_LOG_T(...) SPDLOG_TRACE(__VA_ARGS__)
#define CSK_LOG_D(...) SPDLOG_DEBUG(__VA_ARGS__)
#define CSK_LOG_I(...) SPDLOG_INFO(__VA_ARGS__)
#define CSK_LOG_W(...) SPDLOG_WARN(__VA_ARGS__)
#define CSK_LOG_E(...) SPDLOG_ERROR(__VA_ARGS__)
#define CSK_LOG_C(...) SPDLOG_CRITICAL(__VA_ARGS__)

namespace csk {

struct LogConfig {
    std::string level = "info";
    std::string file = "";
    size_t max_file_size = 10 * 1024 * 1024;  // 10MB
    size_t max_files = 5;
};

inline void setup_logger(const LogConfig &config) {
    std::vector<spdlog::sink_ptr> sinks;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sinks.push_back(console_sink);

    if (!config.file.empty()) {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            config.file, config.max_file_size, config.max_files);
        sinks.push_back(file_sink);
    }

    auto logger = std::make_shared<spdlog::logger>("csk", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::from_str(config.level));
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    spdlog::set_default_logger(logger);
}

}  // namespace csk
