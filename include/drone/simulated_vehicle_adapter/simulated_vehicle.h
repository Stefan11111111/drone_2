#ifndef DRONE_SIMULATED_VEHICLE_ADAPTER_SIMULATED_VEHICLE_H
#define DRONE_SIMULATED_VEHICLE_ADAPTER_SIMULATED_VEHICLE_H

#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/positioning_port.h"

namespace drone::simulated_vehicle
{

struct Configuration final
{
    domain::Position initialPosition;
    domain::Timestamp startsAt;
    double maximumSpeedMetersPerSecond;
};

class SimulatedVehicle final : public interceptor::PositioningPort,
                               public interceptor::FlightControlPort
{
  public:
    explicit SimulatedVehicle(Configuration configuration);

    [[nodiscard]] interceptor::PositionSample currentPosition() const override;
    void moveToward(const domain::Position &destination,
                    domain::Timestamp::Duration timeStep) override;
    void advanceTime(domain::Timestamp::Duration timeStep);

  private:
    domain::Position position_;
    domain::Timestamp measuredAt_;
    double maximumSpeedMetersPerSecond_;
};

} // namespace drone::simulated_vehicle

#endif // DRONE_SIMULATED_VEHICLE_ADAPTER_SIMULATED_VEHICLE_H
