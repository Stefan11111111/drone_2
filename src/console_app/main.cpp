#include "drone/console_core/assignment_use_case.h"
#include "drone/console_core/drone_projection.h"
#include "drone/console_core/interception_command_use_case.h"
#include "drone/console_core/outcome_projection.h"
#include "drone/console_core/target_projection.h"
#include "drone/console_dds_adapter/assignment_publisher.h"
#include "drone/console_dds_adapter/console_subscriber.h"
#include "drone/console_dds_adapter/explosion_event_subscriber.h"
#include "drone/console_dds_adapter/interception_command_publisher.h"
#include "drone/console_ui_adapter/terminal_view.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/target_id.h"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace
{

using namespace std::chrono_literals;

constexpr std::uint32_t defaultDomainId{0};
constexpr std::uint64_t maximumDefaultPortDomainId{232};
constexpr auto receiveTimeout{50ms};
constexpr std::string_view automatedPursuitArgument{"--auto-pursuit"};

struct Configuration final
{
    std::uint32_t domainId{defaultDomainId};
    bool automatePursuit{false};
};

struct AutomatedPursuitState final
{
    bool assignmentSent{false};
    bool startSent{false};
};

struct ConsoleModel final
{
    drone::console::TargetProjection targets;
    drone::console::DroneProjection drones;
    drone::console::OutcomeProjection outcomes{targets, drones};
};

[[nodiscard]] Configuration parseConfiguration(const std::span<const char *const> arguments)
{
    if (arguments.size() > 3)
    {
        throw std::invalid_argument{"usage: console [domain-id [--auto-pursuit]]"};
    }

    Configuration configuration;
    if (arguments.size() == 1)
    {
        return configuration;
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

    configuration.domainId = static_cast<std::uint32_t>(domainId);
    if (arguments.size() == 3)
    {
        if (arguments[2] != automatedPursuitArgument)
        {
            throw std::invalid_argument{"the only supported console action mode is --auto-pursuit"};
        }
        configuration.automatePursuit = true;
    }
    return configuration;
}

[[nodiscard]] std::string_view
receiveIssueDescription(const drone::console_dds::ReceiveIssue issue) noexcept
{
    using enum drone::console_dds::ReceiveIssue;
    switch (issue)
    {
    case timedOut:
        return "receive timed out";
    case discardedInvalidData:
        return "discarded an instance-state notification";
    case discardedMalformedData:
        return "discarded a malformed sample";
    }
    return "unknown receive issue";
}

void reportTargetResult(const drone::console_dds::TargetTrackReceiveResult &received)
{
    if (!received.has_value() && received.error() != drone::console_dds::ReceiveIssue::timedOut)
    {
        std::cerr << "console: TargetTrack " << receiveIssueDescription(received.error()) << '\n';
    }
}

void reportDroneResult(const drone::console_dds::DroneStateReceiveResult &received)
{
    if (!received.has_value() && received.error() != drone::console_dds::ReceiveIssue::timedOut)
    {
        std::cerr << "console: DroneState " << receiveIssueDescription(received.error()) << '\n';
    }
}

void reportOutcomeResult(const drone::console_dds::ExplosionEventReceiveResult &received)
{
    if (!received.has_value() &&
        received.error() != drone::console_dds::ExplosionEventReceiveIssue::timedOut)
    {
        std::cerr << "console: ExplosionEvent receive failed\n";
    }
}

void receiveTarget(drone::console_dds::ConsoleSubscriber &subscriber,
                   const drone::console_ui::TerminalView &view, ConsoleModel &model)
{
    const auto received = subscriber.receiveNextTarget(receiveTimeout);
    if (received.has_value())
    {
        view.render(model.targets, model.drones, model.outcomes);
        return;
    }
    reportTargetResult(received);
}

void receiveDrone(drone::console_dds::ConsoleSubscriber &subscriber,
                  const drone::console_ui::TerminalView &view, ConsoleModel &model)
{
    const auto received = subscriber.receiveNextDrone(receiveTimeout);
    if (received.has_value())
    {
        view.render(model.targets, model.drones, model.outcomes);
        return;
    }
    reportDroneResult(received);
}

void receiveOutcome(drone::console_dds::ExplosionEventSubscriber &subscriber,
                    const drone::console_ui::TerminalView &view, ConsoleModel &model)
{
    const auto received = subscriber.receiveNext(receiveTimeout);
    if (received.has_value())
    {
        view.render(model.targets, model.drones, model.outcomes);
        return;
    }
    reportOutcomeResult(received);
}

void automatePursuit(const bool assignmentReaderMatched, const bool commandReaderMatched,
                     drone::console::AssignmentUseCase &assignment,
                     drone::console::InterceptionCommandUseCase &command,
                     AutomatedPursuitState &automation)
{
    const drone::domain::DroneId droneId{1};
    const drone::domain::TargetId targetId{1};

    if (!automation.assignmentSent && assignmentReaderMatched &&
        assignment.assign(droneId, targetId) == drone::console::AssignmentResult::assigned)
    {
        automation.assignmentSent = true;
        std::cout << "console: automated assignment sent for drone 1 to target 1\n" << std::flush;
    }
    if (automation.assignmentSent && !automation.startSent && commandReaderMatched &&
        command.start(droneId) == drone::console::StartInterceptionResult::started)
    {
        automation.startSent = true;
        std::cout << "console: automated interception start sent for drone 1\n" << std::flush;
    }
}

void run(const Configuration &configuration)
{
    ConsoleModel model;
    drone::console_dds::ConsoleSubscriber subscriber{configuration.domainId, "drone_console",
                                                     model.targets, model.drones};
    drone::console_dds::ExplosionEventSubscriber outcomeSubscriber{
        configuration.domainId, "drone_console_explosion_reader", model.outcomes};
    drone::console_dds::AssignmentPublisher assignmentPublisher{configuration.domainId,
                                                                "drone_console_assignment_writer"};
    drone::console_dds::InterceptionCommandPublisher commandPublisher{
        configuration.domainId, "drone_console_command_writer"};
    drone::console::AssignmentUseCase assignment{model.targets, model.drones, assignmentPublisher};
    drone::console::InterceptionCommandUseCase command{model.drones, commandPublisher};
    const drone::console_ui::TerminalView view{std::cout};

    std::cout << "console: readers ready for targets and drones in DDS domain "
              << configuration.domainId << '\n';
    view.render(model.targets, model.drones, model.outcomes);

    bool targetWriterMatched{false};
    bool droneWriterMatched{false};
    bool eventWriterMatched{false};
    bool assignmentReaderMatched{false};
    bool commandReaderMatched{false};
    AutomatedPursuitState automation;
    while (true)
    {
        if (!targetWriterMatched && subscriber.waitForTargetWriterMatch(0ms))
        {
            targetWriterMatched = true;
            std::cout << "console: matched observer TargetTrack writer\n" << std::flush;
        }
        if (!droneWriterMatched && subscriber.waitForDroneWriterMatch(0ms))
        {
            droneWriterMatched = true;
            std::cout << "console: matched interceptor DroneState writer\n" << std::flush;
        }
        if (!eventWriterMatched && outcomeSubscriber.waitForWriterMatch(0ms))
        {
            eventWriterMatched = true;
            std::cout << "console: matched interceptor ExplosionEvent writer\n" << std::flush;
        }
        if (!assignmentReaderMatched && assignmentPublisher.waitForInterceptorMatch(0ms))
        {
            assignmentReaderMatched = true;
        }
        if (!commandReaderMatched && commandPublisher.waitForInterceptorMatch(0ms))
        {
            commandReaderMatched = true;
        }

        receiveTarget(subscriber, view, model);
        receiveDrone(subscriber, view, model);
        receiveOutcome(outcomeSubscriber, view, model);

        if (configuration.automatePursuit)
        {
            automatePursuit(assignmentReaderMatched, commandReaderMatched, assignment, command,
                            automation);
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
        std::cerr << "console: " << error.what() << '\n';
        return 1;
    }
}
