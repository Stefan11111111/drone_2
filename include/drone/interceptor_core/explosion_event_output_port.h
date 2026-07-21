#ifndef DRONE_INTERCEPTOR_CORE_EXPLOSION_EVENT_OUTPUT_PORT_H
#define DRONE_INTERCEPTOR_CORE_EXPLOSION_EVENT_OUTPUT_PORT_H

#include "drone/domain/explosion_event.h"

namespace drone::interceptor
{

class ExplosionEventOutputPort
{
  public:
    virtual ~ExplosionEventOutputPort() = default;

    virtual void publish(const domain::ExplosionEvent &event) = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_EXPLOSION_EVENT_OUTPUT_PORT_H
