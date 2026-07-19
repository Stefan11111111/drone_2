#ifndef DRONE_DDS_TRANSPORT_DRONE_STATE_MAPPING_H
#define DRONE_DDS_TRANSPORT_DRONE_STATE_MAPPING_H

#include "drone/domain/drone_state.h"

#include <target_track.hpp>

#include <cstdint>
#include <expected>

namespace drone::dds_transport
{

enum class DroneStateMappingError : std::uint8_t
{
    zeroDroneId,
    nonFinitePosition,
    timestampBeforeUnixEpoch,
    unknownStatus,
    zeroAssignedTargetId,
    assignmentDoesNotMatchStatus,
};

[[nodiscard]] dds::DroneState toWireDroneState(const domain::DroneState &state);

[[nodiscard]] std::expected<domain::DroneState, DroneStateMappingError>
fromWireDroneState(const dds::DroneState &state);

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_DRONE_STATE_MAPPING_H
