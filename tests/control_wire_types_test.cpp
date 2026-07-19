#include <target_track.hpp>
#include <target_trackPubSubTypes.hpp>

#include <cstdint>
#include <type_traits>

#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <gtest/gtest.h>

namespace
{

using drone::dds::Assignment;
using drone::dds::AssignmentPubSubType;
using drone::dds::InterceptionCommand;
using drone::dds::InterceptionCommandPubSubType;
using eprosima::fastdds::rtps::InstanceHandle_t;

static_assert(!std::is_same_v<Assignment, InterceptionCommand>);

struct AssignmentValues
{
    std::uint64_t droneId;
    std::uint64_t targetId;
};

struct CommandValues
{
    std::uint64_t commandId;
    std::uint64_t droneId;
    std::uint64_t targetId;
};

template <typename TopicType, typename Sample>
[[nodiscard]] InstanceHandle_t computeKey(TopicType &topicType, const Sample &sample)
{
    InstanceHandle_t handle;
    EXPECT_TRUE(topicType.compute_key(&sample, handle));
    return handle;
}

[[nodiscard]] Assignment makeAssignment(const AssignmentValues values)
{
    Assignment assignment;
    assignment.drone_id(values.droneId);
    assignment.target_id(values.targetId);
    return assignment;
}

[[nodiscard]] InterceptionCommand makeCommand(const CommandValues values)
{
    InterceptionCommand command;
    command.command_id(values.commandId);
    command.drone_id(values.droneId);
    command.target_id(values.targetId);
    return command;
}

TEST(ControlWireTypes,
     GivenGeneratedAssignment_WhenPopulated_ThenItCarriesOnlyTheDroneToTargetSelection)
{
    const Assignment assignment{makeAssignment({.droneId = 7, .targetId = 42})};

    EXPECT_EQ(assignment.drone_id(), 7U);
    EXPECT_EQ(assignment.target_id(), 42U);
}

TEST(ControlWireTypes,
     GivenGeneratedAssignmentTopicType_WhenKeysAreComputed_ThenDroneIdIdentifiesTheInstance)
{
    AssignmentPubSubType topicType;
    const Assignment firstSelection{makeAssignment({.droneId = 7, .targetId = 42})};
    const Assignment changedSelection{makeAssignment({.droneId = 7, .targetId = 43})};
    const Assignment differentDrone{makeAssignment({.droneId = 8, .targetId = 42})};

    EXPECT_EQ(computeKey(topicType, firstSelection), computeKey(topicType, changedSelection));
    EXPECT_NE(computeKey(topicType, firstSelection), computeKey(topicType, differentDrone));
}

TEST(ControlWireTypes,
     GivenGeneratedInterceptionCommand_WhenPopulated_ThenItCarriesDistinctIntentIdentity)
{
    const InterceptionCommand command{
        makeCommand({.commandId = 101, .droneId = 7, .targetId = 42})};

    EXPECT_EQ(command.command_id(), 101U);
    EXPECT_EQ(command.drone_id(), 7U);
    EXPECT_EQ(command.target_id(), 42U);
}

TEST(ControlWireTypes,
     GivenGeneratedCommandTopicType_WhenKeysAreComputed_ThenCommandIdIdentifiesTheInstance)
{
    InterceptionCommandPubSubType topicType;
    const InterceptionCommand firstIntent{
        makeCommand({.commandId = 101, .droneId = 7, .targetId = 42})};
    const InterceptionCommand sameIdentityDifferentPayload{
        makeCommand({.commandId = 101, .droneId = 8, .targetId = 43})};
    const InterceptionCommand differentIntent{
        makeCommand({.commandId = 102, .droneId = 7, .targetId = 42})};

    EXPECT_EQ(computeKey(topicType, firstIntent),
              computeKey(topicType, sameIdentityDifferentPayload));
    EXPECT_NE(computeKey(topicType, firstIntent), computeKey(topicType, differentIntent));
}

} // namespace
