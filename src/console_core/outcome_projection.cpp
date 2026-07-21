#include "drone/console_core/outcome_projection.h"

#include <optional>
#include <vector>

namespace drone::console
{

OutcomeProjection::OutcomeProjection(const TargetProjection &targets,
                                     const DroneProjection &drones) noexcept
    : targets_{targets}, drones_{drones}
{
}

OutcomeUpdateResult OutcomeProjection::onExplosionEvent(const domain::ExplosionEvent &event)
{
    const auto existing = outcomes_.find(event.eventId());
    if (existing != outcomes_.end())
    {
        return existing->second == event ? OutcomeUpdateResult::duplicate
                                         : OutcomeUpdateResult::conflicting;
    }

    const auto target = targets_.latestTarget(event.targetId());
    const auto drone = drones_.latestDrone(event.droneId());
    if (!target.has_value() || !drone.has_value() || drone->assignedTargetId() != event.targetId())
    {
        return OutcomeUpdateResult::unrelated;
    }

    outcomes_.emplace(event.eventId(), event);
    return OutcomeUpdateResult::recorded;
}

std::vector<domain::ExplosionEvent> OutcomeProjection::outcomes() const
{
    std::vector<domain::ExplosionEvent> snapshot;
    snapshot.reserve(outcomes_.size());
    for (const auto &entry : outcomes_)
    {
        snapshot.push_back(entry.second);
    }
    return snapshot;
}

} // namespace drone::console
