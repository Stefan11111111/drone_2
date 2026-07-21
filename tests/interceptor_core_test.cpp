#include "drone/interceptor_core/interceptor_state_machine.h"

#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
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

using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using drone::interceptor::AssignmentHandlingResult;
using drone::interceptor::DroneStateOutputPort;
using drone::interceptor::FlightControlPort;
using drone::interceptor::InterceptorStateMachine;
using drone::interceptor::PositioningPort;
using drone::interceptor::PositionSample;
using drone::interceptor::TargetTrackHandlingResult;
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

TEST(InterceptorCore, GivenAnAssignmentForAnotherDrone_WhenHandled_ThenStateDoesNotChange)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{DroneId{7}, positioning, flightControl, output};
    interceptor.start();

    EXPECT_EQ(interceptor.onAssignment(Assignment{DroneId{8}, TargetId{42}}),
              AssignmentHandlingResult::wrongDrone);
    EXPECT_EQ(interceptor.state()->status(), DroneStatus::available);
    EXPECT_EQ(output.publishedStates.size(), 1U);
}

TEST(InterceptorCore,
     GivenAValidAssignment_WhenHandled_ThenTargetIsRememberedAndAssignedStateIsPublished)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{DroneId{7}, positioning, flightControl, output};
    interceptor.start();
    positioning.sample = {.position = Position{13.0, -4.0, 121.0},
                          .measuredAt = Timestamp{2'100ms}};

    EXPECT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);

    const DroneState expected{DroneId{7}, positioning.sample.position,
                              positioning.sample.measuredAt, DroneStatus::assigned, TargetId{42}};
    EXPECT_EQ(interceptor.state(), std::optional{expected});
    ASSERT_EQ(output.publishedStates.size(), 2U);
    EXPECT_EQ(output.publishedStates.back(), expected);
    EXPECT_TRUE(flightControl.destinations.empty());
}

TEST(InterceptorCore, GivenTheCurrentAssignmentAgain_WhenHandled_ThenItIsIgnoredAsDuplicate)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{DroneId{7}, positioning, flightControl, output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);

    EXPECT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::duplicate);
    EXPECT_EQ(interceptor.state()->assignedTargetId(), TargetId{42});
    EXPECT_EQ(output.publishedStates.size(), 2U);
}

TEST(InterceptorCore,
     GivenAnExistingAssignment_WhenAnotherTargetIsAssigned_ThenItIsIgnoredAsConflicting)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{DroneId{7}, positioning, flightControl, output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);

    EXPECT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{43}}),
              AssignmentHandlingResult::conflicting);
    EXPECT_EQ(interceptor.state()->assignedTargetId(), TargetId{42});
    EXPECT_EQ(output.publishedStates.size(), 2U);
}

TEST(InterceptorCore,
     GivenUnrelatedAndStaleTargetTracks_WhenHandled_ThenOnlyNewestAssignedTrackIsStored)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{DroneId{7}, positioning, flightControl, output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);

    EXPECT_EQ(interceptor.onTargetTrack(
                  TargetTrack{TargetId{43}, Position{9.0, 9.0, 9.0}, Timestamp{3'000ms}}),
              TargetTrackHandlingResult::unrelated);
    EXPECT_FALSE(interceptor.latestTargetTrack().has_value());

    const TargetTrack current{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{3'000ms}};
    EXPECT_EQ(interceptor.onTargetTrack(current), TargetTrackHandlingResult::accepted);
    EXPECT_EQ(interceptor.onTargetTrack(current), TargetTrackHandlingResult::duplicate);
    EXPECT_EQ(interceptor.onTargetTrack(
                  TargetTrack{TargetId{42}, Position{1.0, 2.0, 3.0}, Timestamp{2'000ms}}),
              TargetTrackHandlingResult::stale);
    EXPECT_EQ(interceptor.onTargetTrack(
                  TargetTrack{TargetId{42}, Position{4.0, 5.0, 6.0}, Timestamp{3'000ms}}),
              TargetTrackHandlingResult::conflicting);
    EXPECT_EQ(interceptor.latestTargetTrack(), current);

    const TargetTrack newer{TargetId{42}, Position{11.0, 22.0, 33.0}, Timestamp{4'000ms}};
    EXPECT_EQ(interceptor.onTargetTrack(newer), TargetTrackHandlingResult::updated);
    EXPECT_EQ(interceptor.latestTargetTrack(), newer);
    EXPECT_TRUE(flightControl.destinations.empty());
}

} // namespace
