#include "drone/application_support/process_configuration.h"
#include "drone/application_support/shutdown_monitor.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/interceptor_state_machine.h"
#include "drone/interceptor_dds_adapter/assignment_subscriber.h"
#include "drone/interceptor_dds_adapter/drone_state_publisher.h"
#include "drone/interceptor_dds_adapter/explosion_event_publisher.h"
#include "drone/interceptor_dds_adapter/interception_command_subscriber.h"
#include "drone/interceptor_dds_adapter/target_track_subscriber.h"
#include "drone/simulated_vehicle_adapter/simulated_vehicle.h"

#include <chrono>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <locale>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{

using namespace std::chrono_literals;

constexpr auto defaultControlInterval{100ms};

struct ParticipantNames final
{
    std::string stateWriter;
    std::string explosionWriter;
    std::string assignmentReader;
    std::string commandReader;
    std::string targetReader;
};

[[nodiscard]] drone::application::ProcessConfiguration
parseConfiguration(const std::span<const char *const> arguments)
{
    return drone::application::parseProcessConfiguration(
        arguments, {.executableName = "interceptor",
                    .defaultParticipantName = "drone_interceptor_1",
                    .defaultTickInterval = defaultControlInterval});
}

[[nodiscard]] ParticipantNames participantNames(const std::string_view configuredName)
{
    return ParticipantNames{
        .stateWriter = drone::application::composeParticipantName(configuredName, {}),
        .explosionWriter =
            drone::application::composeParticipantName(configuredName, "explosion-writer"),
        .assignmentReader =
            drone::application::composeParticipantName(configuredName, "assignment-reader"),
        .commandReader =
            drone::application::composeParticipantName(configuredName, "command-reader"),
        .targetReader =
            drone::application::composeParticipantName(configuredName, "target-reader")};
}

void reportMovement(const drone::interceptor::InterceptorStateMachine &stateMachine)
{
    const auto &stateAfterMovement = stateMachine.state();
    const auto &movementTarget = stateMachine.latestTargetTrack();
    if (!stateAfterMovement.has_value() || !movementTarget.has_value())
    {
        throw std::logic_error{"A completed pursuit tick has no state or target"};
    }
    const auto &position = stateAfterMovement->position();
    const auto &targetPosition = movementTarget->position();
    std::cout.imbue(std::locale::classic());
    std::cout << std::fixed << std::setprecision(6) << "interceptor: pursuit position "
              << position.xMeters() << ' ' << position.yMeters() << ' ' << position.zMeters()
              << " target " << targetPosition.xMeters() << ' ' << targetPosition.yMeters() << ' '
              << targetPosition.zMeters() << " target-time "
              << movementTarget->measuredAt().timeSinceUnixEpoch().count() << '\n'
              << std::flush;
}

void run(const drone::application::ProcessConfiguration &configuration)
{
    const auto names = participantNames(configuration.participantName);
    drone::interceptor_dds::DroneStatePublisher statePublisher{configuration.domainId,
                                                               names.stateWriter};
    drone::interceptor_dds::ExplosionEventPublisher eventPublisher{configuration.domainId,
                                                                   names.explosionWriter};
    drone::simulated_vehicle::SimulatedVehicle vehicle{
        {.initialPosition = drone::domain::Position{0.0, 0.0, 0.0},
         .startsAt = drone::domain::Timestamp{0ms},
         .maximumSpeedMetersPerSecond = 50.0}};
    drone::interceptor::InterceptorStateMachine stateMachine{
        {.droneId = drone::domain::DroneId{1}, .arrivalToleranceMeters = 0.25},
        vehicle,
        vehicle,
        vehicle,
        statePublisher,
        eventPublisher};
    drone::interceptor_dds::AssignmentSubscriber assignmentSubscriber{
        configuration.domainId, names.assignmentReader, stateMachine};
    drone::interceptor_dds::InterceptionCommandSubscriber commandSubscriber{
        configuration.domainId, names.commandReader, stateMachine};
    drone::interceptor_dds::TargetTrackSubscriber targetSubscriber{
        configuration.domainId, names.targetReader, stateMachine};

    stateMachine.start();
    std::cout << "interceptor: published available state for drone 1; participant '"
              << configuration.participantName << "' in DDS domain " << configuration.domainId
              << "; control interval " << configuration.tickInterval.count() << " ms\n"
              << std::flush;

    while (!drone::application::ShutdownMonitor::requested())
    {
        const auto &stateAtTickStart = stateMachine.state();
        if (!stateAtTickStart.has_value())
        {
            throw std::logic_error{"The started interceptor has no current state"};
        }
        if (stateAtTickStart->status() != drone::domain::DroneStatus::intercepting)
        {
            vehicle.advanceTime(configuration.tickInterval);
        }

        const auto assignment = assignmentSubscriber.receiveNext(0ms);
        const auto &stateAfterAssignment = stateMachine.state();
        if (assignment.has_value() && stateAfterAssignment.has_value() &&
            stateAfterAssignment->status() == drone::domain::DroneStatus::assigned)
        {
            std::cout << "interceptor: accepted assignment to target 1\n" << std::flush;
        }

        const auto target = targetSubscriber.receiveNext(0ms);
        const auto &latestTarget = stateMachine.latestTargetTrack();
        if (target.has_value() &&
            (*target == drone::interceptor::TargetTrackHandlingResult::accepted ||
             *target == drone::interceptor::TargetTrackHandlingResult::updated) &&
            latestTarget.has_value())
        {
            std::cout << "interceptor: accepted target update "
                      << latestTarget->measuredAt().timeSinceUnixEpoch().count() << '\n'
                      << std::flush;
        }

        const auto command = commandSubscriber.receiveNext(0ms);
        const auto &stateAfterCommand = stateMachine.state();
        if (command == drone::interceptor_dds::InterceptionCommandDelivery::delivered &&
            stateAfterCommand.has_value() &&
            stateAfterCommand->status() == drone::domain::DroneStatus::intercepting)
        {
            std::cout << "interceptor: accepted interception start\n" << std::flush;
        }

        if (stateMachine.tick(configuration.tickInterval) ==
            drone::interceptor::InterceptionTickResult::moved)
        {
            reportMovement(stateMachine);
        }

        static_cast<void>(drone::application::ShutdownMonitor::waitFor(configuration.tickInterval));
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
        std::cout << "interceptor: shutdown complete\n" << std::flush;
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "interceptor: " << error.what() << '\n';
        return 1;
    }
}
