#include "drone/interceptor_dds_adapter/target_track_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::interceptor_dds
{

TargetTrackSubscriber::TargetTrackSubscriber(const std::uint32_t domainId,
                                             const std::string_view participantName,
                                             interceptor::TargetTrackInputPort &input)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      reader_{participant_.participant()}, input_{input}
{
}

TargetTrackSubscriber::~TargetTrackSubscriber() noexcept = default;

bool TargetTrackSubscriber::waitForObserverMatch(const std::chrono::milliseconds timeout)
{
    return reader_.waitForWriterMatch(timeout);
}

TargetTrackReceiveResult TargetTrackSubscriber::receiveNext(const std::chrono::milliseconds timeout)
{
    if (!reader_.waitForData(timeout))
    {
        return std::unexpected{TargetTrackReceiveIssue::timedOut};
    }

    auto sample = reader_.takeNext();
    if (!sample.has_value())
    {
        return std::unexpected{TargetTrackReceiveIssue::discardedMalformedData};
    }
    if (!sample->has_value())
    {
        return std::unexpected{TargetTrackReceiveIssue::discardedInvalidData};
    }
    return input_.onTargetTrack(**sample);
}

} // namespace drone::interceptor_dds
