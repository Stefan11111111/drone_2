#include "drone/application_support/process_configuration.h"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace drone::application
{
namespace
{

constexpr std::uint64_t maximumDefaultPortDomainId{232};
constexpr std::size_t maximumParticipantNameLength{255};

struct SeenOptions final
{
    bool domainId{false};
    bool participantName{false};
    bool tickInterval{false};
    bool tickCount{false};
    bool automatedPursuit{false};
};

struct NamedOptionValue final
{
    std::string_view text;
    std::string_view option;
};

[[nodiscard]] std::string usage(const ProcessConfigurationSpecification &specification)
{
    std::string text{"usage: "};
    text += specification.executableName;
    text += " [--domain-id ID] [--participant-name NAME] [--tick-ms MILLISECONDS]";
    if (specification.acceptsTickCount)
    {
        text += " [--tick-count COUNT]";
    }
    if (specification.acceptsAutomatedPursuit)
    {
        text += " [--auto-pursuit]";
    }
    return text;
}

[[noreturn]] void throwConfigurationError(const std::string_view message,
                                          const ProcessConfigurationSpecification &specification)
{
    throw std::invalid_argument{std::string{message} + "\n" + usage(specification)};
}

[[nodiscard]] std::uint64_t parseUnsigned(const NamedOptionValue argument,
                                          const ProcessConfigurationSpecification &specification)
{
    std::uint64_t value{};
    const auto [parsedEnd, error] =
        std::from_chars(argument.text.begin(), argument.text.end(), value);
    if (error != std::errc{} || parsedEnd != argument.text.end())
    {
        throwConfigurationError(std::string{argument.option} + " must be an unsigned integer",
                                specification);
    }
    return value;
}

[[nodiscard]] std::string_view optionValue(const std::span<const char *const> arguments,
                                           std::size_t &index, const std::string_view option,
                                           const ProcessConfigurationSpecification &specification)
{
    ++index;
    if (index >= arguments.size())
    {
        throwConfigurationError(std::string{option} + " requires a value", specification);
    }
    return arguments[index];
}

void rejectDuplicate(const bool alreadySeen, const std::string_view option,
                     const ProcessConfigurationSpecification &specification)
{
    if (alreadySeen)
    {
        throwConfigurationError(std::string{option} + " may be specified only once", specification);
    }
}

void validateParticipantName(const std::string_view name,
                             const ProcessConfigurationSpecification &specification)
{
    if (name.empty())
    {
        throwConfigurationError("--participant-name must not be empty", specification);
    }
    if (name.size() > maximumParticipantNameLength)
    {
        throwConfigurationError("--participant-name must not exceed 255 characters", specification);
    }
}

void parseDomainId(const std::span<const char *const> arguments, std::size_t &index,
                   ProcessConfiguration &configuration, SeenOptions &seen,
                   const ProcessConfigurationSpecification &specification)
{
    rejectDuplicate(seen.domainId, "--domain-id", specification);
    const auto value =
        parseUnsigned({.text = optionValue(arguments, index, "--domain-id", specification),
                       .option = "--domain-id"},
                      specification);
    if (value > maximumDefaultPortDomainId)
    {
        throwConfigurationError("--domain-id must not exceed 232", specification);
    }
    configuration.domainId = static_cast<std::uint32_t>(value);
    seen.domainId = true;
}

void parseParticipantName(const std::span<const char *const> arguments, std::size_t &index,
                          ProcessConfiguration &configuration, SeenOptions &seen,
                          const ProcessConfigurationSpecification &specification)
{
    rejectDuplicate(seen.participantName, "--participant-name", specification);
    const auto value = optionValue(arguments, index, "--participant-name", specification);
    validateParticipantName(value, specification);
    configuration.participantName = value;
    seen.participantName = true;
}

void parseTickInterval(const std::span<const char *const> arguments, std::size_t &index,
                       ProcessConfiguration &configuration, SeenOptions &seen,
                       const ProcessConfigurationSpecification &specification)
{
    rejectDuplicate(seen.tickInterval, "--tick-ms", specification);
    const auto value = parseUnsigned(
        {.text = optionValue(arguments, index, "--tick-ms", specification), .option = "--tick-ms"},
        specification);
    if (value == 0)
    {
        throwConfigurationError("--tick-ms must be positive", specification);
    }
    using TickRepresentation = std::chrono::milliseconds::rep;
    if (value > static_cast<std::uint64_t>(std::numeric_limits<TickRepresentation>::max()))
    {
        throwConfigurationError("--tick-ms is too large", specification);
    }
    configuration.tickInterval = std::chrono::milliseconds{static_cast<TickRepresentation>(value)};
    seen.tickInterval = true;
}

void parseTickCount(const std::span<const char *const> arguments, std::size_t &index,
                    ProcessConfiguration &configuration, SeenOptions &seen,
                    const ProcessConfigurationSpecification &specification)
{
    if (!specification.acceptsTickCount)
    {
        throwConfigurationError("unknown option '--tick-count'", specification);
    }
    rejectDuplicate(seen.tickCount, "--tick-count", specification);
    const auto value =
        parseUnsigned({.text = optionValue(arguments, index, "--tick-count", specification),
                       .option = "--tick-count"},
                      specification);
    if (value == 0)
    {
        throwConfigurationError("--tick-count must be positive", specification);
    }
    configuration.tickCount = value;
    seen.tickCount = true;
}

void parseAutomatedPursuit(ProcessConfiguration &configuration, SeenOptions &seen,
                           const ProcessConfigurationSpecification &specification)
{
    if (!specification.acceptsAutomatedPursuit)
    {
        throwConfigurationError("unknown option '--auto-pursuit'", specification);
    }
    rejectDuplicate(seen.automatedPursuit, "--auto-pursuit", specification);
    configuration.automatePursuit = true;
    seen.automatedPursuit = true;
}

} // namespace

ProcessConfiguration
parseProcessConfiguration(const std::span<const char *const> arguments,
                          const ProcessConfigurationSpecification &specification)
{
    if (specification.executableName.empty())
    {
        throw std::logic_error{"A process configuration specification needs an executable name"};
    }
    if (specification.defaultTickInterval <= std::chrono::milliseconds::zero())
    {
        throw std::logic_error{"A process configuration specification needs a positive tick"};
    }
    validateParticipantName(specification.defaultParticipantName, specification);

    ProcessConfiguration configuration{.domainId = 0,
                                       .participantName =
                                           std::string{specification.defaultParticipantName},
                                       .tickInterval = specification.defaultTickInterval};
    SeenOptions seen;
    for (std::size_t index = 1; index < arguments.size(); ++index)
    {
        const std::string_view option{arguments[index]};
        if (option == "--domain-id")
        {
            parseDomainId(arguments, index, configuration, seen, specification);
        }
        else if (option == "--participant-name")
        {
            parseParticipantName(arguments, index, configuration, seen, specification);
        }
        else if (option == "--tick-ms")
        {
            parseTickInterval(arguments, index, configuration, seen, specification);
        }
        else if (option == "--tick-count")
        {
            parseTickCount(arguments, index, configuration, seen, specification);
        }
        else if (option == "--auto-pursuit")
        {
            parseAutomatedPursuit(configuration, seen, specification);
        }
        else
        {
            throwConfigurationError("unknown option '" + std::string{option} + "'", specification);
        }
    }
    return configuration;
}

std::string composeParticipantName(const std::string_view configuredName,
                                   const std::string_view endpointSuffix)
{
    std::string name{configuredName};
    if (!endpointSuffix.empty())
    {
        name += '.';
        name += endpointSuffix;
    }
    if (name.size() > maximumParticipantNameLength)
    {
        throw std::invalid_argument{"--participant-name '" + std::string{configuredName} +
                                    "' is too long for DDS endpoint suffix '" +
                                    std::string{endpointSuffix} + "'"};
    }
    return name;
}

} // namespace drone::application
