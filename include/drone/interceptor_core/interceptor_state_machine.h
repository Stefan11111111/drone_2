#ifndef DRONE_INTERCEPTOR_CORE_INTERCEPTOR_STATE_MACHINE_H
#define DRONE_INTERCEPTOR_CORE_INTERCEPTOR_STATE_MACHINE_H

#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/interception_command_id.h"
#include "drone/interceptor_core/assignment_input_port.h"
#include "drone/interceptor_core/drone_state_output_port.h"
#include "drone/interceptor_core/explosion_event_output_port.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/interception_command_input_port.h"
#include "drone/interceptor_core/interception_effect_port.h"
#include "drone/interceptor_core/positioning_port.h"
#include "drone/interceptor_core/target_track_input_port.h"

#include <cstdint>
#include <optional>

namespace drone::interceptor
{

enum class InterceptionStartResult : std::uint8_t
{
    started,
    wrongDrone,
    targetMismatch,
    notAssigned,
    duplicate,
};

enum class InterceptionTickResult : std::uint8_t
{
    moved,
    effectSucceeded,
    effectFailed,
    notIntercepting,
    awaitingTarget,
};

struct InterceptorConfiguration final
{
    domain::DroneId droneId;
    double arrivalToleranceMeters;
};

class InterceptorStateMachine final : public AssignmentInputPort,
                                      public InterceptionCommandInputPort,
                                      public TargetTrackInputPort
{
  public:
    InterceptorStateMachine(InterceptorConfiguration configuration, PositioningPort &positioning,
                            FlightControlPort &flightControl, InterceptionEffectPort &effect,
                            DroneStateOutputPort &stateOutput,
                            ExplosionEventOutputPort &eventOutput);

    void start();
    [[nodiscard]] AssignmentHandlingResult
    onAssignment(const domain::Assignment &assignment) override;
    [[nodiscard]] TargetTrackHandlingResult
    onTargetTrack(const domain::TargetTrack &track) override;
    void onInterceptionCommand(const domain::InterceptionCommand &command) override;

    [[nodiscard]] InterceptionStartResult
    startInterception(const domain::InterceptionCommand &command);
    [[nodiscard]] InterceptionTickResult tick(domain::Timestamp::Duration timeStep);

    [[nodiscard]] const std::optional<domain::DroneState> &state() const noexcept;
    [[nodiscard]] const std::optional<domain::TargetTrack> &latestTargetTrack() const noexcept;

  private:
    domain::DroneId droneId_;
    PositioningPort &positioning_;
    FlightControlPort &flightControl_;
    InterceptionEffectPort &effect_;
    DroneStateOutputPort &stateOutput_;
    ExplosionEventOutputPort &eventOutput_;
    double arrivalToleranceMeters_;
    std::optional<domain::DroneState> state_;
    std::optional<domain::TargetTrack> latestTargetTrack_;
    std::optional<domain::InterceptionCommandId> startedCommandId_;
    std::optional<domain::ExplosionEventId> pendingEventId_;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_INTERCEPTOR_STATE_MACHINE_H
