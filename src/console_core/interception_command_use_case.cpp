#include "drone/console_core/interception_command_use_case.h"

#include "drone/domain/drone_state.h"
#include "drone/domain/interception_command.h"
#include "drone/domain/interception_command_id.h"

#include <limits>
#include <stdexcept>

namespace drone::console
{

InterceptionCommandUseCase::InterceptionCommandUseCase(
    const DroneProjection &drones, InterceptionCommandOutputPort &output) noexcept
    : drones_{drones}, output_{output}
{
}

StartInterceptionResult InterceptionCommandUseCase::start(const domain::DroneId droneId)
{
    const auto drone = drones_.latestDrone(droneId);
    if (!drone.has_value())
    {
        return StartInterceptionResult::unknownDrone;
    }
    if (pendingStarts_.contains(droneId))
    {
        return StartInterceptionResult::duplicate;
    }
    if (drone->status() != domain::DroneStatus::assigned)
    {
        return StartInterceptionResult::droneNotAssigned;
    }
    if (nextCommandId_ == std::numeric_limits<std::uint64_t>::max())
    {
        throw std::overflow_error{"The console exhausted interception command identifiers"};
    }

    const domain::InterceptionCommand command{domain::InterceptionCommandId{nextCommandId_},
                                              droneId, *drone->assignedTargetId()};
    output_.publish(command);
    pendingStarts_.insert(droneId);
    ++nextCommandId_;
    return StartInterceptionResult::started;
}

} // namespace drone::console
