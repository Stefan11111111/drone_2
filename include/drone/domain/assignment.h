#ifndef DRONE_DOMAIN_ASSIGNMENT_H
#define DRONE_DOMAIN_ASSIGNMENT_H

#include "drone/domain/drone_id.h"
#include "drone/domain/target_id.h"

namespace drone::domain
{

class Assignment final
{
  public:
    Assignment(DroneId droneId, TargetId targetId);

    [[nodiscard]] const DroneId &droneId() const noexcept;
    [[nodiscard]] const TargetId &targetId() const noexcept;

    bool operator==(const Assignment &) const = default;

  private:
    DroneId droneId_;
    TargetId targetId_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_ASSIGNMENT_H
