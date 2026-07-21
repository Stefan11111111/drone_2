#include "drone/console_dds_adapter/interception_command_publisher.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstdint>
#include <string_view>

namespace drone::console_dds
{

InterceptionCommandPublisher::InterceptionCommandPublisher(const std::uint32_t domainId,
                                                           const std::string_view participantName)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      writer_{participant_.participant()}
{
}

InterceptionCommandPublisher::~InterceptionCommandPublisher() noexcept = default;

bool InterceptionCommandPublisher::waitForInterceptorMatch(const std::chrono::milliseconds timeout)
{
    return writer_.waitForReaderMatch(timeout);
}

void InterceptionCommandPublisher::publish(const domain::InterceptionCommand &command)
{
    writer_.write(command);
}

} // namespace drone::console_dds
