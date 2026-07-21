#include "drone/interceptor_dds_adapter/explosion_event_publisher.h"

#include <fastdds/dds/core/Types.hpp>

#include <cstdint>
#include <string_view>

namespace drone::interceptor_dds
{

ExplosionEventPublisher::ExplosionEventPublisher(const std::uint32_t domainId,
                                                 const std::string_view participantName)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      writer_{participant_.participant()}
{
}

ExplosionEventPublisher::~ExplosionEventPublisher() noexcept = default;

void ExplosionEventPublisher::publish(const domain::ExplosionEvent &event)
{
    writer_.write(event);
}

} // namespace drone::interceptor_dds
