#include "drone/simulated_radar_adapter/simulated_radar.h"

#include "drone/observer_core/detection.h"

#include <chrono>
#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>

namespace drone::simulated_radar
{
namespace
{

void requireFinite(const double value, const std::string_view parameterName)
{
    if (!std::isfinite(value))
    {
        throw std::invalid_argument{std::string{parameterName} + " must be finite"};
    }
}

void validateScenario(const Scenario &scenario)
{
    requireFinite(scenario.velocity.xMetersPerSecond, "x velocity");
    requireFinite(scenario.velocity.yMetersPerSecond, "y velocity");
    requireFinite(scenario.velocity.zMetersPerSecond, "z velocity");

    if (scenario.tickInterval <= domain::Timestamp::Duration::zero())
    {
        throw std::invalid_argument{"The radar tick interval must be positive"};
    }
}

} // namespace

SimulatedRadar::SimulatedRadar(observer::DetectionInputPort &detectionInput, Scenario scenario)
    : detectionInput_{detectionInput}, scenario_{scenario}
{
    validateScenario(scenario_);
}

void SimulatedRadar::tick()
{
    const auto elapsed = scenario_.tickInterval * tickIndex_;
    const auto elapsedSeconds = std::chrono::duration<double>{elapsed}.count();
    const auto position =
        domain::Position{scenario_.initialPosition.xMeters() +
                             (scenario_.velocity.xMetersPerSecond * elapsedSeconds),
                         scenario_.initialPosition.yMeters() +
                             (scenario_.velocity.yMetersPerSecond * elapsedSeconds),
                         scenario_.initialPosition.zMeters() +
                             (scenario_.velocity.zMetersPerSecond * elapsedSeconds)};
    const auto detectedAt = domain::Timestamp{scenario_.startsAt.timeSinceUnixEpoch() + elapsed};

    detectionInput_.onDetection(observer::Detection{
        .targetId = scenario_.targetId, .position = position, .detectedAt = detectedAt});
    ++tickIndex_;
}

} // namespace drone::simulated_radar
