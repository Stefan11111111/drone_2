#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/interceptor_state_machine.h"
#include "drone/interceptor_dds_adapter/assignment_subscriber.h"
#include "drone/interceptor_dds_adapter/drone_state_publisher.h"
#include "drone/interceptor_dds_adapter/interception_command_subscriber.h"
#include "drone/interceptor_dds_adapter/target_track_subscriber.h"
#include "drone/simulated_vehicle_adapter/simulated_vehicle.h"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <locale>
#include <span>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <thread>

namespace
{

using namespace std::chrono_literals;

constexpr std::uint32_t defaultDomainId{0};
constexpr std::uint64_t maximumDefaultPortDomainId{232};
constexpr auto controlInterval{100ms};

[[nodiscard]] std::uint32_t parseDomainId(const std::span<const char *const> arguments)
{
    if (arguments.size() > 2)
    {
        throw std::invalid_argument{"usage: interceptor [domain-id]"};
    }
    if (arguments.size() == 1)
    {
        return defaultDomainId;
    }

    std::uint64_t domainId{};
    const std::string_view text{arguments[1]};
    const auto [parsedEnd, error] = std::from_chars(text.begin(), text.end(), domainId);
    if (error != std::errc{} || parsedEnd != text.end())
    {
        throw std::invalid_argument{"domain-id must be an unsigned integer"};
    }
    if (domainId > maximumDefaultPortDomainId)
    {
        throw std::invalid_argument{"domain-id must not exceed 232"};
    }
    return static_cast<std::uint32_t>(domainId);
}

void run(const std::uint32_t domainId)
{
    drone::interceptor_dds::DroneStatePublisher statePublisher{domainId, "drone_interceptor_1"};
    drone::simulated_vehicle::SimulatedVehicle vehicle{
        {.initialPosition = drone::domain::Position{0.0, 0.0, 0.0},
         .startsAt = drone::domain::Timestamp{0ms},
         .maximumSpeedMetersPerSecond = 50.0}};
    drone::interceptor::InterceptorStateMachine stateMachine{
        {.droneId = drone::domain::DroneId{1}, .arrivalToleranceMeters = 0.25},
        vehicle,
        vehicle,
        vehicle,
        statePublisher};
    drone::interceptor_dds::AssignmentSubscriber assignmentSubscriber{
        domainId, "drone_interceptor_assignment_reader", stateMachine};
    drone::interceptor_dds::InterceptionCommandSubscriber commandSubscriber{
        domainId, "drone_interceptor_command_reader", stateMachine};
    drone::interceptor_dds::TargetTrackSubscriber targetSubscriber{
        domainId, "drone_interceptor_target_reader", stateMachine};

    stateMachine.start();
    std::cout << "interceptor: published available state for drone 1 in DDS domain " << domainId
              << '\n'
              << std::flush;

    while (true)
    {
        const auto &stateAtTickStart = stateMachine.state();
        if (!stateAtTickStart.has_value())
        {
            throw std::logic_error{"The started interceptor has no current state"};
        }
        if (stateAtTickStart->status() != drone::domain::DroneStatus::intercepting)
        {
            vehicle.advanceTime(controlInterval);
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

        if (stateMachine.tick(controlInterval) == drone::interceptor::InterceptionTickResult::moved)
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
                      << position.xMeters() << ' ' << position.yMeters() << ' '
                      << position.zMeters() << " target " << targetPosition.xMeters() << ' '
                      << targetPosition.yMeters() << ' ' << targetPosition.zMeters()
                      << " target-time "
                      << movementTarget->measuredAt().timeSinceUnixEpoch().count() << '\n'
                      << std::flush;
        }

        std::this_thread::sleep_for(controlInterval);
    }
}

} // namespace

int main(const int argumentCount, const char *const arguments[])
{
    try
    {
        run(parseDomainId(
            std::span<const char *const>{arguments, static_cast<std::size_t>(argumentCount)}));
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "interceptor: " << error.what() << '\n';
        return 1;
    }
}
