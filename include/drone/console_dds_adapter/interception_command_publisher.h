#ifndef DRONE_CONSOLE_DDS_ADAPTER_INTERCEPTION_COMMAND_PUBLISHER_H
#define DRONE_CONSOLE_DDS_ADAPTER_INTERCEPTION_COMMAND_PUBLISHER_H

#include "drone/console_core/interception_command_output_port.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/interception_command_writer.h"

#include <chrono>
#include <cstdint>
#include <string_view>

namespace drone::console_dds
{

class InterceptionCommandPublisher final : public console::InterceptionCommandOutputPort
{
  public:
    InterceptionCommandPublisher(std::uint32_t domainId, std::string_view participantName);
    ~InterceptionCommandPublisher() noexcept override;

    InterceptionCommandPublisher(const InterceptionCommandPublisher &) = delete;
    InterceptionCommandPublisher &operator=(const InterceptionCommandPublisher &) = delete;
    InterceptionCommandPublisher(InterceptionCommandPublisher &&) = delete;
    InterceptionCommandPublisher &operator=(InterceptionCommandPublisher &&) = delete;

    [[nodiscard]] bool waitForInterceptorMatch(std::chrono::milliseconds timeout);
    void publish(const domain::InterceptionCommand &command) override;

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::InterceptionCommandWriter writer_;
};

} // namespace drone::console_dds

#endif // DRONE_CONSOLE_DDS_ADAPTER_INTERCEPTION_COMMAND_PUBLISHER_H
