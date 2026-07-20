#include "drone/simulated_radar_adapter/simulated_radar.h"

#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"
#include "drone/observer_core/detection.h"
#include "drone/observer_core/detection_input_port.h"

#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::Timestamp;
using drone::observer::Detection;
using drone::observer::DetectionInputPort;
using drone::simulated_radar::Scenario;
using drone::simulated_radar::SimulatedRadar;
using drone::simulated_radar::Velocity;
using namespace std::chrono_literals;

class CapturingDetectionInput final : public DetectionInputPort
{
  public:
    void onDetection(const Detection &detection) override
    {
        detections.push_back(detection);
    }

    std::vector<Detection> detections;
};

Scenario movingTargetScenario()
{
    return Scenario{
        .targetId = TargetId{42},
        .initialPosition = Position{10.0, 20.0, 30.0},
        .startsAt = Timestamp{1'000ms},
        .velocity =
            Velocity{.xMetersPerSecond = 2.0, .yMetersPerSecond = -4.0, .zMetersPerSecond = 1.0},
        .tickInterval = 500ms};
}

TEST(SimulatedRadar,
     GivenAMovingTargetScenario_WhenTickedSeveralTimes_ThenPositionsAndTimesAdvanceRepeatably)
{
    CapturingDetectionInput input;
    SimulatedRadar radar{input, movingTargetScenario()};

    radar.tick();
    radar.tick();
    radar.tick();

    ASSERT_EQ(input.detections.size(), 3U);
    EXPECT_EQ(input.detections[0],
              (Detection{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}}));
    EXPECT_EQ(input.detections[1],
              (Detection{TargetId{42}, Position{11.0, 18.0, 30.5}, Timestamp{1'500ms}}));
    EXPECT_EQ(input.detections[2],
              (Detection{TargetId{42}, Position{12.0, 16.0, 31.0}, Timestamp{2'000ms}}));
}

TEST(SimulatedRadar,
     GivenIdenticalScenarios_WhenTickedIndependently_ThenTheyProduceIdenticalDetections)
{
    CapturingDetectionInput firstInput;
    CapturingDetectionInput secondInput;
    SimulatedRadar firstRadar{firstInput, movingTargetScenario()};
    SimulatedRadar secondRadar{secondInput, movingTargetScenario()};

    firstRadar.tick();
    secondRadar.tick();
    firstRadar.tick();
    secondRadar.tick();

    EXPECT_EQ(firstInput.detections, secondInput.detections);
}

TEST(SimulatedRadar, GivenANonPositiveTickInterval_WhenConstructed_ThenItIsRejected)
{
    CapturingDetectionInput input;
    auto scenario = movingTargetScenario();
    scenario.tickInterval = 0ms;

    EXPECT_THROW((SimulatedRadar{input, scenario}), std::invalid_argument);
}

TEST(SimulatedRadar, GivenANonFiniteVelocity_WhenConstructed_ThenItIsRejected)
{
    CapturingDetectionInput input;
    auto scenario = movingTargetScenario();
    scenario.velocity.xMetersPerSecond = std::numeric_limits<double>::infinity();

    EXPECT_THROW((SimulatedRadar{input, scenario}), std::invalid_argument);
}

} // namespace
