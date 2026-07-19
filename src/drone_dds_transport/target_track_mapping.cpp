#include "drone/dds_transport/target_track_mapping.h"

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

dds::TargetTrack toWireTargetTrack(const domain::TargetTrack &track)
{
    dds::CartesianPosition wirePosition;
    wirePosition.x(track.position().xMeters());
    wirePosition.y(track.position().yMeters());
    wirePosition.z(track.position().zMeters());

    dds::TargetTrack wireTrack;
    wireTrack.target_id(track.targetId().value());
    wireTrack.position(std::move(wirePosition));
    wireTrack.measured_at_ms(track.measuredAt().timeSinceUnixEpoch().count());
    return wireTrack;
}

std::expected<domain::TargetTrack, TargetTrackMappingError>
fromWireTargetTrack(const dds::TargetTrack &track)
{
    if (track.target_id() == 0)
    {
        return std::unexpected{TargetTrackMappingError::zeroTargetId};
    }
    if (!hasFiniteCoordinates(track.position()))
    {
        return std::unexpected{TargetTrackMappingError::nonFinitePosition};
    }
    if (track.measured_at_ms() < 0)
    {
        return std::unexpected{TargetTrackMappingError::timestampBeforeUnixEpoch};
    }

    return domain::TargetTrack{
        domain::TargetId{track.target_id()},
        domain::Position{track.position().x(), track.position().y(), track.position().z()},
        domain::Timestamp{std::chrono::milliseconds{track.measured_at_ms()}},
    };
}

} // namespace drone::dds_transport
