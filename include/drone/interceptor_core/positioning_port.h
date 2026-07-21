#ifndef DRONE_INTERCEPTOR_CORE_POSITIONING_PORT_H
#define DRONE_INTERCEPTOR_CORE_POSITIONING_PORT_H

#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"

namespace drone::interceptor
{

struct PositionSample final
{
    domain::Position position;
    domain::Timestamp measuredAt;

    bool operator==(const PositionSample &) const = default;
};

class PositioningPort
{
  public:
    virtual ~PositioningPort() = default;

    [[nodiscard]] virtual PositionSample currentPosition() const = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_POSITIONING_PORT_H
