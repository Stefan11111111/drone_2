#include "drone/dds_transport/interception_command_mapping.h"

#include "drone/domain/drone_id.h"
#include "drone/domain/interception_command_id.h"
#include "drone/domain/target_id.h"

#include <expected>

namespace drone::dds_transport
{

dds::InterceptionCommand toWireInterceptionCommand(const domain::InterceptionCommand &command)
{
    dds::InterceptionCommand wireCommand;
    wireCommand.command_id(command.commandId().value());
    wireCommand.drone_id(command.droneId().value());
    wireCommand.target_id(command.targetId().value());
    return wireCommand;
}

std::expected<domain::InterceptionCommand, InterceptionCommandMappingError>
fromWireInterceptionCommand(const dds::InterceptionCommand &command)
{
    if (command.command_id() == 0)
    {
        return std::unexpected{InterceptionCommandMappingError::zeroCommandId};
    }
    if (command.drone_id() == 0)
    {
        return std::unexpected{InterceptionCommandMappingError::zeroDroneId};
    }
    if (command.target_id() == 0)
    {
        return std::unexpected{InterceptionCommandMappingError::zeroTargetId};
    }

    return domain::InterceptionCommand{domain::InterceptionCommandId{command.command_id()},
                                       domain::DroneId{command.drone_id()},
                                       domain::TargetId{command.target_id()}};
}

} // namespace drone::dds_transport
