#pragma once

#include <string_view>

namespace odin {

enum class LogLevel {
    trace,
    debug,
    info,
    warn,
    error,
    critical,
    off,
};

void init_logger(LogLevel level = LogLevel::info);
void set_log_level(LogLevel level);

void trace(std::string_view message);
void debug(std::string_view message);
void info(std::string_view message);
void warn(std::string_view message);
void error(std::string_view message);
void critical(std::string_view message);

}  // namespace odin

#define ODIN_LOG_TRACE(message) ::odin::trace(message)
#define ODIN_LOG_DEBUG(message) ::odin::debug(message)
#define ODIN_LOG_INFO(message) ::odin::info(message)
#define ODIN_LOG_WARN(message) ::odin::warn(message)
#define ODIN_LOG_ERROR(message) ::odin::error(message)
#define ODIN_LOG_CRITICAL(message) ::odin::critical(message)
