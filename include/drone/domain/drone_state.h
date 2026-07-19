#ifndef DRONE_DOMAIN_DRONE_STATE_H
#define DRONE_DOMAIN_DRONE_STATE_H

#include "drone/domain/drone_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

#include <cstdint>
#include <optional>

namespace drone::domain
{

enum class DroneStatus : std::uint8_t
{
    available,
    assigned,
    intercepting,
    interceptionSucceeded,
    interceptionFailed,
};

class DroneState final
{
  public:
    DroneState(DroneId droneId, Position position, Timestamp reportedAt, DroneStatus status,
               std::optional<TargetId> assignedTargetId);

    [[nodiscard]] const DroneId &droneId() const noexcept;
    [[nodiscard]] const Position &position() const noexcept;
    [[nodiscard]] const Timestamp &reportedAt() const noexcept;
    [[nodiscard]] DroneStatus status() const noexcept;
    [[nodiscard]] const std::optional<TargetId> &assignedTargetId() const noexcept;

    bool operator==(const DroneState &) const = default;

  private:
    DroneId droneId_;
    Position position_;
    Timestamp reportedAt_;
    DroneStatus status_;
    std::optional<TargetId> assignedTargetId_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_DRONE_STATE_H
