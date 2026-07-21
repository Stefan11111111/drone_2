#include "drone/application_support/process_configuration.h"
#include "drone/application_support/shutdown_monitor.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"
#include "drone/observer_core/target_tracker.h"
#include "drone/observer_dds_adapter/target_track_publisher.h"
#include "drone/simulated_radar_adapter/simulated_radar.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <span>

namespace
{

using namespace std::chrono_literals;

constexpr auto defaultRadarTickInterval{250ms};

[[nodiscard]] drone::application::ProcessConfiguration
parseConfiguration(const std::span<const char *const> arguments)
{
    return drone::application::parseProcessConfiguration(
        arguments, {.executableName = "observer",
                    .defaultParticipantName = "drone_observer",
                    .defaultTickInterval = defaultRadarTickInterval,
                    .acceptsTickCount = true});
}

[[nodiscard]] drone::simulated_radar::Scenario
observerScenario(const std::chrono::milliseconds tickInterval)
{
    return drone::simulated_radar::Scenario{
        .targetId = drone::domain::TargetId{1},
        .initialPosition = drone::domain::Position{0.0, 0.0, 1'000.0},
        .startsAt = drone::domain::Timestamp{0ms},
        .velocity = drone::simulated_radar::Velocity{.xMetersPerSecond = 20.0,
                                                     .yMetersPerSecond = 5.0,
                                                     .zMetersPerSecond = 0.0},
        .tickInterval = tickInterval};
}

void run(const drone::application::ProcessConfiguration &configuration)
{
    drone::observer_dds::TargetTrackPublisher publisher{configuration.domainId,
                                                        configuration.participantName};
    drone::observer::TargetTracker tracker{publisher};
    drone::simulated_radar::SimulatedRadar radar{tracker,
                                                 observerScenario(configuration.tickInterval)};

    std::cout << "observer: publishing target 1 in DDS domain " << configuration.domainId
              << "; participant '" << configuration.participantName << "'; tick interval "
              << configuration.tickInterval.count() << " ms\n"
              << std::flush;

    std::uint64_t publishedTicks{};
    while (!drone::application::ShutdownMonitor::requested() &&
           (!configuration.tickCount || publishedTicks < *configuration.tickCount))
    {
        radar.tick();
        ++publishedTicks;
        std::cout << "observer: published target update " << publishedTicks << '\n' << std::flush;

        if ((!configuration.tickCount || publishedTicks < *configuration.tickCount) &&
            drone::application::ShutdownMonitor::waitFor(configuration.tickInterval))
        {
            break;
        }
    }
}

} // namespace

int main(const int argumentCount, const char *const arguments[])
{
    try
    {
        const auto configuration = parseConfiguration(
            std::span<const char *const>{arguments, static_cast<std::size_t>(argumentCount)});
        const drone::application::ShutdownMonitor shutdownMonitor;
        run(configuration);
        std::cout << "observer: shutdown complete\n" << std::flush;
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "observer: " << error.what() << '\n';
        return 1;
    }
}
