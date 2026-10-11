#ifndef DIAGNOSTICS_LOG_LEVEL_HPP
#define DIAGNOSTICS_LOG_LEVEL_HPP

#include <cstdint>
#include <string_view>

namespace Diagnostics {

enum class LogLevel : std::uint8_t {
    Trace,
    Debug,
    Info,
    Warn,
    Error
};

constexpr std::string_view LogLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Trace:
        return "TRACE";
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

} // namespace Diagnostics

#endif // DIAGNOSTICS_LOG_LEVEL_HPP
