#include "drone/domain/drone_state.h"

#include <array>
#include <chrono>
#include <optional>
#include <stdexcept>

#include <gtest/gtest.h>

namespace
{

using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

TEST(DroneState, GivenAvailableDrone_WhenConstructed_ThenItReportsIdentityPositionTimeAndStatus)
{
    const DroneId droneId{7};
    const Position position{12.5, -4.25, 120.0};
    const Timestamp reportedAt{2'000ms};

    const DroneState state{droneId, position, reportedAt, DroneStatus::available, std::nullopt};

    EXPECT_EQ(state.droneId(), droneId);
    EXPECT_EQ(state.position(), position);
    EXPECT_EQ(state.reportedAt(), reportedAt);
    EXPECT_EQ(state.status(), DroneStatus::available);
    EXPECT_FALSE(state.assignedTargetId().has_value());
}

TEST(DroneState, GivenAssignedLifecycleStates_WhenConstructed_ThenTheyPreserveTheTarget)
{
    const TargetId targetId{42};
    constexpr std::array assignedStatuses{
        DroneStatus::assigned,
        DroneStatus::intercepting,
        DroneStatus::interceptionSucceeded,
        DroneStatus::interceptionFailed,
    };

    for (const DroneStatus status : assignedStatuses)
    {
        const DroneState state{DroneId{7}, Position{12.5, -4.25, 120.0}, Timestamp{2'000ms}, status,
                               targetId};

        EXPECT_EQ(state.status(), status);
        EXPECT_EQ(state.assignedTargetId(), targetId);
    }
}

TEST(DroneState, GivenStatusAndTargetDisagree_WhenConstructed_ThenTheStateIsRejected)
{
    EXPECT_THROW((DroneState{DroneId{7}, Position{0.0, 0.0, 0.0}, Timestamp{0ms},
                             DroneStatus::available, TargetId{42}}),
                 std::invalid_argument);
    EXPECT_THROW((DroneState{DroneId{7}, Position{0.0, 0.0, 0.0}, Timestamp{0ms},
                             DroneStatus::assigned, std::nullopt}),
                 std::invalid_argument);
}

TEST(DroneState, GivenStates_WhenCompared_ThenEveryPublishedFieldParticipatesInEquality)
{
    const DroneState state{DroneId{7}, Position{12.5, -4.25, 120.0}, Timestamp{2'000ms},
                           DroneStatus::intercepting, TargetId{42}};
    const DroneState copy{DroneId{7}, Position{12.5, -4.25, 120.0}, Timestamp{2'000ms},
                          DroneStatus::intercepting, TargetId{42}};
    const DroneState differentDrone{DroneId{8}, Position{12.5, -4.25, 120.0}, Timestamp{2'000ms},
                                    DroneStatus::intercepting, TargetId{42}};
    const DroneState differentPosition{DroneId{7}, Position{13.5, -4.25, 120.0}, Timestamp{2'000ms},
                                       DroneStatus::intercepting, TargetId{42}};
    const DroneState differentTime{DroneId{7}, Position{12.5, -4.25, 120.0}, Timestamp{2'001ms},
                                   DroneStatus::intercepting, TargetId{42}};
    const DroneState differentStatus{DroneId{7}, Position{12.5, -4.25, 120.0}, Timestamp{2'000ms},
                                     DroneStatus::assigned, TargetId{42}};
    const DroneState differentTarget{DroneId{7}, Position{12.5, -4.25, 120.0}, Timestamp{2'000ms},
                                     DroneStatus::intercepting, TargetId{43}};

    EXPECT_EQ(state, copy);
    EXPECT_NE(state, differentDrone);
    EXPECT_NE(state, differentPosition);
    EXPECT_NE(state, differentTime);
    EXPECT_NE(state, differentStatus);
    EXPECT_NE(state, differentTarget);
}

} // namespace
