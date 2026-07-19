#include "drone/domain/assignment.h"
#include "drone/domain/interception_command.h"

#include <stdexcept>
#include <type_traits>

#include <gtest/gtest.h>

namespace
{

using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::InterceptionCommand;
using drone::domain::InterceptionCommandId;
using drone::domain::TargetId;

static_assert(!std::is_constructible_v<TargetId, DroneId>);
static_assert(!std::is_constructible_v<DroneId, TargetId>);
static_assert(!std::is_constructible_v<InterceptionCommandId, DroneId>);
static_assert(!std::is_constructible_v<InterceptionCommandId, TargetId>);

TEST(ControlMessages, GivenValidIdentifiers_WhenAssignmentIsConstructed_ThenItNamesDroneAndTarget)
{
    const Assignment assignment{DroneId{7}, TargetId{42}};

    EXPECT_EQ(assignment.droneId(), DroneId{7});
    EXPECT_EQ(assignment.targetId(), TargetId{42});
}

TEST(ControlMessages, GivenAssignments_WhenCompared_ThenDroneAndTargetParticipateInEquality)
{
    const Assignment assignment{DroneId{7}, TargetId{42}};

    EXPECT_EQ(assignment, (Assignment{DroneId{7}, TargetId{42}}));
    EXPECT_NE(assignment, (Assignment{DroneId{8}, TargetId{42}}));
    EXPECT_NE(assignment, (Assignment{DroneId{7}, TargetId{43}}));
}

TEST(ControlMessages, GivenZeroDroneOrTargetIdentifier_WhenAssigning_ThenItIsRejected)
{
    EXPECT_THROW((Assignment{DroneId{0}, TargetId{42}}), std::invalid_argument);
    EXPECT_THROW((Assignment{DroneId{7}, TargetId{0}}), std::invalid_argument);
}

TEST(ControlMessages,
     GivenValidIdentifiers_WhenInterceptionCommandIsConstructed_ThenItPreservesTheIntent)
{
    const InterceptionCommandId commandId{101};
    const InterceptionCommand command{commandId, DroneId{7}, TargetId{42}};

    EXPECT_EQ(command.commandId(), commandId);
    EXPECT_EQ(command.droneId(), DroneId{7});
    EXPECT_EQ(command.targetId(), TargetId{42});
}

TEST(ControlMessages, GivenCommands_WhenCompared_ThenEveryIntentFieldParticipatesInEquality)
{
    const InterceptionCommand command{InterceptionCommandId{101}, DroneId{7}, TargetId{42}};

    EXPECT_EQ(command, (InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{42}}));
    EXPECT_NE(command, (InterceptionCommand{InterceptionCommandId{102}, DroneId{7}, TargetId{42}}));
    EXPECT_NE(command, (InterceptionCommand{InterceptionCommandId{101}, DroneId{8}, TargetId{42}}));
    EXPECT_NE(command, (InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{43}}));
}

TEST(ControlMessages, GivenAnyZeroIdentifier_WhenCommandingInterception_ThenItIsRejected)
{
    EXPECT_THROW((InterceptionCommand{InterceptionCommandId{0}, DroneId{7}, TargetId{42}}),
                 std::invalid_argument);
    EXPECT_THROW((InterceptionCommand{InterceptionCommandId{101}, DroneId{0}, TargetId{42}}),
                 std::invalid_argument);
    EXPECT_THROW((InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{0}}),
                 std::invalid_argument);
}

TEST(ControlMessages, GivenNonzeroCommandIdentifiers_WhenCompared_ThenTheyHaveValueSemantics)
{
    const InterceptionCommandId commandId{101};

    EXPECT_EQ(commandId.value(), 101U);
    EXPECT_EQ(commandId, InterceptionCommandId{101});
    EXPECT_LT(commandId, InterceptionCommandId{102});
}

} // namespace
