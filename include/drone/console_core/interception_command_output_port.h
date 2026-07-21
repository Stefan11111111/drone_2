#ifndef DRONE_CONSOLE_CORE_INTERCEPTION_COMMAND_OUTPUT_PORT_H
#define DRONE_CONSOLE_CORE_INTERCEPTION_COMMAND_OUTPUT_PORT_H

#include "drone/domain/interception_command.h"

namespace drone::console
{

class InterceptionCommandOutputPort
{
  public:
    virtual ~InterceptionCommandOutputPort() = default;

    virtual void publish(const domain::InterceptionCommand &command) = 0;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_INTERCEPTION_COMMAND_OUTPUT_PORT_H
