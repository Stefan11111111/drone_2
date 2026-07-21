#include "drone/console_dds_adapter/assignment_publisher.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstdint>
#include <string_view>

namespace drone::console_dds
{

AssignmentPublisher::AssignmentPublisher(const std::uint32_t domainId,
                                         const std::string_view participantName)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      writer_{participant_.participant()}
{
}

AssignmentPublisher::~AssignmentPublisher() noexcept = default;

bool AssignmentPublisher::waitForInterceptorMatch(const std::chrono::milliseconds timeout)
{
    return writer_.waitForReaderMatch(timeout);
}

void AssignmentPublisher::publish(const domain::Assignment &assignment)
{
    writer_.write(assignment);
}

} // namespace drone::console_dds
