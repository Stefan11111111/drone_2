#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/explosion_event_mapping.h"
#include "drone/dds_transport/explosion_event_topic.h"
#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/explosion_event.h"
#include "drone/domain/explosion_event_id.h"
#include "drone/domain/interception_command.h"
#include "drone/domain/interception_command_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/drone_state_output_port.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/interception_effect_port.h"
#include "drone/interceptor_core/interceptor_state_machine.h"
#include "drone/interceptor_core/positioning_port.h"
#include "drone/interceptor_dds_adapter/explosion_event_publisher.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>

#include <target_track.hpp>

#include <chrono>
#include <cstddef>
#include <optional>
#include <stdexcept>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::explosionEventReaderQos;
using drone::dds_transport::ExplosionEventTopic;
using drone::dds_transport::explosionEventWriterQos;
using drone::dds_transport::fromWireExplosionEvent;
using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::ExplosionEvent;
using drone::domain::ExplosionEventId;
using drone::domain::InterceptionCommand;
using drone::domain::InterceptionCommandId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using drone::interceptor::DroneStateOutputPort;
using drone::interceptor::FlightControlPort;
using drone::interceptor::InterceptionEffectPort;
using drone::interceptor::InterceptionEffectResult;
using drone::interceptor::InterceptionTickResult;
using drone::interceptor::InterceptorStateMachine;
using drone::interceptor::PositioningPort;
using drone::interceptor::PositionSample;
using drone::interceptor_dds::ExplosionEventPublisher;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t explosionEventDomainId{193};
constexpr auto dataTimeout{5s};

class ArrivedVehicle final : public PositioningPort,
                             public FlightControlPort,
                             public InterceptionEffectPort
{
  public:
    [[nodiscard]] PositionSample currentPosition() const override
    {
        return {.position = Position{10.0, 20.0, 30.0}, .measuredAt = Timestamp{2'000ms}};
    }

    void moveToward(const Position &destination, const Timestamp::Duration timeStep) override
    {
        static_cast<void>(destination);
        static_cast<void>(timeStep);
    }

    [[nodiscard]] InterceptionEffectResult trigger() override
    {
        ++effectCount;
        return InterceptionEffectResult::succeeded;
    }

    std::size_t effectCount{};
};

class IgnoringStateOutput final : public DroneStateOutputPort
{
  public:
    void publish(const DroneState &state) override
    {
        static_cast<void>(state);
    }
};

class WireExplosionEventReader final
{
  public:
    explicit WireExplosionEventReader(eprosima::fastdds::dds::DomainParticipant &participant)
        : participant_{participant}, topic_{participant}
    {
        subscriber_ =
            participant_.create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
        if (subscriber_ == nullptr)
        {
            throw std::runtime_error{"Could not create the probe ExplosionEvent Subscriber"};
        }

        reader_ = subscriber_->create_datareader(&topic_.topic(), explosionEventReaderQos());
        if (reader_ == nullptr)
        {
            static_cast<void>(participant_.delete_subscriber(subscriber_));
            subscriber_ = nullptr;
            throw std::runtime_error{"Could not create the probe ExplosionEvent DataReader"};
        }
    }

    ~WireExplosionEventReader() noexcept
    {
        if (reader_ != nullptr)
        {
            static_cast<void>(subscriber_->delete_datareader(reader_));
        }
        if (subscriber_ != nullptr)
        {
            static_cast<void>(participant_.delete_subscriber(subscriber_));
        }
    }

    WireExplosionEventReader(const WireExplosionEventReader &) = delete;
    WireExplosionEventReader &operator=(const WireExplosionEventReader &) = delete;
    WireExplosionEventReader(WireExplosionEventReader &&) = delete;
    WireExplosionEventReader &operator=(WireExplosionEventReader &&) = delete;

    [[nodiscard]] std::optional<ExplosionEvent> receive(const std::chrono::milliseconds timeout)
    {
        const auto duration =
            eprosima::fastdds::dds::Duration_t{std::chrono::duration<long double>{timeout}.count()};
        if (!reader_->wait_for_unread_message(duration))
        {
            return std::nullopt;
        }

        drone::dds::ExplosionEvent wireEvent;
        eprosima::fastdds::dds::SampleInfo sampleInfo{};
        if (reader_->take_next_sample(&wireEvent, &sampleInfo) !=
                eprosima::fastdds::dds::RETCODE_OK ||
            !sampleInfo.valid_data)
        {
            return std::nullopt;
        }

        const auto event = fromWireExplosionEvent(wireEvent);
        if (!event.has_value())
        {
            return std::nullopt;
        }
        return *event;
    }

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    ExplosionEventTopic topic_;
    eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};
    eprosima::fastdds::dds::DataReader *reader_{nullptr};
};

template <typename EndpointQos> void expectDeliveryQos(const EndpointQos &qos)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_EQ(qos.reliability().kind, RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(qos.durability().kind, TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_EQ(qos.history().kind, KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(qos.history().depth, 1);
}

template <typename EndpointQos> void expectResourceLimits(const EndpointQos &qos)
{
    EXPECT_EQ(qos.resource_limits().max_instances, 256);
    EXPECT_EQ(qos.resource_limits().max_samples, 256);
    EXPECT_EQ(qos.resource_limits().max_samples_per_instance, 1);
}

TEST(ExplosionEventDdsAdapter,
     GivenTheCatalogEventPolicy_WhenEndpointQosIsBuilt_ThenWriterAndReaderAgree)
{
    const auto writerQos = explosionEventWriterQos();
    const auto readerQos = explosionEventReaderQos();
    expectDeliveryQos(writerQos);
    expectResourceLimits(writerQos);
    expectDeliveryQos(readerQos);
    expectResourceLimits(readerQos);
}

TEST(ExplosionEventDdsAdapter,
     GivenOneSuccessfulInterception_WhenALateReaderMatches_ThenExactlyOneCorrelatedEventArrives)
{
    ExplosionEventPublisher eventPublisher{explosionEventDomainId, "drone_step_42_writer"};
    ArrivedVehicle vehicle;
    IgnoringStateOutput stateOutput;
    InterceptorStateMachine interceptor{{.droneId = DroneId{7}, .arrivalToleranceMeters = 0.25},
                                        vehicle,
                                        vehicle,
                                        vehicle,
                                        stateOutput,
                                        eventPublisher};
    interceptor.start();
    ASSERT_EQ(interceptor.onAssignment(Assignment{DroneId{7}, TargetId{42}}),
              drone::interceptor::AssignmentHandlingResult::applied);
    ASSERT_EQ(interceptor.onTargetTrack(
                  TargetTrack{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{3'000ms}}),
              drone::interceptor::TargetTrackHandlingResult::accepted);
    ASSERT_EQ(interceptor.startInterception(
                  InterceptionCommand{InterceptionCommandId{101}, DroneId{7}, TargetId{42}}),
              drone::interceptor::InterceptionStartResult::started);

    ASSERT_EQ(interceptor.tick(100ms), InterceptionTickResult::effectSucceeded);
    ASSERT_EQ(interceptor.tick(100ms), InterceptionTickResult::notIntercepting);
    ASSERT_EQ(vehicle.effectCount, 1U);

    DomainParticipantOwner readerParticipant{explosionEventDomainId, "drone_step_42_late_probe"};
    WireExplosionEventReader reader{readerParticipant.participant()};
    const ExplosionEvent expected{ExplosionEventId{101}, DroneId{7}, TargetId{42},
                                  Position{10.0, 20.0, 30.0}, Timestamp{2'000ms}};

    EXPECT_EQ(reader.receive(dataTimeout), expected);
    EXPECT_FALSE(reader.receive(100ms).has_value());
}

} // namespace
