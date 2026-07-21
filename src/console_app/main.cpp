#include "drone/console_core/target_projection.h"
#include "drone/console_dds_adapter/target_track_subscriber.h"
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
constexpr auto receiveTimeout{250ms};

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
receiveIssueDescription(const drone::console_dds::TargetTrackReceiveIssue issue) noexcept
{
    using enum drone::console_dds::TargetTrackReceiveIssue;
    switch (issue)
    {
    case timedOut:
        return "receive timed out";
    case discardedInvalidData:
        return "discarded a TargetTrack instance-state notification";
    case discardedMalformedData:
        return "discarded a malformed TargetTrack sample";
    }
    return "unknown TargetTrack receive issue";
}

void run(const std::uint32_t domainId)
{
    drone::console::TargetProjection projection;
    drone::console_dds::TargetTrackSubscriber subscriber{domainId, "drone_console", projection};
    const drone::console_ui::TerminalView view{std::cout};

    std::cout << "console: receiving target tracks in DDS domain " << domainId << '\n';
    view.render(projection);

    while (!subscriber.waitForWriterMatch(receiveTimeout))
    {
    }
    std::cout << "console: matched observer TargetTrack writer\n" << std::flush;

    while (true)
    {
        const auto received = subscriber.receiveNext(receiveTimeout);
        if (received.has_value())
        {
            view.render(projection);
        }
        else if (received.error() != drone::console_dds::TargetTrackReceiveIssue::timedOut)
        {
            std::cerr << "console: " << receiveIssueDescription(received.error()) << '\n';
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
