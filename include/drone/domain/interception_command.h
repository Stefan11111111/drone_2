#ifndef DRONE_DOMAIN_INTERCEPTION_COMMAND_H
#define DRONE_DOMAIN_INTERCEPTION_COMMAND_H

#include "drone/domain/drone_id.h"
#include "drone/domain/interception_command_id.h"
#include "drone/domain/target_id.h"

namespace drone::domain
{

class InterceptionCommand final
{
  public:
    InterceptionCommand(InterceptionCommandId commandId, DroneId droneId, TargetId targetId);

    [[nodiscard]] const InterceptionCommandId &commandId() const noexcept;
    [[nodiscard]] const DroneId &droneId() const noexcept;
    [[nodiscard]] const TargetId &targetId() const noexcept;

    bool operator==(const InterceptionCommand &) const = default;

  private:
    InterceptionCommandId commandId_;
    DroneId droneId_;
    TargetId targetId_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_INTERCEPTION_COMMAND_H
