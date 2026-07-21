#ifndef DRONE_CONSOLE_CORE_INTERCEPTION_COMMAND_USE_CASE_H
#define DRONE_CONSOLE_CORE_INTERCEPTION_COMMAND_USE_CASE_H

#include "drone/console_core/drone_projection.h"
#include "drone/console_core/interception_command_output_port.h"
#include "drone/domain/drone_id.h"

#include <cstdint>
#include <set>

namespace drone::console
{

enum class StartInterceptionResult : std::uint8_t
{
    started,
    unknownDrone,
    droneNotAssigned,
    duplicate,
};

class InterceptionCommandUseCase final
{
  public:
    InterceptionCommandUseCase(const DroneProjection &drones,
                               InterceptionCommandOutputPort &output) noexcept;

    [[nodiscard]] StartInterceptionResult start(domain::DroneId droneId);

  private:
    const DroneProjection &drones_;
    InterceptionCommandOutputPort &output_;
    std::set<domain::DroneId> pendingStarts_;
    std::uint64_t nextCommandId_{1};
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_INTERCEPTION_COMMAND_USE_CASE_H
