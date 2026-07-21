#ifndef DRONE_CONSOLE_DDS_ADAPTER_EXPLOSION_EVENT_SUBSCRIBER_H
#define DRONE_CONSOLE_DDS_ADAPTER_EXPLOSION_EVENT_SUBSCRIBER_H

#include "drone/console_core/explosion_event_input_port.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/explosion_event_reader.h"

#include <chrono>
#include <cstdint>
#include <expected>
#include <string_view>

namespace drone::console_dds
{

enum class ExplosionEventReceiveIssue : std::uint8_t
{
    timedOut,
    discardedInvalidData,
    discardedMalformedData,
};

using ExplosionEventReceiveResult =
    std::expected<console::OutcomeUpdateResult, ExplosionEventReceiveIssue>;

class ExplosionEventSubscriber final
{
  public:
    ExplosionEventSubscriber(std::uint32_t domainId, std::string_view participantName,
                             console::ExplosionEventInputPort &input);
    ~ExplosionEventSubscriber() noexcept;

    ExplosionEventSubscriber(const ExplosionEventSubscriber &) = delete;
    ExplosionEventSubscriber &operator=(const ExplosionEventSubscriber &) = delete;
    ExplosionEventSubscriber(ExplosionEventSubscriber &&) = delete;
    ExplosionEventSubscriber &operator=(ExplosionEventSubscriber &&) = delete;

    [[nodiscard]] bool waitForWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] ExplosionEventReceiveResult receiveNext(std::chrono::milliseconds timeout);

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::ExplosionEventReader reader_;
    console::ExplosionEventInputPort &input_;
};

} // namespace drone::console_dds

#endif // DRONE_CONSOLE_DDS_ADAPTER_EXPLOSION_EVENT_SUBSCRIBER_H
