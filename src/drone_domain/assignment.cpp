#include "drone/domain/assignment.h"

namespace drone::domain
{

Assignment::Assignment(const DroneId droneId, const TargetId targetId)
    : droneId_{droneId}, targetId_{targetId}
{
}

const DroneId &Assignment::droneId() const noexcept
{
    return droneId_;
}

const TargetId &Assignment::targetId() const noexcept
{
    return targetId_;
}

} // namespace drone::domain
