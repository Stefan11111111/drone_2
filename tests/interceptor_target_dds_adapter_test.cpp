#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_writer.h"
#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/drone_state_output_port.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/interception_effect_port.h"
#include "drone/interceptor_core/interceptor_state_machine.h"
#include "drone/interceptor_core/positioning_port.h"
#include "drone/interceptor_core/target_track_input_port.h"
#include "drone/interceptor_dds_adapter/target_track_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::TargetTrackWriter;
using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using drone::interceptor::DroneStateOutputPort;
using drone::interceptor::FlightControlPort;
using drone::interceptor::InterceptionEffectPort;
using drone::interceptor::InterceptionEffectResult;
using drone::interceptor::InterceptorStateMachine;
using drone::interceptor::PositioningPort;
using drone::interceptor::PositionSample;
using drone::interceptor::TargetTrackHandlingResult;
using drone::interceptor_dds::TargetTrackSubscriber;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t targetDeliveryDomainId{178};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};

class FixedPositioning final : public PositioningPort
{
  public:
    [[nodiscard]] PositionSample currentPosition() const override
    {
        return {.position = Position{0.0, 0.0, 0.0}, .measuredAt = Timestamp{1'000ms}};
    }
};

class CapturingFlightControl final : public FlightControlPort, public InterceptionEffectPort
{
  public:
    void moveToward(const Position &destination, const Timestamp::Duration timeStep) override
    {
        destinations.push_back(destination);
        timeSteps.push_back(timeStep);
    }

    [[nodiscard]] InterceptionEffectResult trigger() override
    {
        return InterceptionEffectResult::succeeded;
    }

    std::vector<Position> destinations;
    std::vector<Timestamp::Duration> timeSteps;
};

class IgnoringStateOutput final : public DroneStateOutputPort
{
  public:
    void publish(const DroneState &state) override
    {
        static_cast<void>(state);
    }
};

TEST(InterceptorTargetDdsAdapter,
     GivenDdsTracks_WhenTheyAreRelevantAndNewer_ThenCoreRetainsOnlyTheLatestAssignedTarget)
{
    FixedPositioning positioning;
    CapturingFlightControl flightControl;
    IgnoringStateOutput stateOutput;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        positioning,
                                        flightControl,
                                        flightControl,
                                        stateOutput};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              drone::interceptor::AssignmentHandlingResult::applied);
    TargetTrackSubscriber subscriber{targetDeliveryDomainId, "drone_step_38_interceptor",
                                     interceptor};
    DomainParticipantOwner writerParticipant{targetDeliveryDomainId, "drone_step_38_observer"};
    TargetTrackWriter writer{writerParticipant.participant()};
    ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout));

    writer.write(TargetTrack{TargetId{43}, Position{9.0, 9.0, 9.0}, Timestamp{3'000ms}});
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), TargetTrackHandlingResult::unrelated);
    EXPECT_FALSE(interceptor.latestTargetTrack().has_value());

    const TargetTrack current{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{3'000ms}};
    writer.write(current);
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), TargetTrackHandlingResult::accepted);
    EXPECT_EQ(interceptor.latestTargetTrack(), current);

    writer.write(TargetTrack{TargetId{42}, Position{1.0, 2.0, 3.0}, Timestamp{2'000ms}});
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), TargetTrackHandlingResult::stale);
    EXPECT_EQ(interceptor.latestTargetTrack(), current);

    const TargetTrack newer{TargetId{42}, Position{11.0, 22.0, 33.0}, Timestamp{4'000ms}};
    writer.write(newer);
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), TargetTrackHandlingResult::updated);
    EXPECT_EQ(interceptor.latestTargetTrack(), newer);
    EXPECT_TRUE(flightControl.destinations.empty());
}

} // namespace
