#ifndef DRONE_CONSOLE_CORE_DRONE_PROJECTION_H
#define DRONE_CONSOLE_CORE_DRONE_PROJECTION_H

#include "drone/console_core/drone_state_input_port.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"

#include <map>
#include <optional>
#include <vector>

namespace drone::console
{

class DroneProjection final : public DroneStateInputPort
{
  public:
    DroneUpdateResult onDroneState(const domain::DroneState &state) override;

    [[nodiscard]] std::optional<domain::DroneState> latestDrone(domain::DroneId droneId) const;
    [[nodiscard]] std::vector<domain::DroneState> droneStates() const;

  private:
    std::map<domain::DroneId, domain::DroneState> droneStates_;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_DRONE_PROJECTION_H
