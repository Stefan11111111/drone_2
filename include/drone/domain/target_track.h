#ifndef DRONE_DOMAIN_TARGET_TRACK_H
#define DRONE_DOMAIN_TARGET_TRACK_H

#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

namespace drone::domain
{

class TargetTrack final
{
  public:
    TargetTrack(TargetId targetId, Position position, Timestamp measuredAt);

    [[nodiscard]] const TargetId &targetId() const noexcept;
    [[nodiscard]] const Position &position() const noexcept;
    [[nodiscard]] const Timestamp &measuredAt() const noexcept;

    bool operator==(const TargetTrack &) const = default;

  private:
    TargetId targetId_;
    Position position_;
    Timestamp measuredAt_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_TARGET_TRACK_H
