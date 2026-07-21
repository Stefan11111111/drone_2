#include "drone/interceptor_core/interceptor_state_machine.h"

#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/interception_command.h"
#include "drone/domain/interception_command_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/drone_state_output_port.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/interception_effect_port.h"
#include "drone/interceptor_core/positioning_port.h"

#include <chrono>
#include <cstddef>
#include <limits>
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
using drone::domain::InterceptionCommand;
using drone::domain::InterceptionCommandId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using drone::interceptor::AssignmentHandlingResult;
using drone::interceptor::DroneStateOutputPort;
using drone::interceptor::FlightControlPort;
using drone::interceptor::InterceptionEffectPort;
using drone::interceptor::InterceptionEffectResult;
using drone::interceptor::InterceptionStartResult;
using drone::interceptor::InterceptionTickResult;
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

class CapturingFlightControl final : public FlightControlPort, public InterceptionEffectPort
{
  public:
    void moveToward(const Position &destination, const Timestamp::Duration timeStep) override
    {
        destinations.push_back(destination);
        timeSteps.push_back(timeStep);
    }

    [[nodiscard]] InterceptionEffectResult trigger() override
    {
        ++effectTriggerCount;
        return effectResult;
    }

    std::vector<Position> destinations;
    std::vector<Timestamp::Duration> timeSteps;
    InterceptionEffectResult effectResult{InterceptionEffectResult::succeeded};
    std::size_t effectTriggerCount{};
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
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
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
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
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
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();

    EXPECT_EQ(interceptor.onAssignment(Assignment{DroneId{8}, TargetId{42}}),
              AssignmentHandlingResult::wrongDrone);
    EXPECT_TRUE(interceptor.state().has_value() &&
                interceptor.state()->status() == DroneStatus::available);
    EXPECT_EQ(output.publishedStates.size(), 1U);
}

TEST(InterceptorCore,
     GivenAValidAssignment_WhenHandled_ThenTargetIsRememberedAndAssignedStateIsPublished)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
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
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);

    EXPECT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::duplicate);
    EXPECT_TRUE(interceptor.state().has_value() &&
                interceptor.state()->assignedTargetId() == TargetId{42});
    EXPECT_EQ(output.publishedStates.size(), 2U);
}

TEST(InterceptorCore,
     GivenAnExistingAssignment_WhenAnotherTargetIsAssigned_ThenItIsIgnoredAsConflicting)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);

    EXPECT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{43}}),
              AssignmentHandlingResult::conflicting);
    EXPECT_TRUE(interceptor.state().has_value() &&
                interceptor.state()->assignedTargetId() == TargetId{42});
    EXPECT_EQ(output.publishedStates.size(), 2U);
}

TEST(InterceptorCore,
     GivenUnrelatedAndStaleTargetTracks_WhenHandled_ThenOnlyNewestAssignedTrackIsStored)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
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

TEST(InterceptorCore, GivenInvalidStartCommands_WhenHandled_ThenInterceptorRemainsAssigned)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    const InterceptionCommand correct{InterceptionCommandId{101}, DroneId{7}, TargetId{42}};

    EXPECT_EQ(interceptor.startInterception(correct), InterceptionStartResult::notAssigned);
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);

    EXPECT_EQ(interceptor.startInterception(
                  InterceptionCommand{InterceptionCommandId{101}, DroneId{8}, TargetId{42}}),
              InterceptionStartResult::wrongDrone);
    EXPECT_EQ(interceptor.startInterception(
                  InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{43}}),
              InterceptionStartResult::targetMismatch);
    EXPECT_TRUE(interceptor.state().has_value() &&
                interceptor.state()->status() == DroneStatus::assigned);
    EXPECT_EQ(output.publishedStates.size(), 2U);
}

TEST(InterceptorCore,
     GivenAValidStartBeforeTargetData_WhenHandled_ThenItInterceptsAndWaitsForATarget)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);
    positioning.sample.measuredAt = Timestamp{2'100ms};
    const InterceptionCommand command{InterceptionCommandId{101}, DroneId{7}, TargetId{42}};

    EXPECT_EQ(interceptor.startInterception(command), InterceptionStartResult::started);
    EXPECT_EQ(interceptor.startInterception(command), InterceptionStartResult::duplicate);
    EXPECT_TRUE(interceptor.state().has_value() &&
                interceptor.state()->status() == DroneStatus::intercepting);
    EXPECT_EQ(interceptor.tick(100ms), InterceptionTickResult::awaitingTarget);
    EXPECT_TRUE(flightControl.destinations.empty());
    EXPECT_EQ(output.publishedStates.size(), 3U);
}

TEST(InterceptorCore, GivenAnInterceptingDrone_WhenTicked_ThenItMovesAndReportsCurrentState)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);
    const TargetTrack target{TargetId{42}, Position{50.0, 60.0, 70.0}, Timestamp{3'000ms}};
    ASSERT_EQ(interceptor.onTargetTrack(target), TargetTrackHandlingResult::accepted);
    ASSERT_EQ(interceptor.startInterception(
                  InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{42}}),
              InterceptionStartResult::started);
    positioning.sample = {.position = Position{14.0, -3.0, 122.0},
                          .measuredAt = Timestamp{2'200ms}};

    EXPECT_EQ(interceptor.tick(100ms), InterceptionTickResult::moved);

    EXPECT_EQ(flightControl.destinations, (std::vector{target.position()}));
    EXPECT_EQ(flightControl.timeSteps, (std::vector{100ms}));
    const DroneState expected{DroneId{7}, positioning.sample.position,
                              positioning.sample.measuredAt, DroneStatus::intercepting,
                              TargetId{42}};
    EXPECT_EQ(interceptor.state(), std::optional{expected});
    EXPECT_EQ(output.publishedStates.back(), expected);
    EXPECT_EQ(flightControl.effectTriggerCount, 0U);
}

TEST(InterceptorCore, GivenANewerTargetDuringPursuit_WhenTickedAgain_ThenItChangesDestination)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);
    const TargetTrack initial{TargetId{42}, Position{50.0, 0.0, 0.0}, Timestamp{3'000ms}};
    ASSERT_EQ(interceptor.onTargetTrack(initial), TargetTrackHandlingResult::accepted);
    ASSERT_EQ(interceptor.startInterception(
                  InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{42}}),
              InterceptionStartResult::started);
    positioning.sample.measuredAt = Timestamp{2'100ms};
    ASSERT_EQ(interceptor.tick(100ms), InterceptionTickResult::moved);
    const TargetTrack retargeted{TargetId{42}, Position{0.0, 50.0, 0.0}, Timestamp{4'000ms}};
    ASSERT_EQ(interceptor.onTargetTrack(retargeted), TargetTrackHandlingResult::updated);
    positioning.sample.measuredAt = Timestamp{2'200ms};

    EXPECT_EQ(interceptor.tick(100ms), InterceptionTickResult::moved);

    EXPECT_EQ(flightControl.destinations, (std::vector{initial.position(), retargeted.position()}));
    EXPECT_EQ(output.publishedStates.back().reportedAt(), Timestamp{2'200ms});
    EXPECT_EQ(flightControl.effectTriggerCount, 0U);
}

TEST(InterceptorCore,
     GivenAnInterceptingDroneWithinArrivalTolerance_WhenTickedRepeatedly_ThenEffectTriggersOnce)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);
    const TargetTrack target{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{3'000ms}};
    ASSERT_EQ(interceptor.onTargetTrack(target), TargetTrackHandlingResult::accepted);
    ASSERT_EQ(interceptor.startInterception(
                  InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{42}}),
              InterceptionStartResult::started);
    positioning.sample = {.position = Position{10.1, 20.1, 30.1}, .measuredAt = Timestamp{2'200ms}};

    EXPECT_EQ(interceptor.tick(100ms), InterceptionTickResult::effectSucceeded);

    const DroneState succeeded{DroneId{7}, positioning.sample.position,
                               positioning.sample.measuredAt, DroneStatus::interceptionSucceeded,
                               TargetId{42}};
    EXPECT_EQ(interceptor.state(), std::optional{succeeded});
    EXPECT_EQ(flightControl.effectTriggerCount, 1U);
    EXPECT_EQ(interceptor.tick(100ms), InterceptionTickResult::notIntercepting);
    EXPECT_EQ(flightControl.effectTriggerCount, 1U);
}

TEST(InterceptorCore, GivenTheInterceptionEffectFails_WhenArrivalIsDetected_ThenFailureIsFinal)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    flightControl.effectResult = InterceptionEffectResult::failed;
    CapturingStateOutput output;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        output};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              AssignmentHandlingResult::applied);
    const TargetTrack target{TargetId{42}, positioning.sample.position, Timestamp{3'000ms}};
    ASSERT_EQ(interceptor.onTargetTrack(target), TargetTrackHandlingResult::accepted);
    ASSERT_EQ(interceptor.startInterception(
                  InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{42}}),
              InterceptionStartResult::started);

    EXPECT_EQ(interceptor.tick(100ms), InterceptionTickResult::effectFailed);

    const DroneState failed{DroneId{7}, positioning.sample.position, positioning.sample.measuredAt,
                            DroneStatus::interceptionFailed, TargetId{42}};
    EXPECT_EQ(interceptor.state(), std::optional{failed});
    EXPECT_EQ(flightControl.effectTriggerCount, 1U);
    EXPECT_EQ(interceptor.tick(100ms), InterceptionTickResult::notIntercepting);
    EXPECT_EQ(flightControl.effectTriggerCount, 1U);
}

TEST(InterceptorCore, GivenAnInvalidArrivalTolerance_WhenConstructed_ThenItIsRejected)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    CapturingStateOutput output;

    EXPECT_THROW((InterceptorStateMachine{{.droneId = DroneId{7}, .arrivalToleranceMeters = -0.01},
                                          positioning,
                                          flightControl,
                                          flightControl,
                                          output}),
                 std::invalid_argument);
    EXPECT_THROW((InterceptorStateMachine{
                     {.droneId = DroneId{7},
                      .arrivalToleranceMeters = std::numeric_limits<double>::infinity()},
                     positioning,
                     flightControl,
                     flightControl,
                     output}),
                 std::invalid_argument);
}

} // namespace
