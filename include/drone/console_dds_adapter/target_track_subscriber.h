#ifndef DRONE_CONSOLE_DDS_ADAPTER_TARGET_TRACK_SUBSCRIBER_H
#define DRONE_CONSOLE_DDS_ADAPTER_TARGET_TRACK_SUBSCRIBER_H

#include "drone/console_core/target_track_input_port.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_reader.h"

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::console_dds
{

enum class TargetTrackReceiveIssue : std::uint8_t
{
    timedOut,
    discardedInvalidData,
    discardedMalformedData,
};

using TargetTrackReceiveResult =
    std::expected<console::TargetUpdateResult, TargetTrackReceiveIssue>;

class TargetTrackSubscriber final
{
  public:
    TargetTrackSubscriber(std::uint32_t domainId, std::string_view participantName,
                          console::TargetTrackInputPort &input);
    ~TargetTrackSubscriber() noexcept;

    TargetTrackSubscriber(const TargetTrackSubscriber &) = delete;
    TargetTrackSubscriber &operator=(const TargetTrackSubscriber &) = delete;
    TargetTrackSubscriber(TargetTrackSubscriber &&) = delete;
    TargetTrackSubscriber &operator=(TargetTrackSubscriber &&) = delete;

    [[nodiscard]] bool waitForWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] TargetTrackReceiveResult receiveNext(std::chrono::milliseconds timeout);

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::TargetTrackReader reader_;
    console::TargetTrackInputPort &input_;
};

} // namespace drone::console_dds

#endif // DRONE_CONSOLE_DDS_ADAPTER_TARGET_TRACK_SUBSCRIBER_H
