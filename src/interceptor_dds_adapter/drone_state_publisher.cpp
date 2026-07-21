#include "drone/interceptor_dds_adapter/drone_state_publisher.h"

#include <fastdds/dds/core/Types.hpp>

#include <cstdint>
#include <string_view>

namespace drone::interceptor_dds
{

DroneStatePublisher::DroneStatePublisher(const std::uint32_t domainId,
                                         const std::string_view participantName)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      writer_{participant_.participant()}
{
}

DroneStatePublisher::~DroneStatePublisher() noexcept = default;

void DroneStatePublisher::publish(const domain::DroneState &state)
{
    writer_.write(state);
}

} // namespace drone::interceptor_dds
