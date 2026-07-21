#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"
#include "drone/observer_core/target_tracker.h"
#include "drone/observer_dds_adapter/target_track_publisher.h"
#include "drone/simulated_radar_adapter/simulated_radar.h"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>

namespace
{

using namespace std::chrono_literals;

constexpr std::uint32_t defaultDomainId{0};
constexpr std::uint64_t maximumDefaultPortDomainId{232};
constexpr auto radarTickInterval{250ms};

struct Configuration final
{
    std::uint32_t domainId{defaultDomainId};
    std::optional<std::uint64_t> tickCount;
};

struct NamedArgument final
{
    std::string_view text;
    std::string_view name;
};

[[nodiscard]] std::uint64_t parseUnsigned(const NamedArgument argument)
{
    std::uint64_t value{};
    const auto [parsedEnd, error] =
        std::from_chars(argument.text.begin(), argument.text.end(), value);
    if (error != std::errc{} || parsedEnd != argument.text.end())
    {
        throw std::invalid_argument{std::string{argument.name} + " must be an unsigned integer"};
    }
    return value;
}

[[nodiscard]] Configuration parseConfiguration(const std::span<const char *const> arguments)
{
    if (arguments.size() > 3)
    {
        throw std::invalid_argument{"usage: observer [domain-id [tick-count]]"};
    }

    Configuration configuration;
    if (arguments.size() >= 2)
    {
        const auto domainId = parseUnsigned({.text = arguments[1], .name = "domain-id"});
        if (domainId > maximumDefaultPortDomainId)
        {
            throw std::invalid_argument{"domain-id must not exceed 232"};
        }
        configuration.domainId = static_cast<std::uint32_t>(domainId);
    }
    if (arguments.size() == 3)
    {
        const auto tickCount = parseUnsigned({.text = arguments[2], .name = "tick-count"});
        if (tickCount == 0)
        {
            throw std::invalid_argument{"tick-count must be positive"};
        }
        configuration.tickCount = tickCount;
    }
    return configuration;
}

[[nodiscard]] drone::simulated_radar::Scenario observerScenario()
{
    return drone::simulated_radar::Scenario{
        .targetId = drone::domain::TargetId{1},
        .initialPosition = drone::domain::Position{0.0, 0.0, 1'000.0},
        .startsAt = drone::domain::Timestamp{0ms},
        .velocity = drone::simulated_radar::Velocity{.xMetersPerSecond = 20.0,
                                                     .yMetersPerSecond = 5.0,
                                                     .zMetersPerSecond = 0.0},
        .tickInterval = radarTickInterval};
}

void run(const Configuration &configuration)
{
    drone::observer_dds::TargetTrackPublisher publisher{configuration.domainId, "drone_observer"};
    drone::observer::TargetTracker tracker{publisher};
    drone::simulated_radar::SimulatedRadar radar{tracker, observerScenario()};

    std::cout << "observer: publishing target 1 in DDS domain " << configuration.domainId << '\n';

    std::uint64_t publishedTicks{};
    while (!configuration.tickCount || publishedTicks < *configuration.tickCount)
    {
        radar.tick();
        ++publishedTicks;
        std::cout << "observer: published target update " << publishedTicks << '\n';

        if (!configuration.tickCount || publishedTicks < *configuration.tickCount)
        {
            std::this_thread::sleep_for(radarTickInterval);
        }
    }
}

} // namespace

int main(const int argumentCount, const char *const arguments[])
{
    try
    {
        run(parseConfiguration(
            std::span<const char *const>{arguments, static_cast<std::size_t>(argumentCount)}));
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "observer: " << error.what() << '\n';
        return 1;
    }
}
