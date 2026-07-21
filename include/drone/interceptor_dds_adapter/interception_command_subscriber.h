#ifndef DRONE_INTERCEPTOR_DDS_ADAPTER_INTERCEPTION_COMMAND_SUBSCRIBER_H
#define DRONE_INTERCEPTOR_DDS_ADAPTER_INTERCEPTION_COMMAND_SUBSCRIBER_H

#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/interception_command_reader.h"
#include "drone/domain/interception_command_id.h"
#include "drone/interceptor_core/interception_command_input_port.h"

#include <chrono>
#include <cstdint>
#include <expected>
#include <set>
#include <string_view>

namespace drone::interceptor_dds
{

enum class InterceptionCommandDelivery : std::uint8_t
{
    delivered,
    duplicate,
};

enum class InterceptionCommandReceiveIssue : std::uint8_t
{
    timedOut,
    discardedInvalidData,
    discardedMalformedData,
};

using InterceptionCommandReceiveResult =
    std::expected<InterceptionCommandDelivery, InterceptionCommandReceiveIssue>;

class InterceptionCommandSubscriber final
{
  public:
    InterceptionCommandSubscriber(std::uint32_t domainId, std::string_view participantName,
                                  interceptor::InterceptionCommandInputPort &input);
    ~InterceptionCommandSubscriber() noexcept;

    InterceptionCommandSubscriber(const InterceptionCommandSubscriber &) = delete;
    InterceptionCommandSubscriber &operator=(const InterceptionCommandSubscriber &) = delete;
    InterceptionCommandSubscriber(InterceptionCommandSubscriber &&) = delete;
    InterceptionCommandSubscriber &operator=(InterceptionCommandSubscriber &&) = delete;

    [[nodiscard]] bool waitForConsoleMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] InterceptionCommandReceiveResult receiveNext(std::chrono::milliseconds timeout);

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::InterceptionCommandReader reader_;
    interceptor::InterceptionCommandInputPort &input_;
    std::set<domain::InterceptionCommandId> deliveredCommandIds_;
};

} // namespace drone::interceptor_dds

#endif // DRONE_INTERCEPTOR_DDS_ADAPTER_INTERCEPTION_COMMAND_SUBSCRIBER_H
