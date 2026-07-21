#include "drone/console_core/drone_projection.h"
#include "drone/console_core/target_projection.h"
#include "drone/console_dds_adapter/console_subscriber.h"
#include "drone/console_ui_adapter/terminal_view.h"

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

[[nodiscard]] std::uint32_t parseDomainId(const std::span<const char *const> arguments)
{
    if (arguments.size() > 2)
    {
        throw std::invalid_argument{"usage: console [domain-id]"};
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

void run(const std::uint32_t domainId)
{
    drone::console::TargetProjection projection;
    drone::console::DroneProjection drones;
    drone::console_dds::ConsoleSubscriber subscriber{domainId, "drone_console", projection, drones};
    const drone::console_ui::TerminalView view{std::cout};

    std::cout << "console: readers ready for targets and drones in DDS domain " << domainId << '\n';
    view.render(projection, drones);

    bool targetWriterMatched{false};
    bool droneWriterMatched{false};
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

        const auto target = subscriber.receiveNextTarget(receiveTimeout);
        if (target.has_value())
        {
            view.render(projection, drones);
        }
        else
        {
            reportTargetResult(target);
        }

        const auto drone = subscriber.receiveNextDrone(receiveTimeout);
        if (drone.has_value())
        {
            view.render(projection, drones);
        }
        else
        {
            reportDroneResult(drone);
        }
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
        std::cerr << "console: " << error.what() << '\n';
        return 1;
    }
}
