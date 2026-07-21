#include "drone/interceptor_dds_adapter/interception_command_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::interceptor_dds
{

InterceptionCommandSubscriber::InterceptionCommandSubscriber(
    const std::uint32_t domainId, const std::string_view participantName,
    interceptor::InterceptionCommandInputPort &input)
    : participant_{static_cast<eprosima::fastdds::dds::DomainId_t>(domainId), participantName},
      reader_{participant_.participant()}, input_{input}
{
}

InterceptionCommandSubscriber::~InterceptionCommandSubscriber() noexcept = default;

bool InterceptionCommandSubscriber::waitForConsoleMatch(const std::chrono::milliseconds timeout)
{
    return reader_.waitForWriterMatch(timeout);
}

InterceptionCommandReceiveResult
InterceptionCommandSubscriber::receiveNext(const std::chrono::milliseconds timeout)
{
    if (!reader_.waitForData(timeout))
    {
        return std::unexpected{InterceptionCommandReceiveIssue::timedOut};
    }

    auto sample = reader_.takeNext();
    if (!sample.has_value())
    {
        return std::unexpected{InterceptionCommandReceiveIssue::discardedMalformedData};
    }
    if (!sample->has_value())
    {
        return std::unexpected{InterceptionCommandReceiveIssue::discardedInvalidData};
    }

    const auto [entry, inserted] = deliveredCommandIds_.insert((*sample)->commandId());
    static_cast<void>(entry);
    if (!inserted)
    {
        return InterceptionCommandDelivery::duplicate;
    }

    input_.onInterceptionCommand(**sample);
    return InterceptionCommandDelivery::delivered;
}

} // namespace drone::interceptor_dds
