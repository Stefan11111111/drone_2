#ifndef DRONE_DOMAIN_EXPLOSION_EVENT_H
#define DRONE_DOMAIN_EXPLOSION_EVENT_H

#include "drone/domain/drone_id.h"
#include "drone/domain/explosion_event_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

namespace drone::domain
{

class ExplosionEvent final
{
  public:
    ExplosionEvent(ExplosionEventId eventId, DroneId droneId, TargetId targetId, Position position,
                   Timestamp occurredAt);

    [[nodiscard]] const ExplosionEventId &eventId() const noexcept;
    [[nodiscard]] const DroneId &droneId() const noexcept;
    [[nodiscard]] const TargetId &targetId() const noexcept;
    [[nodiscard]] const Position &position() const noexcept;
    [[nodiscard]] const Timestamp &occurredAt() const noexcept;

    bool operator==(const ExplosionEvent &) const = default;

  private:
    ExplosionEventId eventId_;
    DroneId droneId_;
    TargetId targetId_;
    Position position_;
    Timestamp occurredAt_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_EXPLOSION_EVENT_H
