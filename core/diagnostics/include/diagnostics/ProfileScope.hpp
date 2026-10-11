#ifndef DIAGNOSTICS_PROFILE_SCOPE_HPP
#define DIAGNOSTICS_PROFILE_SCOPE_HPP

#include <diagnostics/Logger.hpp>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace Diagnostics {

class ProfileScope {
public:
    ProfileScope(std::string name, LogChannel channel = LogChannel::General)
        : name(std::move(name)), channel(channel), startTime(std::chrono::steady_clock::now()) {}

    ~ProfileScope() {
        const auto endTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double, std::milli> elapsedMs = endTime - startTime;

        std::ostringstream ss;
        ss << "[PROFILE] " << name << " completed in " << std::fixed << std::setprecision(3) << elapsedMs.count() << " ms";

        Logger::GetInstance().Log(channel, LogLevel::Debug, ss.str());
    }

    ProfileScope(const ProfileScope&) = delete;
    ProfileScope& operator=(const ProfileScope&) = delete;

private:
    std::string name;
    LogChannel channel;
    std::chrono::steady_clock::time_point startTime;
};

} // namespace Diagnostics

#define PROFILE_SCOPE_CHANNEL(channel, name) ::Diagnostics::ProfileScope profileScope##__LINE__(name, channel)
#define PROFILE_SCOPE(name) PROFILE_SCOPE_CHANNEL(::Diagnostics::LogChannel::General, name)

#endif // DIAGNOSTICS_PROFILE_SCOPE_HPP
