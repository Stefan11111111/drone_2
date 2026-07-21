#include "drone/console_core/assignment_use_case.h"

#include "drone/domain/assignment.h"
#include "drone/domain/drone_state.h"

namespace drone::console
{

AssignmentUseCase::AssignmentUseCase(const TargetProjection &targets, const DroneProjection &drones,
                                     AssignmentOutputPort &output) noexcept
    : targets_{targets}, drones_{drones}, output_{output}
{
}

AssignmentResult AssignmentUseCase::assign(const domain::DroneId droneId,
                                           const domain::TargetId targetId)
{
    const auto drone = drones_.latestDrone(droneId);
    if (!drone.has_value())
    {
        return AssignmentResult::unknownDrone;
    }
    if (!targets_.latestTarget(targetId).has_value())
    {
        return AssignmentResult::unknownTarget;
    }

    const auto pending = pendingAssignments_.find(droneId);
    if (pending != pendingAssignments_.end())
    {
        return pending->second == targetId ? AssignmentResult::duplicate
                                           : AssignmentResult::unavailableDrone;
    }
    if (drone->status() != domain::DroneStatus::available)
    {
        return AssignmentResult::unavailableDrone;
    }

    const domain::Assignment assignment{droneId, targetId};
    output_.publish(assignment);
    pendingAssignments_.emplace(droneId, targetId);
    return AssignmentResult::assigned;
}

} // namespace drone::console
