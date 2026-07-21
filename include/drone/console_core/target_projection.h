#ifndef DRONE_CONSOLE_CORE_TARGET_PROJECTION_H
#define DRONE_CONSOLE_CORE_TARGET_PROJECTION_H

#include "drone/console_core/target_track_input_port.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"

#include <map>
#include <optional>
#include <vector>

namespace drone::console
{

class TargetProjection final : public TargetTrackInputPort
{
  public:
    TargetUpdateResult onTargetTrack(const domain::TargetTrack &targetTrack) override;

    [[nodiscard]] std::optional<domain::TargetTrack> latestTarget(domain::TargetId targetId) const;
    [[nodiscard]] std::vector<domain::TargetTrack> targetTracks() const;

  private:
    std::map<domain::TargetId, domain::TargetTrack> targetTracks_;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_TARGET_PROJECTION_H
