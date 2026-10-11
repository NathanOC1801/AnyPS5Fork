#ifndef DIAGNOSTICS_LOG_CHANNEL_HPP
#define DIAGNOSTICS_LOG_CHANNEL_HPP

#include <cstdint>
#include <string_view>

namespace Diagnostics {

enum class LogChannel : std::uint8_t {
    Kernel,
    Prx,
    Loader,
    GpuSpirv,
    Memory,
    General
};

constexpr std::string_view LogChannelToString(LogChannel channel) {
    switch (channel) {
    case LogChannel::Kernel:
        return "[KERNEL]";
    case LogChannel::Prx:
        return "[PRX]";
    case LogChannel::Loader:
        return "[LOADER]";
    case LogChannel::GpuSpirv:
        return "[GPU/SPIRV]";
    case LogChannel::Memory:
        return "[MEMORY]";
    case LogChannel::General:
        return "[GENERAL]";
    default:
        return "[UNKNOWN]";
    }
}

} // namespace Diagnostics

#endif // DIAGNOSTICS_LOG_CHANNEL_HPP
