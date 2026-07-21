#ifndef DRONE_CONSOLE_DDS_ADAPTER_CONSOLE_SUBSCRIBER_H
#define DRONE_CONSOLE_DDS_ADAPTER_CONSOLE_SUBSCRIBER_H

#include "drone/console_core/drone_state_input_port.h"
#include "drone/console_core/target_track_input_port.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/drone_state_reader.h"
#include "drone/dds_transport/target_track_reader.h"

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::console_dds
{

enum class ReceiveIssue : std::uint8_t
{
    timedOut,
    discardedInvalidData,
    discardedMalformedData,
};

using TargetTrackReceiveResult = std::expected<console::TargetUpdateResult, ReceiveIssue>;
using DroneStateReceiveResult = std::expected<console::DroneUpdateResult, ReceiveIssue>;

class ConsoleSubscriber final
{
  public:
    ConsoleSubscriber(std::uint32_t domainId, std::string_view participantName,
                      console::TargetTrackInputPort &targetInput,
                      console::DroneStateInputPort &droneInput);
    ~ConsoleSubscriber() noexcept;

    ConsoleSubscriber(const ConsoleSubscriber &) = delete;
    ConsoleSubscriber &operator=(const ConsoleSubscriber &) = delete;
    ConsoleSubscriber(ConsoleSubscriber &&) = delete;
    ConsoleSubscriber &operator=(ConsoleSubscriber &&) = delete;

    [[nodiscard]] bool waitForTargetWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] bool waitForDroneWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] TargetTrackReceiveResult receiveNextTarget(std::chrono::milliseconds timeout);
    [[nodiscard]] DroneStateReceiveResult receiveNextDrone(std::chrono::milliseconds timeout);

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::TargetTrackReader targetReader_;
    dds_transport::DroneStateReader droneReader_;
    console::TargetTrackInputPort &targetInput_;
    console::DroneStateInputPort &droneInput_;
};

} // namespace drone::console_dds

#endif // DRONE_CONSOLE_DDS_ADAPTER_CONSOLE_SUBSCRIBER_H
