#include "drone/interceptor_core/interceptor_state_machine.h"

#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/drone_state_output_port.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/positioning_port.h"

#include <chrono>
#include <optional>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::Position;
using drone::domain::Timestamp;
using drone::interceptor::DroneStateOutputPort;
using drone::interceptor::FlightControlPort;
using drone::interceptor::InterceptorStateMachine;
using drone::interceptor::PositioningPort;
using drone::interceptor::PositionSample;
using namespace std::chrono_literals;

class FixedPositioning final : public PositioningPort
{
  public:
    [[nodiscard]] PositionSample currentPosition() const override
    {
        return sample;
    }

    PositionSample sample{.position = Position{12.5, -4.25, 120.0},
                          .measuredAt = Timestamp{2'000ms}};
};

class CapturingFlightControl final : public FlightControlPort
{
  public:
    void moveToward(const Position &destination, const Timestamp::Duration timeStep) override
    {
        destinations.push_back(destination);
        timeSteps.push_back(timeStep);
    }

    std::vector<Position> destinations;
    std::vector<Timestamp::Duration> timeSteps;
};

class CapturingStateOutput final : public DroneStateOutputPort
{
  public:
    void publish(const DroneState &state) override
    {
        publishedStates.push_back(state);
    }

    std::vector<DroneState> publishedStates;
};

TEST(InterceptorCore, GivenAnUnstartedInterceptor_WhenStarted_ThenItReportsItsCurrentAvailableState)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{DroneId{7}, positioning, flightControl, output};
    EXPECT_FALSE(interceptor.state().has_value());

    interceptor.start();

    const DroneState expected{DroneId{7}, positioning.sample.position,
                              positioning.sample.measuredAt, DroneStatus::available, std::nullopt};
    EXPECT_EQ(interceptor.state(), std::optional{expected});
    EXPECT_EQ(output.publishedStates, (std::vector{expected}));
    EXPECT_TRUE(flightControl.destinations.empty());
}

TEST(InterceptorCore,
     GivenAStartedInterceptor_WhenStartupIsRequestedAgain_ThenDuplicateStateIsNotReported)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{DroneId{7}, positioning, flightControl, output};
    interceptor.start();

    EXPECT_THROW(interceptor.start(), std::logic_error);

    EXPECT_EQ(output.publishedStates.size(), 1U);
    EXPECT_TRUE(flightControl.destinations.empty());
}

} // namespace
