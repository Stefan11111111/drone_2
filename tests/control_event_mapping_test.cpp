#include "drone/dds_transport/assignment_mapping.h"
#include "drone/dds_transport/explosion_event_mapping.h"
#include "drone/dds_transport/interception_command_mapping.h"

#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/explosion_event.h"
#include "drone/domain/explosion_event_id.h"
#include "drone/domain/interception_command.h"
#include "drone/domain/interception_command_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

#include <target_track.hpp>

#include <chrono>
#include <limits>

#include <gtest/gtest.h>

namespace
{

using WireAssignment = drone::dds::Assignment;
using drone::dds::CartesianPosition;
using WireExplosionEvent = drone::dds::ExplosionEvent;
using WireInterceptionCommand = drone::dds::InterceptionCommand;
using drone::dds_transport::AssignmentMappingError;
using drone::dds_transport::ExplosionEventMappingError;
using drone::dds_transport::fromWireAssignment;
using drone::dds_transport::fromWireExplosionEvent;
using drone::dds_transport::fromWireInterceptionCommand;
using drone::dds_transport::InterceptionCommandMappingError;
using drone::dds_transport::toWireAssignment;
using drone::dds_transport::toWireExplosionEvent;
using drone::dds_transport::toWireInterceptionCommand;
using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::ExplosionEvent;
using drone::domain::ExplosionEventId;
using drone::domain::InterceptionCommand;
using drone::domain::InterceptionCommandId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

[[nodiscard]] WireAssignment makeWireAssignment()
{
    WireAssignment assignment;
    assignment.drone_id(7);
    assignment.target_id(42);
    return assignment;
}

[[nodiscard]] WireInterceptionCommand makeWireCommand()
{
    WireInterceptionCommand command;
    command.command_id(101);
    command.drone_id(7);
    command.target_id(42);
    return command;
}

[[nodiscard]] WireExplosionEvent makeWireEvent()
{
    CartesianPosition position;
    position.x(125.0);
    position.y(-30.5);
    position.z(850.0);

    WireExplosionEvent event;
    event.event_id(301);
    event.drone_id(7);
    event.target_id(42);
    event.position(position);
    event.occurred_at_ms(2'000);
    return event;
}

TEST(ControlEventMapping, GivenValidAssignment_WhenMappedToWireAndBack_ThenNoDataIsLost)
{
    const Assignment original{DroneId{7}, TargetId{42}};

    const auto mapped = fromWireAssignment(toWireAssignment(original));

    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(*mapped, original);
}

TEST(ControlEventMapping, GivenZeroDroneId_WhenAssignmentIsMappedFromWire_ThenItIsRejected)
{
    WireAssignment wireAssignment{makeWireAssignment()};
    wireAssignment.drone_id(0);

    const auto mapped = fromWireAssignment(wireAssignment);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), AssignmentMappingError::zeroDroneId);
}

TEST(ControlEventMapping, GivenZeroTargetId_WhenAssignmentIsMappedFromWire_ThenItIsRejected)
{
    WireAssignment wireAssignment{makeWireAssignment()};
    wireAssignment.target_id(0);

    const auto mapped = fromWireAssignment(wireAssignment);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), AssignmentMappingError::zeroTargetId);
}

TEST(ControlEventMapping, GivenValidCommand_WhenMappedToWireAndBack_ThenNoDataIsLost)
{
    const InterceptionCommand original{InterceptionCommandId{101}, DroneId{7}, TargetId{42}};

    const auto mapped = fromWireInterceptionCommand(toWireInterceptionCommand(original));

    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(*mapped, original);
}

TEST(ControlEventMapping, GivenZeroCommandId_WhenCommandIsMappedFromWire_ThenItIsRejected)
{
    WireInterceptionCommand wireCommand{makeWireCommand()};
    wireCommand.command_id(0);

    const auto mapped = fromWireInterceptionCommand(wireCommand);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), InterceptionCommandMappingError::zeroCommandId);
}

TEST(ControlEventMapping, GivenZeroDroneId_WhenCommandIsMappedFromWire_ThenItIsRejected)
{
    WireInterceptionCommand wireCommand{makeWireCommand()};
    wireCommand.drone_id(0);

    const auto mapped = fromWireInterceptionCommand(wireCommand);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), InterceptionCommandMappingError::zeroDroneId);
}

TEST(ControlEventMapping, GivenZeroTargetId_WhenCommandIsMappedFromWire_ThenItIsRejected)
{
    WireInterceptionCommand wireCommand{makeWireCommand()};
    wireCommand.target_id(0);

    const auto mapped = fromWireInterceptionCommand(wireCommand);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), InterceptionCommandMappingError::zeroTargetId);
}

TEST(ControlEventMapping, GivenValidExplosionEvent_WhenMappedToWireAndBack_ThenNoDataIsLost)
{
    const ExplosionEvent original{ExplosionEventId{301}, DroneId{7}, TargetId{42},
                                  Position{125.0, -30.5, 850.0}, Timestamp{2'000ms}};

    const auto mapped = fromWireExplosionEvent(toWireExplosionEvent(original));

    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(*mapped, original);
}

TEST(ControlEventMapping, GivenZeroEventId_WhenExplosionEventIsMappedFromWire_ThenItIsRejected)
{
    WireExplosionEvent wireEvent{makeWireEvent()};
    wireEvent.event_id(0);

    const auto mapped = fromWireExplosionEvent(wireEvent);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), ExplosionEventMappingError::zeroEventId);
}

TEST(ControlEventMapping, GivenZeroDroneId_WhenExplosionEventIsMappedFromWire_ThenItIsRejected)
{
    WireExplosionEvent wireEvent{makeWireEvent()};
    wireEvent.drone_id(0);

    const auto mapped = fromWireExplosionEvent(wireEvent);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), ExplosionEventMappingError::zeroDroneId);
}

TEST(ControlEventMapping, GivenZeroTargetId_WhenExplosionEventIsMappedFromWire_ThenItIsRejected)
{
    WireExplosionEvent wireEvent{makeWireEvent()};
    wireEvent.target_id(0);

    const auto mapped = fromWireExplosionEvent(wireEvent);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), ExplosionEventMappingError::zeroTargetId);
}

TEST(ControlEventMapping,
     GivenNonFinitePosition_WhenExplosionEventIsMappedFromWire_ThenItIsRejected)
{
    WireExplosionEvent wireEvent{makeWireEvent()};
    wireEvent.position().x(std::numeric_limits<double>::quiet_NaN());

    const auto mapped = fromWireExplosionEvent(wireEvent);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), ExplosionEventMappingError::nonFinitePosition);
}

TEST(ControlEventMapping,
     GivenNegativeTimestamp_WhenExplosionEventIsMappedFromWire_ThenItIsRejected)
{
    WireExplosionEvent wireEvent{makeWireEvent()};
    wireEvent.occurred_at_ms(-1);

    const auto mapped = fromWireExplosionEvent(wireEvent);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), ExplosionEventMappingError::timestampBeforeUnixEpoch);
}

} // namespace
