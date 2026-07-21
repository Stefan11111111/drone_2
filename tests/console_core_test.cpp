#include "drone/console_core/assignment_use_case.h"
#include "drone/console_core/drone_projection.h"
#include "drone/console_core/interception_command_use_case.h"
#include "drone/console_core/outcome_projection.h"
#include "drone/console_core/target_projection.h"

#include "drone/console_core/assignment_output_port.h"
#include "drone/console_core/drone_state_input_port.h"
#include "drone/console_core/interception_command_output_port.h"
#include "drone/console_core/target_track_input_port.h"
#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/explosion_event.h"
#include "drone/domain/explosion_event_id.h"
#include "drone/domain/interception_command.h"
#include "drone/domain/interception_command_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <chrono>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::console::AssignmentOutputPort;
using drone::console::AssignmentResult;
using drone::console::AssignmentUseCase;
using drone::console::DroneProjection;
using drone::console::DroneStateInputPort;
using drone::console::DroneUpdateResult;
using drone::console::InterceptionCommandOutputPort;
using drone::console::InterceptionCommandUseCase;
using drone::console::OutcomeProjection;
using drone::console::OutcomeUpdateResult;
using drone::console::StartInterceptionResult;
using drone::console::TargetProjection;
using drone::console::TargetTrackInputPort;
using drone::console::TargetUpdateResult;
using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::ExplosionEvent;
using drone::domain::ExplosionEventId;
using drone::domain::InterceptionCommand;
using drone::domain::InterceptionCommandId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

class CapturingAssignmentOutput final : public AssignmentOutputPort
{
  public:
    void publish(const Assignment &assignment) override
    {
        assignments.push_back(assignment);
    }

    std::vector<Assignment> assignments;
};

class CapturingCommandOutput final : public InterceptionCommandOutputPort
{
  public:
    void publish(const InterceptionCommand &command) override
    {
        commands.push_back(command);
    }

    std::vector<InterceptionCommand> commands;
};

struct AssignmentFixture
{
    AssignmentFixture() : useCase{targets, drones, output}
    {
        targets.onTargetTrack(
            TargetTrack{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}});
        drones.onDroneState(DroneState{DroneId{7}, Position{0.0, 0.0, 0.0}, Timestamp{1'000ms},
                                       DroneStatus::available, std::nullopt});
    }

    TargetProjection targets;
    DroneProjection drones;
    CapturingAssignmentOutput output;
    AssignmentUseCase useCase;
};

TEST(ConsoleCore, GivenNewTargets_WhenAccepted_ThenTheirLatestStateIsProjectedByIdentifier)
{
    TargetProjection projection;
    TargetTrackInputPort &input = projection;
    const TargetTrack targetTwo{TargetId{2}, Position{20.0, 21.0, 22.0}, Timestamp{2'000ms}};
    const TargetTrack targetOne{TargetId{1}, Position{10.0, 11.0, 12.0}, Timestamp{1'000ms}};

    EXPECT_EQ(input.onTargetTrack(targetTwo), TargetUpdateResult::added);
    EXPECT_EQ(input.onTargetTrack(targetOne), TargetUpdateResult::added);

    EXPECT_EQ(projection.latestTarget(TargetId{1}), targetOne);
    EXPECT_EQ(projection.latestTarget(TargetId{2}), targetTwo);
    EXPECT_EQ(projection.latestTarget(TargetId{3}), std::nullopt);
    EXPECT_EQ(projection.targetTracks(), (std::vector{targetOne, targetTwo}));
}

TEST(ConsoleCore, GivenAnExistingTarget_WhenANewerUpdateArrives_ThenItReplacesTheProjectedState)
{
    TargetProjection projection;
    const TargetTrack initial{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}};
    const TargetTrack newer{TargetId{42}, Position{11.0, 22.0, 33.0}, Timestamp{1'100ms}};
    ASSERT_EQ(projection.onTargetTrack(initial), TargetUpdateResult::added);

    EXPECT_EQ(projection.onTargetTrack(newer), TargetUpdateResult::updated);

    EXPECT_EQ(projection.latestTarget(TargetId{42}), newer);
    EXPECT_EQ(projection.targetTracks(), (std::vector{newer}));
}

TEST(ConsoleCore, GivenTheCurrentTargetSample_WhenItArrivesAgain_ThenItIsIgnoredAsADuplicate)
{
    TargetProjection projection;
    const TargetTrack current{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}};
    ASSERT_EQ(projection.onTargetTrack(current), TargetUpdateResult::added);

    EXPECT_EQ(projection.onTargetTrack(current), TargetUpdateResult::duplicate);

    EXPECT_EQ(projection.latestTarget(TargetId{42}), current);
}

TEST(ConsoleCore, GivenANewerProjectedTarget_WhenAnOlderUpdateArrives_ThenItIsIgnoredAsStale)
{
    TargetProjection projection;
    const TargetTrack current{TargetId{42}, Position{11.0, 22.0, 33.0}, Timestamp{1'100ms}};
    const TargetTrack stale{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}};
    ASSERT_EQ(projection.onTargetTrack(current), TargetUpdateResult::added);

    EXPECT_EQ(projection.onTargetTrack(stale), TargetUpdateResult::stale);

    EXPECT_EQ(projection.latestTarget(TargetId{42}), current);
}

TEST(ConsoleCore,
     GivenAProjectedTarget_WhenADifferentUpdateHasTheSameTime_ThenItIsIgnoredAsConflicting)
{
    TargetProjection projection;
    const TargetTrack current{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}};
    const TargetTrack conflicting{TargetId{42}, Position{11.0, 22.0, 33.0}, Timestamp{1'000ms}};
    ASSERT_EQ(projection.onTargetTrack(current), TargetUpdateResult::added);

    EXPECT_EQ(projection.onTargetTrack(conflicting), TargetUpdateResult::conflicting);

    EXPECT_EQ(projection.latestTarget(TargetId{42}), current);
}

TEST(ConsoleCore, GivenNewDroneStates_WhenAccepted_ThenTheyAreProjectedByIdentifier)
{
    DroneProjection projection;
    DroneStateInputPort &input = projection;
    const DroneState droneTwo{DroneId{2}, Position{20.0, 21.0, 22.0}, Timestamp{2'000ms},
                              DroneStatus::available, std::nullopt};
    const DroneState droneOne{DroneId{1}, Position{10.0, 11.0, 12.0}, Timestamp{1'000ms},
                              DroneStatus::available, std::nullopt};

    EXPECT_EQ(input.onDroneState(droneTwo), DroneUpdateResult::added);
    EXPECT_EQ(input.onDroneState(droneOne), DroneUpdateResult::added);

    EXPECT_EQ(projection.latestDrone(DroneId{1}), droneOne);
    EXPECT_EQ(projection.latestDrone(DroneId{2}), droneTwo);
    EXPECT_EQ(projection.latestDrone(DroneId{3}), std::nullopt);
    EXPECT_EQ(projection.droneStates(), (std::vector{droneOne, droneTwo}));
}

TEST(ConsoleCore,
     GivenAProjectedDrone_WhenNewerDuplicateStaleAndConflictingStatesArrive_ThenOnlyNewerWins)
{
    DroneProjection projection;
    const DroneState initial{DroneId{1}, Position{0.0, 0.0, 0.0}, Timestamp{1'000ms},
                             DroneStatus::available, std::nullopt};
    const DroneState newer{DroneId{1}, Position{1.0, 2.0, 3.0}, Timestamp{2'000ms},
                           DroneStatus::available, std::nullopt};
    const DroneState stale{DroneId{1}, Position{-1.0, -2.0, -3.0}, Timestamp{1'500ms},
                           DroneStatus::available, std::nullopt};
    const DroneState conflicting{DroneId{1}, Position{9.0, 9.0, 9.0}, Timestamp{2'000ms},
                                 DroneStatus::available, std::nullopt};

    ASSERT_EQ(projection.onDroneState(initial), DroneUpdateResult::added);
    EXPECT_EQ(projection.onDroneState(initial), DroneUpdateResult::duplicate);
    EXPECT_EQ(projection.onDroneState(newer), DroneUpdateResult::updated);
    EXPECT_EQ(projection.onDroneState(stale), DroneUpdateResult::stale);
    EXPECT_EQ(projection.onDroneState(conflicting), DroneUpdateResult::conflicting);
    EXPECT_EQ(projection.latestDrone(DroneId{1}), newer);
}

TEST(ConsoleCore, GivenACorrelatedOutcome_WhenReceivedAgain_ThenItIsRecordedOnceByEventIdentity)
{
    TargetProjection targets;
    DroneProjection drones;
    ASSERT_EQ(targets.onTargetTrack(
                  TargetTrack{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}}),
              TargetUpdateResult::added);
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{7}, Position{10.0, 20.0, 30.0}, Timestamp{2'000ms},
                                       DroneStatus::interceptionSucceeded, TargetId{42}}),
        DroneUpdateResult::added);
    OutcomeProjection outcomes{targets, drones};
    const ExplosionEvent event{ExplosionEventId{101}, DroneId{7}, TargetId{42},
                               Position{10.0, 20.0, 30.0}, Timestamp{2'000ms}};

    EXPECT_EQ(outcomes.onExplosionEvent(event), OutcomeUpdateResult::recorded);
    EXPECT_EQ(outcomes.onExplosionEvent(event), OutcomeUpdateResult::duplicate);
    EXPECT_EQ(
        outcomes.onExplosionEvent(ExplosionEvent{ExplosionEventId{101}, DroneId{7}, TargetId{42},
                                                 Position{11.0, 20.0, 30.0}, Timestamp{2'000ms}}),
        OutcomeUpdateResult::conflicting);
    EXPECT_EQ(outcomes.outcomes(), (std::vector{event}));
}

TEST(ConsoleCore, GivenAnOutcomeWithoutMatchingProjectedSubjects_WhenReceived_ThenItIsUnrelated)
{
    TargetProjection targets;
    DroneProjection drones;
    ASSERT_EQ(targets.onTargetTrack(
                  TargetTrack{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}}),
              TargetUpdateResult::added);
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{7}, Position{10.0, 20.0, 30.0}, Timestamp{2'000ms},
                                       DroneStatus::assigned, TargetId{42}}),
        DroneUpdateResult::added);
    OutcomeProjection outcomes{targets, drones};

    EXPECT_EQ(
        outcomes.onExplosionEvent(ExplosionEvent{ExplosionEventId{101}, DroneId{8}, TargetId{42},
                                                 Position{10.0, 20.0, 30.0}, Timestamp{2'000ms}}),
        OutcomeUpdateResult::unrelated);
    EXPECT_EQ(
        outcomes.onExplosionEvent(ExplosionEvent{ExplosionEventId{102}, DroneId{7}, TargetId{43},
                                                 Position{10.0, 20.0, 30.0}, Timestamp{2'000ms}}),
        OutcomeUpdateResult::unrelated);
    EXPECT_TRUE(outcomes.outcomes().empty());
}

TEST(ConsoleCore, GivenAnUnknownDrone_WhenAssignmentIsRequested_ThenNothingIsEmitted)
{
    AssignmentFixture fixture;

    EXPECT_EQ(fixture.useCase.assign(DroneId{8}, TargetId{42}), AssignmentResult::unknownDrone);
    EXPECT_TRUE(fixture.output.assignments.empty());
}

TEST(ConsoleCore, GivenAnUnknownTarget_WhenAssignmentIsRequested_ThenNothingIsEmitted)
{
    AssignmentFixture fixture;

    EXPECT_EQ(fixture.useCase.assign(DroneId{7}, TargetId{43}), AssignmentResult::unknownTarget);
    EXPECT_TRUE(fixture.output.assignments.empty());
}

TEST(ConsoleCore, GivenAnUnavailableDrone_WhenAssignmentIsRequested_ThenNothingIsEmitted)
{
    AssignmentFixture fixture;
    ASSERT_EQ(fixture.drones.onDroneState(DroneState{DroneId{7}, Position{0.0, 0.0, 0.0},
                                                     Timestamp{2'000ms}, DroneStatus::assigned,
                                                     TargetId{42}}),
              DroneUpdateResult::updated);

    EXPECT_EQ(fixture.useCase.assign(DroneId{7}, TargetId{42}), AssignmentResult::unavailableDrone);
    EXPECT_TRUE(fixture.output.assignments.empty());
}

TEST(ConsoleCore, GivenARepeatedValidSelection_WhenAssignmentIsRequested_ThenItIsEmittedOnce)
{
    AssignmentFixture fixture;
    ASSERT_EQ(fixture.useCase.assign(DroneId{7}, TargetId{42}), AssignmentResult::assigned);

    EXPECT_EQ(fixture.useCase.assign(DroneId{7}, TargetId{42}), AssignmentResult::duplicate);
    EXPECT_EQ(fixture.output.assignments, (std::vector{Assignment{DroneId{7}, TargetId{42}}}));
}

TEST(ConsoleCore, GivenAnAvailableDroneAndKnownTarget_WhenAssigned_ThenOneAssignmentIsEmitted)
{
    AssignmentFixture fixture;

    EXPECT_EQ(fixture.useCase.assign(DroneId{7}, TargetId{42}), AssignmentResult::assigned);
    EXPECT_EQ(fixture.output.assignments, (std::vector{Assignment{DroneId{7}, TargetId{42}}}));
}

TEST(ConsoleCore, GivenAnUnknownDrone_WhenStartIsRequested_ThenNoCommandIsEmitted)
{
    DroneProjection drones;
    CapturingCommandOutput output;
    InterceptionCommandUseCase useCase{drones, output};

    EXPECT_EQ(useCase.start(DroneId{7}), StartInterceptionResult::unknownDrone);
    EXPECT_TRUE(output.commands.empty());
}

TEST(ConsoleCore, GivenAnAvailableDrone_WhenStartIsRequested_ThenNoCommandIsEmitted)
{
    DroneProjection drones;
    CapturingCommandOutput output;
    InterceptionCommandUseCase useCase{drones, output};
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{7}, Position{0.0, 0.0, 0.0}, Timestamp{1'000ms},
                                       DroneStatus::available, std::nullopt}),
        DroneUpdateResult::added);

    EXPECT_EQ(useCase.start(DroneId{7}), StartInterceptionResult::droneNotAssigned);
    EXPECT_TRUE(output.commands.empty());
}

TEST(ConsoleCore, GivenAnAssignedDrone_WhenStartIsRequested_ThenOneCorrelatedCommandIsEmitted)
{
    DroneProjection drones;
    CapturingCommandOutput output;
    InterceptionCommandUseCase useCase{drones, output};
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{7}, Position{0.0, 0.0, 0.0}, Timestamp{1'000ms},
                                       DroneStatus::assigned, TargetId{42}}),
        DroneUpdateResult::added);

    EXPECT_EQ(useCase.start(DroneId{7}), StartInterceptionResult::started);
    EXPECT_EQ(output.commands, (std::vector{InterceptionCommand{InterceptionCommandId{1},
                                                                DroneId{7}, TargetId{42}}}));
}

TEST(ConsoleCore, GivenAStartedDrone_WhenStartIsRequestedAgain_ThenNoSecondCommandIsEmitted)
{
    DroneProjection drones;
    CapturingCommandOutput output;
    InterceptionCommandUseCase useCase{drones, output};
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{7}, Position{0.0, 0.0, 0.0}, Timestamp{1'000ms},
                                       DroneStatus::assigned, TargetId{42}}),
        DroneUpdateResult::added);
    ASSERT_EQ(useCase.start(DroneId{7}), StartInterceptionResult::started);

    EXPECT_EQ(useCase.start(DroneId{7}), StartInterceptionResult::duplicate);
    EXPECT_EQ(output.commands.size(), 1U);
}

} // namespace
