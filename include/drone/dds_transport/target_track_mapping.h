#ifndef DRONE_DDS_TRANSPORT_TARGET_TRACK_MAPPING_H
#define DRONE_DDS_TRANSPORT_TARGET_TRACK_MAPPING_H

#include "drone/domain/target_track.h"

#include <target_track.hpp>

#include <cstdint>
#include <expected>

namespace drone::dds_transport
{

enum class TargetTrackMappingError : std::uint8_t
{
    zeroTargetId,
    nonFinitePosition,
    timestampBeforeUnixEpoch,
};

[[nodiscard]] dds::TargetTrack toWireTargetTrack(const domain::TargetTrack &track);

[[nodiscard]] std::expected<domain::TargetTrack, TargetTrackMappingError>
fromWireTargetTrack(const dds::TargetTrack &track);

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_TARGET_TRACK_MAPPING_H
