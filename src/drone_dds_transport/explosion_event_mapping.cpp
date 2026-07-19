#include "drone/dds_transport/explosion_event_mapping.h"

#include "drone/domain/drone_id.h"
#include "drone/domain/explosion_event_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

#include <chrono>
#include <cmath>
#include <expected>
#include <utility>

namespace drone::dds_transport
{
namespace
{

[[nodiscard]] bool hasFiniteCoordinates(const dds::CartesianPosition &position)
{
    return std::isfinite(position.x()) && std::isfinite(position.y()) &&
           std::isfinite(position.z());
}

} // namespace

dds::ExplosionEvent toWireExplosionEvent(const domain::ExplosionEvent &event)
{
    dds::CartesianPosition wirePosition;
    wirePosition.x(event.position().xMeters());
    wirePosition.y(event.position().yMeters());
    wirePosition.z(event.position().zMeters());

    dds::ExplosionEvent wireEvent;
    wireEvent.event_id(event.eventId().value());
    wireEvent.drone_id(event.droneId().value());
    wireEvent.target_id(event.targetId().value());
    wireEvent.position(std::move(wirePosition));
    wireEvent.occurred_at_ms(event.occurredAt().timeSinceUnixEpoch().count());
    return wireEvent;
}

std::expected<domain::ExplosionEvent, ExplosionEventMappingError>
fromWireExplosionEvent(const dds::ExplosionEvent &event)
{
    if (event.event_id() == 0)
    {
        return std::unexpected{ExplosionEventMappingError::zeroEventId};
    }
    if (event.drone_id() == 0)
    {
        return std::unexpected{ExplosionEventMappingError::zeroDroneId};
    }
    if (event.target_id() == 0)
    {
        return std::unexpected{ExplosionEventMappingError::zeroTargetId};
    }
    if (!hasFiniteCoordinates(event.position()))
    {
        return std::unexpected{ExplosionEventMappingError::nonFinitePosition};
    }
    if (event.occurred_at_ms() < 0)
    {
        return std::unexpected{ExplosionEventMappingError::timestampBeforeUnixEpoch};
    }

    return domain::ExplosionEvent{
        domain::ExplosionEventId{event.event_id()},
        domain::DroneId{event.drone_id()},
        domain::TargetId{event.target_id()},
        domain::Position{event.position().x(), event.position().y(), event.position().z()},
        domain::Timestamp{std::chrono::milliseconds{event.occurred_at_ms()}},
    };
}

} // namespace drone::dds_transport
