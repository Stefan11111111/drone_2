#ifndef DRONE_CONSOLE_CORE_OUTCOME_PROJECTION_H
#define DRONE_CONSOLE_CORE_OUTCOME_PROJECTION_H

#include "drone/console_core/drone_projection.h"
#include "drone/console_core/explosion_event_input_port.h"
#include "drone/console_core/target_projection.h"
#include "drone/domain/explosion_event.h"
#include "drone/domain/explosion_event_id.h"

#include <map>
#include <vector>

namespace drone::console
{

class OutcomeProjection final : public ExplosionEventInputPort
{
  public:
    OutcomeProjection(const TargetProjection &targets, const DroneProjection &drones) noexcept;

    OutcomeUpdateResult onExplosionEvent(const domain::ExplosionEvent &event) override;

    [[nodiscard]] std::vector<domain::ExplosionEvent> outcomes() const;

  private:
    const TargetProjection &targets_;
    const DroneProjection &drones_;
    std::map<domain::ExplosionEventId, domain::ExplosionEvent> outcomes_;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_OUTCOME_PROJECTION_H
