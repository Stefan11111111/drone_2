#include "drone/console_dds_adapter/explosion_event_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::console_dds
{

ExplosionEventSubscriber::ExplosionEventSubscriber(const std::uint32_t domainId,
                                                   const std::string_view participantName,
                                                   console::ExplosionEventInputPort &input)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      reader_{participant_.participant()}, input_{input}
{
}

ExplosionEventSubscriber::~ExplosionEventSubscriber() noexcept = default;

bool ExplosionEventSubscriber::waitForWriterMatch(const std::chrono::milliseconds timeout)
{
    return reader_.waitForWriterMatch(timeout);
}

ExplosionEventReceiveResult
ExplosionEventSubscriber::receiveNext(const std::chrono::milliseconds timeout)
{
    if (!reader_.waitForData(timeout))
    {
        return std::unexpected{ExplosionEventReceiveIssue::timedOut};
    }

    auto sample = reader_.takeNext();
    if (!sample.has_value())
    {
        return std::unexpected{ExplosionEventReceiveIssue::discardedMalformedData};
    }
    if (!sample->has_value())
    {
        return std::unexpected{ExplosionEventReceiveIssue::discardedInvalidData};
    }
    return input_.onExplosionEvent(**sample);
}

} // namespace drone::console_dds
