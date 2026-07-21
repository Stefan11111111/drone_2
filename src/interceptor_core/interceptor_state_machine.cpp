#include "drone/interceptor_core/interceptor_state_machine.h"

#include "drone/domain/assignment.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/interception_command.h"
#include "drone/domain/target_track.h"

#include <cmath>
#include <optional>
#include <stdexcept>

namespace drone::interceptor
{

InterceptorStateMachine::InterceptorStateMachine(const InterceptorConfiguration configuration,
                                                 PositioningPort &positioning,
                                                 FlightControlPort &flightControl,
                                                 InterceptionEffectPort &effect,
                                                 DroneStateOutputPort &stateOutput)
    : droneId_{configuration.droneId}, positioning_{positioning}, flightControl_{flightControl},
      effect_{effect}, stateOutput_{stateOutput},
      arrivalToleranceMeters_{configuration.arrivalToleranceMeters}
{
    if (!std::isfinite(arrivalToleranceMeters_) || arrivalToleranceMeters_ < 0.0)
    {
        throw std::invalid_argument{
            "The interceptor arrival tolerance must be finite and nonnegative"};
    }
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

TargetTrackHandlingResult InterceptorStateMachine::onTargetTrack(const domain::TargetTrack &track)
{
    if (!state_.has_value() || !state_->assignedTargetId().has_value() ||
        track.targetId() != *state_->assignedTargetId())
    {
        return TargetTrackHandlingResult::unrelated;
    }
    if (!latestTargetTrack_.has_value())
    {
        latestTargetTrack_ = track;
        return TargetTrackHandlingResult::accepted;
    }
    if (track == *latestTargetTrack_)
    {
        return TargetTrackHandlingResult::duplicate;
    }
    if (track.measuredAt() < latestTargetTrack_->measuredAt())
    {
        return TargetTrackHandlingResult::stale;
    }
    if (track.measuredAt() == latestTargetTrack_->measuredAt())
    {
        return TargetTrackHandlingResult::conflicting;
    }

    latestTargetTrack_ = track;
    return TargetTrackHandlingResult::updated;
}

void InterceptorStateMachine::onInterceptionCommand(const domain::InterceptionCommand &command)
{
    static_cast<void>(startInterception(command));
}

InterceptionStartResult
InterceptorStateMachine::startInterception(const domain::InterceptionCommand &command)
{
    if (command.droneId() != droneId_)
    {
        return InterceptionStartResult::wrongDrone;
    }
    if (startedCommandId_ == command.commandId())
    {
        return InterceptionStartResult::duplicate;
    }
    if (!state_.has_value() || state_->status() != domain::DroneStatus::assigned)
    {
        return InterceptionStartResult::notAssigned;
    }
    if (state_->assignedTargetId() != command.targetId())
    {
        return InterceptionStartResult::targetMismatch;
    }

    const auto position = positioning_.currentPosition();
    state_.emplace(droneId_, position.position, position.measuredAt,
                   domain::DroneStatus::intercepting, command.targetId());
    stateOutput_.publish(*state_);
    startedCommandId_ = command.commandId();
    return InterceptionStartResult::started;
}

InterceptionTickResult InterceptorStateMachine::tick(const domain::Timestamp::Duration timeStep)
{
    if (!state_.has_value() || state_->status() != domain::DroneStatus::intercepting)
    {
        return InterceptionTickResult::notIntercepting;
    }
    if (!latestTargetTrack_.has_value())
    {
        return InterceptionTickResult::awaitingTarget;
    }

    flightControl_.moveToward(latestTargetTrack_->position(), timeStep);
    const auto position = positioning_.currentPosition();
    const auto &targetPosition = latestTargetTrack_->position();
    const auto distanceToTarget =
        std::hypot(position.position.xMeters() - targetPosition.xMeters(),
                   position.position.yMeters() - targetPosition.yMeters(),
                   position.position.zMeters() - targetPosition.zMeters());
    if (distanceToTarget <= arrivalToleranceMeters_)
    {
        const auto effectResult = effect_.trigger();
        const auto outcomeStatus = effectResult == InterceptionEffectResult::succeeded
                                       ? domain::DroneStatus::interceptionSucceeded
                                       : domain::DroneStatus::interceptionFailed;
        state_.emplace(droneId_, position.position, position.measuredAt, outcomeStatus,
                       state_->assignedTargetId());
        stateOutput_.publish(*state_);
        return effectResult == InterceptionEffectResult::succeeded
                   ? InterceptionTickResult::effectSucceeded
                   : InterceptionTickResult::effectFailed;
    }

    state_.emplace(droneId_, position.position, position.measuredAt,
                   domain::DroneStatus::intercepting, state_->assignedTargetId());
    stateOutput_.publish(*state_);
    return InterceptionTickResult::moved;
}

const std::optional<domain::DroneState> &InterceptorStateMachine::state() const noexcept
{
    return state_;
}

const std::optional<domain::TargetTrack> &
InterceptorStateMachine::latestTargetTrack() const noexcept
{
    return latestTargetTrack_;
}

} // namespace drone::interceptor
