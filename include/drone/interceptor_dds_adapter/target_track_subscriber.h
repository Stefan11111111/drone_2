#ifndef DRONE_INTERCEPTOR_DDS_ADAPTER_TARGET_TRACK_SUBSCRIBER_H
#define DRONE_INTERCEPTOR_DDS_ADAPTER_TARGET_TRACK_SUBSCRIBER_H

#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_reader.h"
#include "drone/interceptor_core/target_track_input_port.h"

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::interceptor_dds
{

enum class TargetTrackReceiveIssue : std::uint8_t
{
    timedOut,
    discardedInvalidData,
    discardedMalformedData,
};

using TargetTrackReceiveResult =
    std::expected<interceptor::TargetTrackHandlingResult, TargetTrackReceiveIssue>;

class TargetTrackSubscriber final
{
  public:
    TargetTrackSubscriber(std::uint32_t domainId, std::string_view participantName,
                          interceptor::TargetTrackInputPort &input);
    ~TargetTrackSubscriber() noexcept;

    TargetTrackSubscriber(const TargetTrackSubscriber &) = delete;
    TargetTrackSubscriber &operator=(const TargetTrackSubscriber &) = delete;
    TargetTrackSubscriber(TargetTrackSubscriber &&) = delete;
    TargetTrackSubscriber &operator=(TargetTrackSubscriber &&) = delete;

    [[nodiscard]] bool waitForObserverMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] TargetTrackReceiveResult receiveNext(std::chrono::milliseconds timeout);

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::TargetTrackReader reader_;
    interceptor::TargetTrackInputPort &input_;
};

} // namespace drone::interceptor_dds

#endif // DRONE_INTERCEPTOR_DDS_ADAPTER_TARGET_TRACK_SUBSCRIBER_H
