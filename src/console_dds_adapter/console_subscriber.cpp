#include "drone/console_dds_adapter/console_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::console_dds
{

ConsoleSubscriber::ConsoleSubscriber(const std::uint32_t domainId,
                                     const std::string_view participantName,
                                     console::TargetTrackInputPort &targetInput,
                                     console::DroneStateInputPort &droneInput)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      targetReader_{participant_.participant()}, droneReader_{participant_.participant()},
      targetInput_{targetInput}, droneInput_{droneInput}
{
}

ConsoleSubscriber::~ConsoleSubscriber() noexcept = default;

bool ConsoleSubscriber::waitForTargetWriterMatch(const std::chrono::milliseconds timeout)
{
    return targetReader_.waitForWriterMatch(timeout);
}

bool ConsoleSubscriber::waitForDroneWriterMatch(const std::chrono::milliseconds timeout)
{
    return droneReader_.waitForWriterMatch(timeout);
}

TargetTrackReceiveResult
ConsoleSubscriber::receiveNextTarget(const std::chrono::milliseconds timeout)
{
    if (!targetReader_.waitForData(timeout))
    {
        return std::unexpected{ReceiveIssue::timedOut};
    }

    auto sample = targetReader_.takeNext();
    if (!sample.has_value())
    {
        return std::unexpected{ReceiveIssue::discardedMalformedData};
    }
    if (!sample->has_value())
    {
        return std::unexpected{ReceiveIssue::discardedInvalidData};
    }
    return targetInput_.onTargetTrack(**sample);
}

DroneStateReceiveResult ConsoleSubscriber::receiveNextDrone(const std::chrono::milliseconds timeout)
{
    if (!droneReader_.waitForData(timeout))
    {
        return std::unexpected{ReceiveIssue::timedOut};
    }

    auto sample = droneReader_.takeNext();
    if (!sample.has_value())
    {
        return std::unexpected{ReceiveIssue::discardedMalformedData};
    }
    if (!sample->has_value())
    {
        return std::unexpected{ReceiveIssue::discardedInvalidData};
    }
    return droneInput_.onDroneState(**sample);
}

} // namespace drone::console_dds
