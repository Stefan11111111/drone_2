#include "drone/domain/target_track.h"

namespace drone::domain
{

TargetTrack::TargetTrack(const TargetId targetId, const Position position,
                         const Timestamp measuredAt)
    : targetId_{targetId}, position_{position}, measuredAt_{measuredAt}
{
}

const TargetId &TargetTrack::targetId() const noexcept
{
    return targetId_;
}

const Position &TargetTrack::position() const noexcept
{
    return position_;
}

const Timestamp &TargetTrack::measuredAt() const noexcept
{
    return measuredAt_;
}

} // namespace drone::domain
