#ifndef DRONE_SIMULATED_RADAR_ADAPTER_SIMULATED_RADAR_H
#define DRONE_SIMULATED_RADAR_ADAPTER_SIMULATED_RADAR_H

#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"
#include "drone/observer_core/detection_input_port.h"

#include <cstdint>

namespace drone::simulated_radar
{

struct Velocity final
{
    double xMetersPerSecond;
    double yMetersPerSecond;
    double zMetersPerSecond;

    bool operator==(const Velocity &) const = default;
};

struct Scenario final
{
    domain::TargetId targetId;
    domain::Position initialPosition;
    domain::Timestamp startsAt;
    Velocity velocity;
    domain::Timestamp::Duration tickInterval;
};

class SimulatedRadar final
{
  public:
    SimulatedRadar(observer::DetectionInputPort &detectionInput, Scenario scenario);

    void tick();

  private:
    observer::DetectionInputPort &detectionInput_;
    Scenario scenario_;
    std::uint64_t tickIndex_{0};
};

} // namespace drone::simulated_radar

#endif // DRONE_SIMULATED_RADAR_ADAPTER_SIMULATED_RADAR_H
