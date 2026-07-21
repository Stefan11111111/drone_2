#ifndef DRONE_INTERCEPTOR_CORE_DRONE_STATE_OUTPUT_PORT_H
#define DRONE_INTERCEPTOR_CORE_DRONE_STATE_OUTPUT_PORT_H

#include "drone/domain/drone_state.h"

namespace drone::interceptor
{

class DroneStateOutputPort
{
  public:
    virtual ~DroneStateOutputPort() = default;

    virtual void publish(const domain::DroneState &state) = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_DRONE_STATE_OUTPUT_PORT_H
