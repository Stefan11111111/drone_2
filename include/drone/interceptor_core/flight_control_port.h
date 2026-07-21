#ifndef DRONE_INTERCEPTOR_CORE_FLIGHT_CONTROL_PORT_H
#define DRONE_INTERCEPTOR_CORE_FLIGHT_CONTROL_PORT_H

#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"

namespace drone::interceptor
{

class FlightControlPort
{
  public:
    virtual ~FlightControlPort() = default;

    virtual void moveToward(const domain::Position &destination,
                            domain::Timestamp::Duration timeStep) = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_FLIGHT_CONTROL_PORT_H
