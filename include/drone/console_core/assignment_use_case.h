#ifndef DRONE_CONSOLE_CORE_ASSIGNMENT_USE_CASE_H
#define DRONE_CONSOLE_CORE_ASSIGNMENT_USE_CASE_H

#include "drone/console_core/assignment_output_port.h"
#include "drone/console_core/drone_projection.h"
#include "drone/console_core/target_projection.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/target_id.h"

#include <cstdint>
#include <map>

namespace drone::console
{

enum class AssignmentResult : std::uint8_t
{
    assigned,
    unknownDrone,
    unknownTarget,
    unavailableDrone,
    duplicate,
};

class AssignmentUseCase final
{
  public:
    AssignmentUseCase(const TargetProjection &targets, const DroneProjection &drones,
                      AssignmentOutputPort &output) noexcept;

    [[nodiscard]] AssignmentResult assign(domain::DroneId droneId, domain::TargetId targetId);

  private:
    const TargetProjection &targets_;
    const DroneProjection &drones_;
    AssignmentOutputPort &output_;
    std::map<domain::DroneId, domain::TargetId> pendingAssignments_;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_ASSIGNMENT_USE_CASE_H
