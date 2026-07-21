#include "drone/interceptor_core/interceptor_state_machine.h"

#include "drone/domain/assignment.h"
#include "drone/domain/drone_state.h"

#include <optional>
#include <stdexcept>

namespace drone::interceptor
{

InterceptorStateMachine::InterceptorStateMachine(const domain::DroneId droneId,
                                                 PositioningPort &positioning,
                                                 FlightControlPort &flightControl,
                                                 DroneStateOutputPort &stateOutput)
    : droneId_{droneId}, positioning_{positioning}, flightControl_{flightControl},
      stateOutput_{stateOutput}
{
}

void InterceptorStateMachine::start()
{
    if (state_.has_value())
    {
        throw std::logic_error{"The interceptor state machine is already started"};
    }

    const auto position = positioning_.currentPosition();
    state_.emplace(droneId_, position.position, position.measuredAt, domain::DroneStatus::available,
                   std::nullopt);
    stateOutput_.publish(*state_);
}

AssignmentHandlingResult InterceptorStateMachine::onAssignment(const domain::Assignment &assignment)
{
    if (assignment.droneId() != droneId_)
    {
        return AssignmentHandlingResult::wrongDrone;
    }
    if (!state_.has_value())
    {
        return AssignmentHandlingResult::notStarted;
    }
    if (state_->assignedTargetId().has_value())
    {
        return *state_->assignedTargetId() == assignment.targetId()
                   ? AssignmentHandlingResult::duplicate
                   : AssignmentHandlingResult::conflicting;
    }

    const auto position = positioning_.currentPosition();
    state_.emplace(droneId_, position.position, position.measuredAt, domain::DroneStatus::assigned,
                   assignment.targetId());
    stateOutput_.publish(*state_);
    return AssignmentHandlingResult::applied;
}

const std::optional<domain::DroneState> &InterceptorStateMachine::state() const noexcept
{
    return state_;
}

} // namespace drone::interceptor
