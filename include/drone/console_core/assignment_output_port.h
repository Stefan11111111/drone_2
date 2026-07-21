#ifndef DRONE_CONSOLE_CORE_ASSIGNMENT_OUTPUT_PORT_H
#define DRONE_CONSOLE_CORE_ASSIGNMENT_OUTPUT_PORT_H

#include "drone/domain/assignment.h"

namespace drone::console
{

class AssignmentOutputPort
{
  public:
    virtual ~AssignmentOutputPort() = default;

    virtual void publish(const domain::Assignment &assignment) = 0;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_ASSIGNMENT_OUTPUT_PORT_H
