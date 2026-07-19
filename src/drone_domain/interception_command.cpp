#include "drone/domain/interception_command.h"

namespace drone::domain
{

InterceptionCommand::InterceptionCommand(const InterceptionCommandId commandId,
                                         const DroneId droneId, const TargetId targetId)
    : commandId_{commandId}, droneId_{droneId}, targetId_{targetId}
{
}

const InterceptionCommandId &InterceptionCommand::commandId() const noexcept
{
    return commandId_;
}

const DroneId &InterceptionCommand::droneId() const noexcept
{
    return droneId_;
}

const TargetId &InterceptionCommand::targetId() const noexcept
{
    return targetId_;
}

} // namespace drone::domain
