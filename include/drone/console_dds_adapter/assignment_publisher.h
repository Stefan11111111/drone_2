#ifndef DRONE_CONSOLE_DDS_ADAPTER_ASSIGNMENT_PUBLISHER_H
#define DRONE_CONSOLE_DDS_ADAPTER_ASSIGNMENT_PUBLISHER_H

#include "drone/console_core/assignment_output_port.h"
#include "drone/dds_transport/assignment_writer.h"
#include "drone/dds_transport/domain_participant_owner.h"

#include <chrono>
#include <cstdint>
#include <string_view>

namespace drone::console_dds
{

class AssignmentPublisher final : public console::AssignmentOutputPort
{
  public:
    AssignmentPublisher(std::uint32_t domainId, std::string_view participantName);
    ~AssignmentPublisher() noexcept override;

    AssignmentPublisher(const AssignmentPublisher &) = delete;
    AssignmentPublisher &operator=(const AssignmentPublisher &) = delete;
    AssignmentPublisher(AssignmentPublisher &&) = delete;
    AssignmentPublisher &operator=(AssignmentPublisher &&) = delete;

    [[nodiscard]] bool waitForInterceptorMatch(std::chrono::milliseconds timeout);
    void publish(const domain::Assignment &assignment) override;

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::AssignmentWriter writer_;
};

} // namespace drone::console_dds

#endif // DRONE_CONSOLE_DDS_ADAPTER_ASSIGNMENT_PUBLISHER_H
