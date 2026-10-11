#include <diagnostics/Logger.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace Diagnostics {

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    ringBuffer.resize(RingBufferCapacity);
    Initialize("anyps5_trace.log", true, true);
}

Logger::~Logger() {
    if (fileStream.is_open()) {
        fileStream.flush();
        fileStream.close();
    }
}

void Logger::Initialize(const std::string& path, bool enableConsoleOutput, bool enableFileOutput) {
    std::lock_guard<std::mutex> lock(mutex);
    if (fileStream.is_open()) {
        fileStream.close();
    }
    logFilePath = path;
    consoleOutputEnabled = enableConsoleOutput;
    fileOutputEnabled = enableFileOutput;
}

void Logger::SetLogLevel(LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(mutex);
    minLogLevel = minLevel;
}

LogLevel Logger::GetLogLevel() const {
    std::lock_guard<std::mutex> lock(mutex);
    return minLogLevel;
}

void Logger::SetFileLoggingEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex);
    fileOutputEnabled = enabled;
    if (!fileOutputEnabled && fileStream.is_open()) {
        fileStream.close();
    }
}

bool Logger::IsFileLoggingEnabled() const {
    std::lock_guard<std::mutex> lock(mutex);
    return fileOutputEnabled;
}

std::string Logger::FormatLogLine(const LogEntry& entry) const {
    std::ostringstream ss;
    ss << "[" << entry.timestamp << "] "
       << "[TID:" << entry.threadId << "] "
       << LogChannelToString(entry.channel) << " "
       << "[" << LogLevelToString(entry.level) << "] "
       << entry.message;
    return ss.str();
}

void Logger::Log(LogChannel channel, LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(mutex);

    if (static_cast<std::uint8_t>(level) < static_cast<std::uint8_t>(minLogLevel)) {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto timeT = std::chrono::system_clock::to_time_t(now);
    const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()) % 1000000;

    std::tm timeInfo{};
#if defined(_WIN32)
    localtime_s(&timeInfo, &timeT);
#else
    localtime_r(&timeT, &timeInfo);
#endif

    std::ostringstream timestampStream;
    timestampStream << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S")
                    << "." << std::setfill('0') << std::setw(6) << microseconds.count();

    const std::uint64_t threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());

    LogEntry entry;
    entry.timestamp = timestampStream.str();
    entry.threadId = threadId;
    entry.channel = channel;
    entry.level = level;
    entry.message = std::string(message);

    if (ringBufferSize < RingBufferCapacity) {
        ringBuffer[(ringBufferHead + ringBufferSize) % RingBufferCapacity] = entry;
        ringBufferSize++;
    } else {
        ringBuffer[ringBufferHead] = entry;
        ringBufferHead = (ringBufferHead + 1) % RingBufferCapacity;
    }

    const std::string line = FormatLogLine(entry);

    if (consoleOutputEnabled) {
        if (level == LogLevel::Error || level == LogLevel::Warn) {
            std::cerr << line << "\n";
        } else {
            std::cout << line << "\n";
        }
    }

    if (fileOutputEnabled) {
        if (!fileStream.is_open() && !logFilePath.empty()) {
            fileStream.open(logFilePath, std::ios::out | std::ios::app);
        }
        if (fileStream.is_open()) {
            fileStream << line << "\n";
            fileStream.flush();
        }
    }
}

void Logger::DumpRingBuffer() {
    std::lock_guard<std::mutex> lock(mutex);

    if (fileOutputEnabled && !fileStream.is_open() && !logFilePath.empty()) {
        fileStream.open(logFilePath, std::ios::out | std::ios::app);
    }

    std::string header = "=== RECENT CALL RING-BUFFER DUMP START ===";
    std::string footer = "=== RECENT CALL RING-BUFFER DUMP END ===";

    if (consoleOutputEnabled) {
        std::cerr << header << "\n";
    }
    if (fileOutputEnabled && fileStream.is_open()) {
        fileStream << header << "\n";
    }

    for (std::size_t i = 0; i < ringBufferSize; ++i) {
        const std::size_t index = (ringBufferHead + i) % RingBufferCapacity;
        const std::string line = FormatLogLine(ringBuffer[index]);

        if (consoleOutputEnabled) {
            std::cerr << line << "\n";
        }
        if (fileOutputEnabled && fileStream.is_open()) {
            fileStream << line << "\n";
        }
    }

    if (consoleOutputEnabled) {
        std::cerr << footer << "\n";
        std::cerr.flush();
    }
    if (fileOutputEnabled && fileStream.is_open()) {
        fileStream << footer << "\n";
        fileStream.flush();
    }
}

void Logger::Flush() {
    std::lock_guard<std::mutex> lock(mutex);
    if (consoleOutputEnabled) {
        std::cout.flush();
        std::cerr.flush();
    }
    if (fileOutputEnabled && fileStream.is_open()) {
        fileStream.flush();
    }
}

std::vector<LogEntry> Logger::GetRingBufferEntries() {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<LogEntry> entries;
    entries.reserve(ringBufferSize);

    for (std::size_t i = 0; i < ringBufferSize; ++i) {
        const std::size_t index = (ringBufferHead + i) % RingBufferCapacity;
        entries.push_back(ringBuffer[index]);
    }

    return entries;
}

void Logger::ClearRingBuffer() {
    std::lock_guard<std::mutex> lock(mutex);
    ringBufferHead = 0;
    ringBufferSize = 0;
}

} // namespace Diagnostics
