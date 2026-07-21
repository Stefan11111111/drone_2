#ifndef DRONE_CONSOLE_CORE_DRONE_STATE_INPUT_PORT_H
#define DRONE_CONSOLE_CORE_DRONE_STATE_INPUT_PORT_H

#include "drone/domain/drone_state.h"

#include <cstdint>

namespace drone::console
{

enum class DroneUpdateResult : std::uint8_t
{
    added,
    updated,
    duplicate,
    stale,
    conflicting,
};

class DroneStateInputPort
{
  public:
    virtual ~DroneStateInputPort() = default;

    [[nodiscard]] virtual DroneUpdateResult onDroneState(const domain::DroneState &state) = 0;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_DRONE_STATE_INPUT_PORT_H
