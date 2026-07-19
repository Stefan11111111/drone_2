#include "drone/domain/drone_state.h"

#include <stdexcept>

namespace drone::domain
{
namespace
{

bool requiresAssignedTarget(const DroneStatus status)
{
    switch (status)
    {
    case DroneStatus::available:
        return false;
    case DroneStatus::assigned:
    case DroneStatus::intercepting:
    case DroneStatus::interceptionSucceeded:
    case DroneStatus::interceptionFailed:
        return true;
    }

    throw std::invalid_argument{"Unknown drone status"};
}

void validateTargetAssignment(const DroneStatus status,
                              const std::optional<TargetId> &assignedTargetId)
{
    if (requiresAssignedTarget(status) != assignedTargetId.has_value())
    {
        throw std::invalid_argument{status == DroneStatus::available
                                        ? "An available drone cannot have an assigned target"
                                        : "A non-available drone must have an assigned target"};
    }
}

} // namespace

DroneState::DroneState(const DroneId droneId, const Position position, const Timestamp reportedAt,
                       const DroneStatus status, std::optional<TargetId> assignedTargetId)
    : droneId_{droneId}, position_{position}, reportedAt_{reportedAt}, status_{status},
      assignedTargetId_{assignedTargetId}
{
    validateTargetAssignment(status_, assignedTargetId_);
}

const DroneId &DroneState::droneId() const noexcept
{
    return droneId_;
}

const Position &DroneState::position() const noexcept
{
    return position_;
}

const Timestamp &DroneState::reportedAt() const noexcept
{
    return reportedAt_;
}

DroneStatus DroneState::status() const noexcept
{
    return status_;
}

const std::optional<TargetId> &DroneState::assignedTargetId() const noexcept
{
    return assignedTargetId_;
}

} // namespace drone::domain
