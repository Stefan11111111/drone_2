#ifndef DRONE_INTERCEPTOR_CORE_INTERCEPTION_COMMAND_INPUT_PORT_H
#define DRONE_INTERCEPTOR_CORE_INTERCEPTION_COMMAND_INPUT_PORT_H

#include "drone/domain/interception_command.h"

namespace drone::interceptor
{

class InterceptionCommandInputPort
{
  public:
    virtual ~InterceptionCommandInputPort() = default;

    virtual void onInterceptionCommand(const domain::InterceptionCommand &command) = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_INTERCEPTION_COMMAND_INPUT_PORT_H
