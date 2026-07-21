#include "drone/console_core/drone_projection.h"

namespace drone::console
{

DroneUpdateResult DroneProjection::onDroneState(const domain::DroneState &state)
{
    auto [current, inserted] = droneStates_.try_emplace(state.droneId(), state);
    if (inserted)
    {
        return DroneUpdateResult::added;
    }

    if (state == current->second)
    {
        return DroneUpdateResult::duplicate;
    }

    if (state.reportedAt() < current->second.reportedAt())
    {
        return DroneUpdateResult::stale;
    }

    if (state.reportedAt() == current->second.reportedAt())
    {
        return DroneUpdateResult::conflicting;
    }

    current->second = state;
    return DroneUpdateResult::updated;
}

std::optional<domain::DroneState> DroneProjection::latestDrone(const domain::DroneId droneId) const
{
    const auto current = droneStates_.find(droneId);
    if (current == droneStates_.end())
    {
        return std::nullopt;
    }
    return current->second;
}

std::vector<domain::DroneState> DroneProjection::droneStates() const
{
    std::vector<domain::DroneState> snapshot;
    snapshot.reserve(droneStates_.size());
    for (const auto &entry : droneStates_)
    {
        snapshot.push_back(entry.second);
    }
    return snapshot;
}

} // namespace drone::console
