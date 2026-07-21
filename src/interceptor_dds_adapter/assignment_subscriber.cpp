#include "drone/interceptor_dds_adapter/assignment_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::interceptor_dds
{

AssignmentSubscriber::AssignmentSubscriber(const std::uint32_t domainId,
                                           const std::string_view participantName,
                                           interceptor::AssignmentInputPort &input)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      reader_{participant_.participant()}, input_{input}
{
}

AssignmentSubscriber::~AssignmentSubscriber() noexcept = default;

bool AssignmentSubscriber::waitForConsoleMatch(const std::chrono::milliseconds timeout)
{
    return reader_.waitForWriterMatch(timeout);
}

AssignmentReceiveResult AssignmentSubscriber::receiveNext(const std::chrono::milliseconds timeout)
{
    if (!reader_.waitForData(timeout))
    {
        return std::unexpected{AssignmentReceiveIssue::timedOut};
    }

    auto sample = reader_.takeNext();
    if (!sample.has_value())
    {
        return std::unexpected{AssignmentReceiveIssue::discardedMalformedData};
    }
    if (!sample->has_value())
    {
        return std::unexpected{AssignmentReceiveIssue::discardedInvalidData};
    }

    static_cast<void>(input_.onAssignment(**sample));
    return {};
}

} // namespace drone::interceptor_dds
