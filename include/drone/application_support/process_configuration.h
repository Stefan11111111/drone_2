#ifndef DRONE_APPLICATION_SUPPORT_PROCESS_CONFIGURATION_H
#define DRONE_APPLICATION_SUPPORT_PROCESS_CONFIGURATION_H

#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace drone::application
{

struct ProcessConfiguration final
{
    std::uint32_t domainId{};
    std::string participantName;
    std::chrono::milliseconds tickInterval{};
    std::optional<std::uint64_t> tickCount;
    bool automatePursuit{false};
};

struct ProcessConfigurationSpecification final
{
    std::string_view executableName;
    std::string_view defaultParticipantName;
    std::chrono::milliseconds defaultTickInterval;
    bool acceptsTickCount{false};
    bool acceptsAutomatedPursuit{false};
};

[[nodiscard]] ProcessConfiguration
parseProcessConfiguration(std::span<const char *const> arguments,
                          const ProcessConfigurationSpecification &specification);

[[nodiscard]] std::string composeParticipantName(std::string_view configuredName,
                                                 std::string_view endpointSuffix);

} // namespace drone::application

#endif // DRONE_APPLICATION_SUPPORT_PROCESS_CONFIGURATION_H
