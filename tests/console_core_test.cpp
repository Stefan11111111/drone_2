#include "drone/console_core/drone_projection.h"
#include "drone/console_core/target_projection.h"

#include "drone/console_core/drone_state_input_port.h"
#include "drone/console_core/target_track_input_port.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
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

using drone::console::DroneProjection;
using drone::console::DroneStateInputPort;
using drone::console::DroneUpdateResult;
using drone::console::TargetProjection;
using drone::console::TargetTrackInputPort;
using drone::console::TargetUpdateResult;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

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

} // namespace
