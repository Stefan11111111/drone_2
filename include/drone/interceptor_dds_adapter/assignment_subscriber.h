#ifndef DRONE_INTERCEPTOR_DDS_ADAPTER_ASSIGNMENT_SUBSCRIBER_H
#define DRONE_INTERCEPTOR_DDS_ADAPTER_ASSIGNMENT_SUBSCRIBER_H

#include "drone/dds_transport/assignment_reader.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/interceptor_core/assignment_input_port.h"

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::interceptor_dds
{

enum class AssignmentReceiveIssue : std::uint8_t
{
    timedOut,
    discardedInvalidData,
    discardedMalformedData,
};

using AssignmentReceiveResult = std::expected<void, AssignmentReceiveIssue>;

class AssignmentSubscriber final
{
  public:
    AssignmentSubscriber(std::uint32_t domainId, std::string_view participantName,
                         interceptor::AssignmentInputPort &input);
    ~AssignmentSubscriber() noexcept;

    AssignmentSubscriber(const AssignmentSubscriber &) = delete;
    AssignmentSubscriber &operator=(const AssignmentSubscriber &) = delete;
    AssignmentSubscriber(AssignmentSubscriber &&) = delete;
    AssignmentSubscriber &operator=(AssignmentSubscriber &&) = delete;

    [[nodiscard]] bool waitForConsoleMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] AssignmentReceiveResult receiveNext(std::chrono::milliseconds timeout);

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::AssignmentReader reader_;
    interceptor::AssignmentInputPort &input_;
};

} // namespace drone::interceptor_dds

#endif // DRONE_INTERCEPTOR_DDS_ADAPTER_ASSIGNMENT_SUBSCRIBER_H
