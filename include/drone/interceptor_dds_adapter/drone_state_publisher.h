#ifndef DRONE_INTERCEPTOR_DDS_ADAPTER_DRONE_STATE_PUBLISHER_H
#define DRONE_INTERCEPTOR_DDS_ADAPTER_DRONE_STATE_PUBLISHER_H

#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/drone_state_writer.h"
#include "drone/interceptor_core/drone_state_output_port.h"

#include <cstdint>
#include <string_view>

namespace drone::interceptor_dds
{

class DroneStatePublisher final : public interceptor::DroneStateOutputPort
{
  public:
    DroneStatePublisher(std::uint32_t domainId, std::string_view participantName);
    ~DroneStatePublisher() noexcept override;

    DroneStatePublisher(const DroneStatePublisher &) = delete;
    DroneStatePublisher &operator=(const DroneStatePublisher &) = delete;
    DroneStatePublisher(DroneStatePublisher &&) = delete;
    DroneStatePublisher &operator=(DroneStatePublisher &&) = delete;

    void publish(const domain::DroneState &state) override;

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::DroneStateWriter writer_;
};

} // namespace drone::interceptor_dds

#endif // DRONE_INTERCEPTOR_DDS_ADAPTER_DRONE_STATE_PUBLISHER_H
