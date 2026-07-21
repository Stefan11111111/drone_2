#ifndef DRONE_INTERCEPTOR_CORE_INTERCEPTION_EFFECT_PORT_H
#define DRONE_INTERCEPTOR_CORE_INTERCEPTION_EFFECT_PORT_H

#include <cstdint>

namespace drone::interceptor
{

enum class InterceptionEffectResult : std::uint8_t
{
    succeeded,
    failed,
};

class InterceptionEffectPort
{
  public:
    virtual ~InterceptionEffectPort() = default;

    [[nodiscard]] virtual InterceptionEffectResult trigger() = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_INTERCEPTION_EFFECT_PORT_H
