#ifndef DRONE_CONSOLE_CORE_EXPLOSION_EVENT_INPUT_PORT_H
#define DRONE_CONSOLE_CORE_EXPLOSION_EVENT_INPUT_PORT_H

#include "drone/domain/explosion_event.h"

#include <cstdint>

namespace drone::console
{

enum class OutcomeUpdateResult : std::uint8_t
{
    recorded,
    duplicate,
    conflicting,
    unrelated,
};

class ExplosionEventInputPort
{
  public:
    virtual ~ExplosionEventInputPort() = default;

    [[nodiscard]] virtual OutcomeUpdateResult
    onExplosionEvent(const domain::ExplosionEvent &event) = 0;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_EXPLOSION_EVENT_INPUT_PORT_H
