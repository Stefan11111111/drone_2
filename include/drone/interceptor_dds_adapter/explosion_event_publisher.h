#ifndef DRONE_INTERCEPTOR_DDS_ADAPTER_EXPLOSION_EVENT_PUBLISHER_H
#define DRONE_INTERCEPTOR_DDS_ADAPTER_EXPLOSION_EVENT_PUBLISHER_H

#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/explosion_event_writer.h"
#include "drone/interceptor_core/explosion_event_output_port.h"

#include <cstdint>
#include <string_view>

namespace drone::interceptor_dds
{

class ExplosionEventPublisher final : public interceptor::ExplosionEventOutputPort
{
  public:
    ExplosionEventPublisher(std::uint32_t domainId, std::string_view participantName);
    ~ExplosionEventPublisher() noexcept override;

    ExplosionEventPublisher(const ExplosionEventPublisher &) = delete;
    ExplosionEventPublisher &operator=(const ExplosionEventPublisher &) = delete;
    ExplosionEventPublisher(ExplosionEventPublisher &&) = delete;
    ExplosionEventPublisher &operator=(ExplosionEventPublisher &&) = delete;

    void publish(const domain::ExplosionEvent &event) override;

  private:
    dds_transport::DomainParticipantOwner participant_;
    dds_transport::ExplosionEventWriter writer_;
};

} // namespace drone::interceptor_dds

#endif // DRONE_INTERCEPTOR_DDS_ADAPTER_EXPLOSION_EVENT_PUBLISHER_H
