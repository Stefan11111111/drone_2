#include <target_track.hpp>
#include <target_trackPubSubTypes.hpp>

#include <array>
#include <cstdint>
#include <utility>

#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <gtest/gtest.h>

namespace
{

using drone::dds::CartesianPosition;
using drone::dds::DroneState;
using drone::dds::DroneStatePubSubType;
using drone::dds::DroneStatus;
using eprosima::fastdds::rtps::InstanceHandle_t;

[[nodiscard]] DroneState makeState(const std::uint64_t droneId)
{
    CartesianPosition position;
    position.x(12.5);
    position.y(-4.25);
    position.z(120.0);

    DroneState state;
    state.drone_id(droneId);
    state.position(position);
    state.reported_at_ms(2'000);
    state.status(DroneStatus::DRONE_INTERCEPTING);
    state.assigned_target_id(42);
    return state;
}

[[nodiscard]] InstanceHandle_t computeKey(DroneStatePubSubType &topicType, const DroneState &state)
{
    InstanceHandle_t handle;
    EXPECT_TRUE(topicType.compute_key(&state, handle));
    return handle;
}

TEST(DroneStateWireType, GivenGeneratedDroneState_WhenPopulated_ThenAllIdlFieldsAreAvailable)
{
    const DroneState state{makeState(7)};

    EXPECT_EQ(state.drone_id(), 7U);
    EXPECT_DOUBLE_EQ(state.position().x(), 12.5);
    EXPECT_DOUBLE_EQ(state.position().y(), -4.25);
    EXPECT_DOUBLE_EQ(state.position().z(), 120.0);
    EXPECT_EQ(state.reported_at_ms(), 2'000);
    EXPECT_EQ(state.status(), DroneStatus::DRONE_INTERCEPTING);
    ASSERT_TRUE(state.assigned_target_id().has_value());
    EXPECT_EQ(state.assigned_target_id().value(), 42U);
}

TEST(DroneStateWireType, GivenAvailableGeneratedDroneState_WhenDefaulted_ThenTargetIsAbsent)
{
    DroneState state;
    state.drone_id(7);
    state.status(DroneStatus::DRONE_AVAILABLE);

    EXPECT_FALSE(state.assigned_target_id().has_value());
}

TEST(DroneStateWireType, GivenGeneratedDroneStatus_WhenEnumerated_ThenEveryLifecycleValueIsStable)
{
    constexpr std::array statuses{
        DroneStatus::DRONE_AVAILABLE,           DroneStatus::DRONE_ASSIGNED,
        DroneStatus::DRONE_INTERCEPTING,        DroneStatus::DRONE_INTERCEPTION_SUCCEEDED,
        DroneStatus::DRONE_INTERCEPTION_FAILED,
    };

    for (std::size_t index{0}; index < statuses.size(); ++index)
    {
        EXPECT_EQ(std::to_underlying(statuses[index]), static_cast<std::int32_t>(index));
    }
}

TEST(DroneStateWireType,
     GivenGeneratedTopicDataType_WhenKeysAreComputed_ThenOnlyDroneIdIdentifiesTheInstance)
{
    DroneStatePubSubType topicType;
    const DroneState firstReport{makeState(7)};
    DroneState newerReport{makeState(7)};
    newerReport.position().x(13.5);
    newerReport.status(DroneStatus::DRONE_INTERCEPTION_SUCCEEDED);
    const DroneState differentDrone{makeState(8)};

    EXPECT_EQ(computeKey(topicType, firstReport), computeKey(topicType, newerReport));
    EXPECT_NE(computeKey(topicType, firstReport), computeKey(topicType, differentDrone));
}

} // namespace
