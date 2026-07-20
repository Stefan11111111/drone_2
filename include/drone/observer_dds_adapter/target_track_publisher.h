#ifndef DRONE_OBSERVER_DDS_ADAPTER_TARGET_TRACK_PUBLISHER_H
#define DRONE_OBSERVER_DDS_ADAPTER_TARGET_TRACK_PUBLISHER_H

#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_writer.h"
#include "drone/observer_core/target_track_output_port.h"

#include <cstdint>
#include <string_view>

namespace drone::observer_dds
{

class TargetTrackPublisher final : public observer::TargetTrackOutputPort
{
  public:
    TargetTrackPublisher(std::uint32_t domainId, std::string_view participantName);
    ~TargetTrackPublisher() noexcept override;

    TargetTrackPublisher(const TargetTrackPublisher &) = delete;
    TargetTrackPublisher &operator=(const TargetTrackPublisher &) = delete;
    TargetTrackPublisher(TargetTrackPublisher &&) = delete;
    TargetTrackPublisher &operator=(TargetTrackPublisher &&) = delete;

    void publish(const domain::TargetTrack &targetTrack) override;

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::TargetTrackWriter writer_;
};

} // namespace drone::observer_dds

#endif // DRONE_OBSERVER_DDS_ADAPTER_TARGET_TRACK_PUBLISHER_H
