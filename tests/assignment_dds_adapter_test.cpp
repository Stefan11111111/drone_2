#include "drone/console_dds_adapter/assignment_publisher.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/drone_state_reader.h"
#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/explosion_event.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/assignment_input_port.h"
#include "drone/interceptor_core/drone_state_output_port.h"
#include "drone/interceptor_core/explosion_event_output_port.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/interception_effect_port.h"
#include "drone/interceptor_core/interceptor_state_machine.h"
#include "drone/interceptor_core/positioning_port.h"
#include "drone/interceptor_dds_adapter/assignment_subscriber.h"
#include "drone/interceptor_dds_adapter/drone_state_publisher.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::console_dds::AssignmentPublisher;
using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::DroneStateReader;
using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::ExplosionEvent;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::Timestamp;
using drone::interceptor::AssignmentHandlingResult;
using drone::interceptor::AssignmentInputPort;
using drone::interceptor::ExplosionEventOutputPort;
using drone::interceptor::FlightControlPort;
using drone::interceptor::InterceptionEffectPort;
using drone::interceptor::InterceptionEffectResult;
using drone::interceptor::InterceptorStateMachine;
using drone::interceptor::PositioningPort;
using drone::interceptor::PositionSample;
using drone::interceptor_dds::AssignmentSubscriber;
using drone::interceptor_dds::DroneStatePublisher;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t assignmentDeliveryDomainId{183};
constexpr DomainId_t assignedStateDomainId{182};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};

class CapturingAssignmentInput final : public AssignmentInputPort
{
  public:
    AssignmentHandlingResult onAssignment(const Assignment &assignment) override
    {
        assignments.push_back(assignment);
        return AssignmentHandlingResult::applied;
    }

    std::vector<Assignment> assignments;
};

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

class IgnoringFlightControl final : public FlightControlPort, public InterceptionEffectPort
{
  public:
    void moveToward(const Position &destination, const Timestamp::Duration timeStep) override
    {
        static_cast<void>(destination);
        static_cast<void>(timeStep);
    }

    [[nodiscard]] InterceptionEffectResult trigger() override
    {
        return InterceptionEffectResult::succeeded;
    }
};

class IgnoringEventOutput final : public ExplosionEventOutputPort
{
  public:
    void publish(const ExplosionEvent &event) override
    {
        static_cast<void>(event);
    }
};

TEST(AssignmentDdsAdapter,
     GivenMatchedConsoleAndInterceptorAdapters_WhenAnAssignmentIsPublished_ThenCoreInputReceivesIt)
{
    CapturingAssignmentInput input;
    AssignmentSubscriber subscriber{assignmentDeliveryDomainId, "drone_step_34_interceptor", input};
    AssignmentPublisher publisher{assignmentDeliveryDomainId, "drone_step_34_console"};
    ASSERT_TRUE(publisher.waitForInterceptorMatch(discoveryTimeout))
        << "No interceptor Assignment DataReader matched within " << discoveryTimeout.count()
        << " ms";

    const Assignment sent{DroneId{7}, TargetId{42}};
    publisher.publish(sent);

    const auto received = subscriber.receiveNext(dataTimeout);
    ASSERT_TRUE(received.has_value())
        << "No valid Assignment reached interceptor core within " << dataTimeout.count() << " ms";
    EXPECT_EQ(input.assignments, (std::vector{sent}));
}

TEST(AssignmentDdsAdapter,
     GivenAValidDdsAssignment_WhenInterceptorCoreAppliesIt_ThenAssignedStateIsPublishedThroughDds)
{
    DomainParticipantOwner stateReaderParticipant{assignedStateDomainId,
                                                  "drone_step_35_state_probe"};
    DroneStateReader stateReader{stateReaderParticipant.participant()};
    DroneStatePublisher statePublisher{assignedStateDomainId, "drone_step_35_state_writer"};
    FixedPositioning positioning;
    IgnoringFlightControl flightControl;
    IgnoringEventOutput eventOutput;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        statePublisher,
                                        eventOutput};
    AssignmentSubscriber assignmentSubscriber{assignedStateDomainId,
                                              "drone_step_35_assignment_reader", interceptor};
    AssignmentPublisher assignmentPublisher{assignedStateDomainId,
                                            "drone_step_35_assignment_writer"};
    ASSERT_TRUE(stateReader.waitForWriterMatch(discoveryTimeout));
    ASSERT_TRUE(assignmentPublisher.waitForInterceptorMatch(discoveryTimeout));
    interceptor.start();
    ASSERT_TRUE(stateReader.waitForData(dataTimeout));
    ASSERT_TRUE(stateReader.takeNext().has_value());
    positioning.sample.measuredAt = Timestamp{2'100ms};

    assignmentPublisher.publish(Assignment{DroneId{7}, TargetId{42}});
    ASSERT_TRUE(assignmentSubscriber.receiveNext(dataTimeout).has_value());

    ASSERT_TRUE(stateReader.waitForData(dataTimeout));
    const auto received = stateReader.takeNext();
    ASSERT_TRUE(received.has_value());
    ASSERT_TRUE(received->has_value());
    EXPECT_EQ(*received, (std::optional{DroneState{DroneId{7}, positioning.sample.position,
                                                   positioning.sample.measuredAt,
                                                   DroneStatus::assigned, TargetId{42}}}));
}

} // namespace
