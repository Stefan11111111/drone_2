#ifndef DRONE_INTERCEPTOR_CORE_INTERCEPTOR_STATE_MACHINE_H
#define DRONE_INTERCEPTOR_CORE_INTERCEPTOR_STATE_MACHINE_H

#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/interceptor_core/assignment_input_port.h"
#include "drone/interceptor_core/drone_state_output_port.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/positioning_port.h"

#include <optional>

namespace drone::interceptor
{

class InterceptorStateMachine final : public AssignmentInputPort
{
  public:
    InterceptorStateMachine(domain::DroneId droneId, PositioningPort &positioning,
                            FlightControlPort &flightControl, DroneStateOutputPort &stateOutput);

    void start();
    [[nodiscard]] AssignmentHandlingResult
    onAssignment(const domain::Assignment &assignment) override;

    [[nodiscard]] const std::optional<domain::DroneState> &state() const noexcept;

  private:
    domain::DroneId droneId_;
    PositioningPort &positioning_;
    FlightControlPort &flightControl_;
    DroneStateOutputPort &stateOutput_;
    std::optional<domain::DroneState> state_;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_INTERCEPTOR_STATE_MACHINE_H
