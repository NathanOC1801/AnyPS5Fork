#include <diagnostics/LogChannel.hpp>
#include <diagnostics/LogLevel.hpp>
#include <diagnostics/Logger.hpp>
#include <diagnostics/ProfileScope.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void TestChannelAndLevelFormatting() {
    assert(Diagnostics::LogChannelToString(Diagnostics::LogChannel::Kernel) == "[KERNEL]");
    assert(Diagnostics::LogChannelToString(Diagnostics::LogChannel::Prx) == "[PRX]");
    assert(Diagnostics::LogChannelToString(Diagnostics::LogChannel::Loader) == "[LOADER]");
    assert(Diagnostics::LogChannelToString(Diagnostics::LogChannel::GpuSpirv) == "[GPU/SPIRV]");
    assert(Diagnostics::LogChannelToString(Diagnostics::LogChannel::Memory) == "[MEMORY]");
    assert(Diagnostics::LogChannelToString(Diagnostics::LogChannel::General) == "[GENERAL]");

    assert(Diagnostics::LogLevelToString(Diagnostics::LogLevel::Trace) == "TRACE");
    assert(Diagnostics::LogLevelToString(Diagnostics::LogLevel::Debug) == "DEBUG");
    assert(Diagnostics::LogLevelToString(Diagnostics::LogLevel::Info) == "INFO");
    assert(Diagnostics::LogLevelToString(Diagnostics::LogLevel::Warn) == "WARN");
    assert(Diagnostics::LogLevelToString(Diagnostics::LogLevel::Error) == "ERROR");
}

void TestLoggerInitializationAndFileCreation() {
    const std::string testLogPath = "test_diagnostics.log";
    auto& logger = Diagnostics::Logger::GetInstance();

    logger.Initialize(testLogPath, false, true);
    logger.ClearRingBuffer();

    LOG_INFO(Diagnostics::LogChannel::Kernel, "Kernel initialization message");
    LOG_WARN(Diagnostics::LogChannel::Memory, "Memory allocation warning");

    logger.Flush();

    std::ifstream file(testLogPath);
    assert(file.is_open());

    std::string line1;
    std::string line2;
    std::getline(file, line1);
    std::getline(file, line2);
    file.close();

    assert(line1.find("[KERNEL]") != std::string::npos);
    assert(line1.find("[INFO]") != std::string::npos);
    assert(line1.find("Kernel initialization message") != std::string::npos);

    assert(line2.find("[MEMORY]") != std::string::npos);
    assert(line2.find("[WARN]") != std::string::npos);
    assert(line2.find("Memory allocation warning") != std::string::npos);
}

void TestRingBufferAndCrashDump() {
    auto& logger = Diagnostics::Logger::GetInstance();
    logger.SetLogLevel(Diagnostics::LogLevel::Trace);
    logger.ClearRingBuffer();

    for (int i = 0; i < 10; ++i) {
        LOG_TRACE(Diagnostics::LogChannel::Loader, "Loader step " + std::to_string(i));
    }

    auto entries = logger.GetRingBufferEntries();
    assert(entries.size() == 10);
    assert(entries[0].message == "Loader step 0");
    assert(entries[9].message == "Loader step 9");

    logger.DumpRingBuffer();
}

void TestMultithreadedLogging() {
    auto& logger = Diagnostics::Logger::GetInstance();
    logger.SetLogLevel(Diagnostics::LogLevel::Trace);
    logger.ClearRingBuffer();

    constexpr int ThreadCount = 8;
    constexpr int LogsPerThread = 50;

    std::vector<std::thread> threads;
    threads.reserve(ThreadCount);

    for (int t = 0; t < ThreadCount; ++t) {
        threads.emplace_back([t]() {
            for (int i = 0; i < LogsPerThread; ++i) {
                LOG_DEBUG(Diagnostics::LogChannel::GpuSpirv,
                          "Thread " + std::to_string(t) + " shader compilation item " + std::to_string(i));
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    auto entries = logger.GetRingBufferEntries();
    assert(entries.size() == ThreadCount * LogsPerThread);
}

void TestProfileScope() {
    auto& logger = Diagnostics::Logger::GetInstance();
    logger.SetLogLevel(Diagnostics::LogLevel::Trace);
    logger.ClearRingBuffer();

    {
        PROFILE_SCOPE_CHANNEL(Diagnostics::LogChannel::GpuSpirv, "ShaderRecompilationTest");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    auto entries = logger.GetRingBufferEntries();
    assert(!entries.empty());
    assert(entries.back().message.find("[PROFILE] ShaderRecompilationTest completed in") != std::string::npos);
    assert(entries.back().channel == Diagnostics::LogChannel::GpuSpirv);
}

void TestCrashAndThrowMacro() {
    auto& logger = Diagnostics::Logger::GetInstance();
    logger.ClearRingBuffer();

    bool caught = false;
    try {
        LOG_CRASH_AND_THROW(Diagnostics::LogChannel::Prx, "Fatal PRX load error");
    } catch (const std::runtime_error& e) {
        caught = true;
        assert(std::string(e.what()) == "Fatal PRX load error");
    }

    assert(caught);
}

int main() {
    TestChannelAndLevelFormatting();
    TestLoggerInitializationAndFileCreation();
    TestRingBufferAndCrashDump();
    TestMultithreadedLogging();
    TestProfileScope();
    TestCrashAndThrowMacro();

    std::cout << "All diagnostics tests passed successfully.\n";
    return 0;
}
