#include "drone/simulated_vehicle_adapter/simulated_vehicle.h"

#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/positioning_port.h"

#include <chrono>
#include <cmath>
#include <stdexcept>

namespace drone::simulated_vehicle
{
namespace
{

void validateConfiguration(const Configuration &configuration)
{
    if (!std::isfinite(configuration.maximumSpeedMetersPerSecond) ||
        configuration.maximumSpeedMetersPerSecond <= 0.0)
    {
        throw std::invalid_argument{
            "The simulated vehicle maximum speed must be finite and positive"};
    }
}

void validateTimeStep(const domain::Timestamp::Duration timeStep)
{
    if (timeStep <= domain::Timestamp::Duration::zero())
    {
        throw std::invalid_argument{"The simulated vehicle time step must be positive"};
    }
}

[[nodiscard]] domain::Position nextPosition(const domain::Position &current,
                                            const domain::Position &destination,
                                            const double maximumDistance)
{
    const auto xDistance = destination.xMeters() - current.xMeters();
    const auto yDistance = destination.yMeters() - current.yMeters();
    const auto zDistance = destination.zMeters() - current.zMeters();
    const auto distance = std::hypot(xDistance, yDistance, zDistance);
    if (distance == 0.0 || distance <= maximumDistance)
    {
        return destination;
    }

    const auto movementFraction = maximumDistance / distance;
    return domain::Position{current.xMeters() + (xDistance * movementFraction),
                            current.yMeters() + (yDistance * movementFraction),
                            current.zMeters() + (zDistance * movementFraction)};
}

} // namespace

SimulatedVehicle::SimulatedVehicle(const Configuration configuration)
    : position_{configuration.initialPosition}, measuredAt_{configuration.startsAt},
      maximumSpeedMetersPerSecond_{configuration.maximumSpeedMetersPerSecond}
{
    validateConfiguration(configuration);
}

interceptor::PositionSample SimulatedVehicle::currentPosition() const
{
    return {.position = position_, .measuredAt = measuredAt_};
}

void SimulatedVehicle::moveToward(const domain::Position &destination,
                                  const domain::Timestamp::Duration timeStep)
{
    validateTimeStep(timeStep);
    const auto maximumDistance =
        maximumSpeedMetersPerSecond_ * std::chrono::duration<double>{timeStep}.count();
    position_ = nextPosition(position_, destination, maximumDistance);
    measuredAt_ = domain::Timestamp{measuredAt_.timeSinceUnixEpoch() + timeStep};
}

} // namespace drone::simulated_vehicle
