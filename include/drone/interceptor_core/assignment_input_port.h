#ifndef DRONE_INTERCEPTOR_CORE_ASSIGNMENT_INPUT_PORT_H
#define DRONE_INTERCEPTOR_CORE_ASSIGNMENT_INPUT_PORT_H

#include "drone/domain/assignment.h"

#include <cstdint>

namespace drone::interceptor
{

enum class AssignmentHandlingResult : std::uint8_t
{
    applied,
    wrongDrone,
    duplicate,
    conflicting,
    notStarted,
};

class AssignmentInputPort
{
  public:
    virtual ~AssignmentInputPort() = default;

    [[nodiscard]] virtual AssignmentHandlingResult
    onAssignment(const domain::Assignment &assignment) = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_ASSIGNMENT_INPUT_PORT_H
