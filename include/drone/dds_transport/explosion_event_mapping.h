#ifndef DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_MAPPING_H
#define DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_MAPPING_H

#include "drone/domain/explosion_event.h"

#include <target_track.hpp>

#include <cstdint>
#include <expected>

namespace drone::dds_transport
{

enum class ExplosionEventMappingError : std::uint8_t
{
    zeroEventId,
    zeroDroneId,
    zeroTargetId,
    nonFinitePosition,
    timestampBeforeUnixEpoch,
};

[[nodiscard]] dds::ExplosionEvent toWireExplosionEvent(const domain::ExplosionEvent &event);

[[nodiscard]] std::expected<domain::ExplosionEvent, ExplosionEventMappingError>
fromWireExplosionEvent(const dds::ExplosionEvent &event);

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_MAPPING_H
