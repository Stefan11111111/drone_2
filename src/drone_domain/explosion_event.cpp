#include "drone/domain/explosion_event.h"

namespace drone::domain
{

ExplosionEvent::ExplosionEvent(const ExplosionEventId eventId, const DroneId droneId,
                               const TargetId targetId, const Position position,
                               const Timestamp occurredAt)
    : eventId_{eventId}, droneId_{droneId}, targetId_{targetId}, position_{position},
      occurredAt_{occurredAt}
{
}

const ExplosionEventId &ExplosionEvent::eventId() const noexcept
{
    return eventId_;
}

const DroneId &ExplosionEvent::droneId() const noexcept
{
    return droneId_;
}

const TargetId &ExplosionEvent::targetId() const noexcept
{
    return targetId_;
}

const Position &ExplosionEvent::position() const noexcept
{
    return position_;
}

const Timestamp &ExplosionEvent::occurredAt() const noexcept
{
    return occurredAt_;
}

} // namespace drone::domain
