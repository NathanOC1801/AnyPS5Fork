#ifndef DIAGNOSTICS_LOGGER_HPP
#define DIAGNOSTICS_LOGGER_HPP

#include <diagnostics/LogChannel.hpp>
#include <diagnostics/LogLevel.hpp>

#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>

namespace Diagnostics {

struct LogEntry {
    std::string timestamp;
    std::uint64_t threadId{0};
    LogChannel channel{LogChannel::General};
    LogLevel level{LogLevel::Info};
    std::string message;
};

class Logger {
public:
    static Logger& GetInstance();

    void Initialize(const std::string& logFilePath = "anyps5_trace.log", bool enableConsoleOutput = true, bool enableFileOutput = true);
    void SetLogLevel(LogLevel minLevel);
    LogLevel GetLogLevel() const;

    void SetFileLoggingEnabled(bool enabled);
    bool IsFileLoggingEnabled() const;

    void Log(LogChannel channel, LogLevel level, std::string_view message);
    void DumpRingBuffer();
    void Flush();

    std::vector<LogEntry> GetRingBufferEntries();
    void ClearRingBuffer();

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string FormatLogLine(const LogEntry& entry) const;

    mutable std::mutex mutex;
    std::ofstream fileStream;
    std::string logFilePath{"anyps5_trace.log"};
    LogLevel minLogLevel{LogLevel::Info};
    bool consoleOutputEnabled{true};
    bool fileOutputEnabled{true};

    static constexpr std::size_t RingBufferCapacity = 500;
    std::vector<LogEntry> ringBuffer;
    std::size_t ringBufferHead{0};
    std::size_t ringBufferSize{0};
};

} // namespace Diagnostics

#define LOG_TRACE(channel, msg) ::Diagnostics::Logger::GetInstance().Log(channel, ::Diagnostics::LogLevel::Trace, msg)
#define LOG_DEBUG(channel, msg) ::Diagnostics::Logger::GetInstance().Log(channel, ::Diagnostics::LogLevel::Debug, msg)
#define LOG_INFO(channel, msg) ::Diagnostics::Logger::GetInstance().Log(channel, ::Diagnostics::LogLevel::Info, msg)
#define LOG_WARN(channel, msg) ::Diagnostics::Logger::GetInstance().Log(channel, ::Diagnostics::LogLevel::Warn, msg)
#define LOG_ERROR(channel, msg) ::Diagnostics::Logger::GetInstance().Log(channel, ::Diagnostics::LogLevel::Error, msg)

#define LOG_CRASH_AND_THROW(channel, msg) \
    do { \
        ::Diagnostics::Logger::GetInstance().Log(channel, ::Diagnostics::LogLevel::Error, msg); \
        ::Diagnostics::Logger::GetInstance().DumpRingBuffer(); \
        throw std::runtime_error(msg); \
    } while (0)

#endif // DIAGNOSTICS_LOGGER_HPP
