#include "drone/dds_transport/drone_state_mapping.h"

#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

#include <target_track.hpp>

#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

#include <gtest/gtest.h>

namespace
{

using drone::dds::CartesianPosition;
using WireDroneState = drone::dds::DroneState;
using WireDroneStatus = drone::dds::DroneStatus;
using drone::dds_transport::DroneStateMappingError;
using drone::dds_transport::fromWireDroneState;
using drone::dds_transport::toWireDroneState;
using drone::domain::DroneId;
using DomainDroneState = drone::domain::DroneState;
using DomainDroneStatus = drone::domain::DroneStatus;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

struct StatusMappingCase final
{
    DomainDroneStatus domainStatus;
    WireDroneStatus wireStatus;
    bool hasAssignedTarget;
};

constexpr std::array statusMappings{
    StatusMappingCase{.domainStatus = DomainDroneStatus::available,
                      .wireStatus = WireDroneStatus::DRONE_AVAILABLE,
                      .hasAssignedTarget = false},
    StatusMappingCase{.domainStatus = DomainDroneStatus::assigned,
                      .wireStatus = WireDroneStatus::DRONE_ASSIGNED,
                      .hasAssignedTarget = true},
    StatusMappingCase{.domainStatus = DomainDroneStatus::intercepting,
                      .wireStatus = WireDroneStatus::DRONE_INTERCEPTING,
                      .hasAssignedTarget = true},
    StatusMappingCase{.domainStatus = DomainDroneStatus::interceptionSucceeded,
                      .wireStatus = WireDroneStatus::DRONE_INTERCEPTION_SUCCEEDED,
                      .hasAssignedTarget = true},
    StatusMappingCase{.domainStatus = DomainDroneStatus::interceptionFailed,
                      .wireStatus = WireDroneStatus::DRONE_INTERCEPTION_FAILED,
                      .hasAssignedTarget = true},
};

[[nodiscard]] WireDroneState makeWireState()
{
    CartesianPosition position;
    position.x(12.5);
    position.y(-4.25);
    position.z(120.0);

    WireDroneState state;
    state.drone_id(7);
    state.position(position);
    state.reported_at_ms(2'000);
    state.status(WireDroneStatus::DRONE_INTERCEPTING);
    state.assigned_target_id(42);
    return state;
}

TEST(DroneStateMapping, GivenEveryDomainStatus_WhenMappedToWireAndBack_ThenNoDataIsLost)
{
    for (const auto &[domainStatus, wireStatus, hasAssignedTarget] : statusMappings)
    {
        SCOPED_TRACE(std::to_underlying(domainStatus));
        const std::optional<TargetId> assignedTargetId =
            hasAssignedTarget ? std::optional{TargetId{42}} : std::nullopt;
        const DomainDroneState original{DroneId{7}, Position{12.5, -4.25, 120.0},
                                        Timestamp{2'000ms}, domainStatus, assignedTargetId};

        const WireDroneState wireState = toWireDroneState(original);
        const auto mapped = fromWireDroneState(wireState);

        EXPECT_EQ(wireState.status(), wireStatus);
        ASSERT_TRUE(mapped.has_value());
        EXPECT_EQ(*mapped, original);
    }
}

TEST(DroneStateMapping, GivenZeroDroneId_WhenMappedFromWire_ThenTheStateIsRejected)
{
    WireDroneState wireState{makeWireState()};
    wireState.drone_id(0);

    const auto mapped = fromWireDroneState(wireState);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), DroneStateMappingError::zeroDroneId);
}

TEST(DroneStateMapping, GivenNonFinitePosition_WhenMappedFromWire_ThenTheStateIsRejected)
{
    WireDroneState wireState{makeWireState()};
    wireState.position().z(std::numeric_limits<double>::infinity());

    const auto mapped = fromWireDroneState(wireState);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), DroneStateMappingError::nonFinitePosition);
}

TEST(DroneStateMapping, GivenNegativeTimestamp_WhenMappedFromWire_ThenTheStateIsRejected)
{
    WireDroneState wireState{makeWireState()};
    wireState.reported_at_ms(-1);

    const auto mapped = fromWireDroneState(wireState);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), DroneStateMappingError::timestampBeforeUnixEpoch);
}

TEST(DroneStateMapping, GivenUnknownStatus_WhenMappedFromWire_ThenTheStateIsRejected)
{
    WireDroneState wireState{makeWireState()};
    wireState.status(std::bit_cast<WireDroneStatus>(std::int32_t{99}));

    const auto mapped = fromWireDroneState(wireState);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), DroneStateMappingError::unknownStatus);
}

TEST(DroneStateMapping, GivenZeroAssignedTargetId_WhenMappedFromWire_ThenTheStateIsRejected)
{
    WireDroneState wireState{makeWireState()};
    wireState.assigned_target_id(0);

    const auto mapped = fromWireDroneState(wireState);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), DroneStateMappingError::zeroAssignedTargetId);
}

TEST(DroneStateMapping, GivenAssignmentAndStatusDisagree_WhenMappedFromWire_ThenTheStateIsRejected)
{
    WireDroneState availableWithTarget{makeWireState()};
    availableWithTarget.status(WireDroneStatus::DRONE_AVAILABLE);
    WireDroneState assignedWithoutTarget{makeWireState()};
    assignedWithoutTarget.assigned_target_id().reset();

    const auto mappedAvailable = fromWireDroneState(availableWithTarget);
    const auto mappedAssigned = fromWireDroneState(assignedWithoutTarget);

    ASSERT_FALSE(mappedAvailable.has_value());
    EXPECT_EQ(mappedAvailable.error(), DroneStateMappingError::assignmentDoesNotMatchStatus);
    ASSERT_FALSE(mappedAssigned.has_value());
    EXPECT_EQ(mappedAssigned.error(), DroneStateMappingError::assignmentDoesNotMatchStatus);
}

} // namespace
