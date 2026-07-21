#include "drone/console_core/target_projection.h"

namespace drone::console
{

TargetUpdateResult TargetProjection::onTargetTrack(const domain::TargetTrack &targetTrack)
{
    auto [current, inserted] = targetTracks_.try_emplace(targetTrack.targetId(), targetTrack);
    if (inserted)
    {
        return TargetUpdateResult::added;
    }

    if (targetTrack == current->second)
    {
        return TargetUpdateResult::duplicate;
    }

    if (targetTrack.measuredAt() < current->second.measuredAt())
    {
        return TargetUpdateResult::stale;
    }

    if (targetTrack.measuredAt() == current->second.measuredAt())
    {
        return TargetUpdateResult::conflicting;
    }

    current->second = targetTrack;
    return TargetUpdateResult::updated;
}

std::optional<domain::TargetTrack>
TargetProjection::latestTarget(const domain::TargetId targetId) const
{
    const auto current = targetTracks_.find(targetId);
    if (current == targetTracks_.end())
    {
        return std::nullopt;
    }

    return current->second;
}

std::vector<domain::TargetTrack> TargetProjection::targetTracks() const
{
    std::vector<domain::TargetTrack> snapshot;
    snapshot.reserve(targetTracks_.size());
    for (const auto &entry : targetTracks_)
    {
        snapshot.push_back(entry.second);
    }
    return snapshot;
}

} // namespace drone::console
