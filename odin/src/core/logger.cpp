#include <odin/logger.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <string>

namespace odin {
namespace {

spdlog::level::level_enum to_spdlog_level(LogLevel level)
{
    switch (level) {
    case LogLevel::trace:
        return spdlog::level::trace;
    case LogLevel::debug:
        return spdlog::level::debug;
    case LogLevel::info:
        return spdlog::level::info;
    case LogLevel::warn:
        return spdlog::level::warn;
    case LogLevel::error:
        return spdlog::level::err;
    case LogLevel::critical:
        return spdlog::level::critical;
    case LogLevel::off:
        return spdlog::level::off;
    }

    return spdlog::level::info;
}

std::shared_ptr<spdlog::logger> &logger_instance()
{
    static auto instance = [] {
        auto logger = spdlog::stdout_color_mt("odin");
        logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^%l%$] %v");
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::warn);
        spdlog::set_default_logger(logger);
        return logger;
    }();

    return instance;
}

void log_message(LogLevel level, std::string_view message)
{
    auto &logger = logger_instance();
    logger->log(to_spdlog_level(level), "{}", message);
}

}  // namespace

void init_logger(LogLevel level)
{
    auto &logger = logger_instance();
    logger->set_level(to_spdlog_level(level));
}

void set_log_level(LogLevel level)
{
    logger_instance()->set_level(to_spdlog_level(level));
}

void trace(std::string_view message)
{
    log_message(LogLevel::trace, message);
}

void debug(std::string_view message)
{
    log_message(LogLevel::debug, message);
}

void info(std::string_view message)
{
    log_message(LogLevel::info, message);
}

void warn(std::string_view message)
{
    log_message(LogLevel::warn, message);
}

void error(std::string_view message)
{
    log_message(LogLevel::error, message);
}

void critical(std::string_view message)
{
    log_message(LogLevel::critical, message);
}

}  // namespace odin
