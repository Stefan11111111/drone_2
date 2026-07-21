#include "drone/console_ui_adapter/terminal_view.h"

#include "drone/console_core/drone_projection.h"
#include "drone/console_core/target_projection.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <chrono>
#include <optional>
#include <sstream>

#include <gtest/gtest.h>

namespace
{

using drone::console::DroneProjection;
using drone::console::TargetProjection;
using drone::console_ui::TerminalView;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

TEST(TerminalView, GivenNoLiveTargets_WhenRendered_ThenTheEmptySnapshotIsExplicit)
{
    TargetProjection projection;
    DroneProjection drones;
    std::ostringstream output;
    TerminalView view{output};

    view.render(projection, drones);

    EXPECT_EQ(output.str(), "Targets (0)\n"
                            "ID | X (m) | Y (m) | Z (m) | Measured (ms)\n"
                            "<none>\n\n"
                            "Drones (0)\n"
                            "ID | X (m) | Y (m) | Z (m) | Status | Target | Reported (ms)\n"
                            "<none>\n\n");
}

TEST(TerminalView, GivenLiveTargetsAddedOutOfOrder_WhenRendered_ThenTheSnapshotIsStableAndReadable)
{
    TargetProjection projection;
    DroneProjection drones;
    ASSERT_EQ(projection.onTargetTrack(
                  TargetTrack{TargetId{2}, Position{-20.0, 21.1, 22.5}, Timestamp{2'000ms}}),
              drone::console::TargetUpdateResult::added);
    ASSERT_EQ(projection.onTargetTrack(
                  TargetTrack{TargetId{1}, Position{10.0, 11.5, 12.25}, Timestamp{1'000ms}}),
              drone::console::TargetUpdateResult::added);
    std::ostringstream output;
    TerminalView view{output};

    view.render(projection, drones);

    EXPECT_EQ(output.str(), "Targets (2)\n"
                            "ID | X (m) | Y (m) | Z (m) | Measured (ms)\n"
                            "1 | 10.00 | 11.50 | 12.25 | 1000\n"
                            "2 | -20.00 | 21.10 | 22.50 | 2000\n\n"
                            "Drones (0)\n"
                            "ID | X (m) | Y (m) | Z (m) | Status | Target | Reported (ms)\n"
                            "<none>\n\n");
}

TEST(TerminalView, GivenLiveDroneStates_WhenRendered_ThenLifecycleAndAssignmentAreReadable)
{
    TargetProjection targets;
    DroneProjection drones;
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{2}, Position{-2.0, 4.5, 8.25}, Timestamp{2'000ms},
                                       DroneStatus::assigned, TargetId{7}}),
        drone::console::DroneUpdateResult::added);
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{1}, Position{0.0, 0.0, 0.0}, Timestamp{1'000ms},
                                       DroneStatus::available, std::nullopt}),
        drone::console::DroneUpdateResult::added);
    std::ostringstream output;
    TerminalView view{output};

    view.render(targets, drones);

    EXPECT_EQ(output.str(), "Targets (0)\n"
                            "ID | X (m) | Y (m) | Z (m) | Measured (ms)\n"
                            "<none>\n\n"
                            "Drones (2)\n"
                            "ID | X (m) | Y (m) | Z (m) | Status | Target | Reported (ms)\n"
                            "1 | 0.00 | 0.00 | 0.00 | available | - | 1000\n"
                            "2 | -2.00 | 4.50 | 8.25 | assigned | 7 | 2000\n\n");
}

} // namespace
