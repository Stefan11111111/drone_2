#include "drone/observer_dds_adapter/target_track_publisher.h"

#include <fastdds/dds/core/Types.hpp>

#include <cstdint>
#include <string_view>

namespace drone::observer_dds
{

TargetTrackPublisher::TargetTrackPublisher(const std::uint32_t domainId,
                                           const std::string_view participantName)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      writer_{participant_.participant()}
{
}

TargetTrackPublisher::~TargetTrackPublisher() noexcept = default;

void TargetTrackPublisher::publish(const domain::TargetTrack &targetTrack)
{
    writer_.write(targetTrack);
}

} // namespace drone::observer_dds
