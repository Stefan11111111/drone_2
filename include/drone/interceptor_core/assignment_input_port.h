#ifndef DRONE_INTERCEPTOR_CORE_ASSIGNMENT_INPUT_PORT_H
#define DRONE_INTERCEPTOR_CORE_ASSIGNMENT_INPUT_PORT_H

#include "drone/domain/assignment.h"

namespace drone::interceptor
{

class AssignmentInputPort
{
  public:
    virtual ~AssignmentInputPort() = default;

    virtual void onAssignment(const domain::Assignment &assignment) = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_ASSIGNMENT_INPUT_PORT_H
