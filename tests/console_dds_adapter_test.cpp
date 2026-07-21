#include "drone/console_core/target_projection.h"
#include "drone/console_dds_adapter/target_track_subscriber.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_mapping.h"
#include "drone/dds_transport/target_track_topic.h"
#include "drone/dds_transport/target_track_writer.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

#include <target_track.hpp>

#include <chrono>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

namespace
{

using drone::console::TargetProjection;
using drone::console::TargetUpdateResult;
using drone::console_dds::TargetTrackReceiveIssue;
using drone::console_dds::TargetTrackSubscriber;
using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::TargetTrackTopic;
using drone::dds_transport::TargetTrackWriter;
using drone::dds_transport::targetTrackWriterQos;
using drone::dds_transport::toWireTargetTrack;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t validDeliveryDomainId{185};
constexpr DomainId_t invalidDeliveryDomainId{186};
constexpr DomainId_t boundedShutdownDomainId{187};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};

class WireTargetTrackWriter final
{
  public:
    explicit WireTargetTrackWriter(eprosima::fastdds::dds::DomainParticipant &participant)
        : participant_{participant}, topic_{participant}
    {
        publisher_ = participant_.create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
        if (publisher_ == nullptr)
        {
            throw std::runtime_error{"Could not create the test TargetTrack Publisher"};
        }

        writer_ = publisher_->create_datawriter(&topic_.topic(), targetTrackWriterQos());
        if (writer_ == nullptr)
        {
            static_cast<void>(participant_.delete_publisher(publisher_));
            publisher_ = nullptr;
            throw std::runtime_error{"Could not create the test TargetTrack DataWriter"};
        }
    }

    ~WireTargetTrackWriter() noexcept
    {
        if (writer_ != nullptr)
        {
            static_cast<void>(publisher_->delete_datawriter(writer_));
        }
        if (publisher_ != nullptr)
        {
            static_cast<void>(participant_.delete_publisher(publisher_));
        }
    }

    WireTargetTrackWriter(const WireTargetTrackWriter &) = delete;
    WireTargetTrackWriter &operator=(const WireTargetTrackWriter &) = delete;
    WireTargetTrackWriter(WireTargetTrackWriter &&) = delete;
    WireTargetTrackWriter &operator=(WireTargetTrackWriter &&) = delete;

    void write(const drone::dds::TargetTrack &sample)
    {
        const auto returnCode = writer_->write(&sample);
        if (returnCode != eprosima::fastdds::dds::RETCODE_OK)
        {
            throw std::runtime_error{"Could not write the test TargetTrack; return code " +
                                     std::to_string(returnCode)};
        }
    }

    void dispose(const drone::dds::TargetTrack &sample)
    {
        const auto returnCode = writer_->dispose(&sample, eprosima::fastdds::dds::HANDLE_NIL);
        if (returnCode != eprosima::fastdds::dds::RETCODE_OK)
        {
            throw std::runtime_error{"Could not dispose the test TargetTrack; return code " +
                                     std::to_string(returnCode)};
        }
    }

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    TargetTrackTopic topic_;
    eprosima::fastdds::dds::Publisher *publisher_{nullptr};
    eprosima::fastdds::dds::DataWriter *writer_{nullptr};
};

TEST(ConsoleDdsAdapter,
     GivenAMatchedObserverWriter_WhenATrackArrives_ThenItIsDeliveredIntoTheConsoleProjection)
{
    TargetProjection projection;
    TargetTrackSubscriber subscriber{validDeliveryDomainId, "drone_step_25_console", projection};
    DomainParticipantOwner writerParticipant{validDeliveryDomainId, "drone_step_25_observer"};
    TargetTrackWriter writer{writerParticipant.participant()};
    ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout))
        << "No console TargetTrack DataReader matched within " << discoveryTimeout.count() << " ms";

    const TargetTrack sent{TargetId{42}, Position{125.25, -30.5, 850.75}, Timestamp{1'500ms}};
    writer.write(sent);

    const auto result = subscriber.receiveNext(dataTimeout);
    ASSERT_TRUE(result.has_value())
        << "No valid TargetTrack reached console core within " << dataTimeout.count() << " ms";
    EXPECT_EQ(*result, TargetUpdateResult::added);
    EXPECT_EQ(projection.latestTarget(TargetId{42}), sent);
}

TEST(ConsoleDdsAdapter,
     GivenMalformedAndInstanceStateSamples_WhenTheyAreTaken_ThenTheyAreDiscardedSafely)
{
    TargetProjection projection;
    TargetTrackSubscriber subscriber{invalidDeliveryDomainId, "drone_step_25_invalid_console",
                                     projection};
    DomainParticipantOwner writerParticipant{invalidDeliveryDomainId, "drone_step_25_wire_writer"};
    WireTargetTrackWriter writer{writerParticipant.participant()};
    ASSERT_TRUE(subscriber.waitForWriterMatch(discoveryTimeout))
        << "No test TargetTrack DataWriter matched within " << discoveryTimeout.count() << " ms";

    drone::dds::TargetTrack malformed;
    malformed.target_id(0);
    writer.write(malformed);

    const auto malformedResult = subscriber.receiveNext(dataTimeout);
    ASSERT_FALSE(malformedResult.has_value());
    EXPECT_EQ(malformedResult.error(), TargetTrackReceiveIssue::discardedMalformedData);
    EXPECT_TRUE(projection.targetTracks().empty());

    const TargetTrack valid{TargetId{7}, Position{1.0, 2.0, 3.0}, Timestamp{2'000ms}};
    const auto validWireSample = toWireTargetTrack(valid);
    writer.write(validWireSample);
    const auto validResult = subscriber.receiveNext(dataTimeout);
    ASSERT_TRUE(validResult.has_value());
    EXPECT_EQ(*validResult, TargetUpdateResult::added);
    EXPECT_EQ(projection.latestTarget(TargetId{7}), valid);

    writer.dispose(validWireSample);
    const auto invalidDataResult = subscriber.receiveNext(dataTimeout);
    ASSERT_FALSE(invalidDataResult.has_value());
    EXPECT_EQ(invalidDataResult.error(), TargetTrackReceiveIssue::discardedInvalidData);
    EXPECT_EQ(projection.latestTarget(TargetId{7}), valid);
}

TEST(ConsoleDdsAdapter,
     GivenNoAvailableSample_WhenTheBoundedReceiveExpires_ThenTheAdapterCanShutDownCleanly)
{
    TargetProjection projection;
    TargetTrackSubscriber subscriber{boundedShutdownDomainId, "drone_step_25_bounded_console",
                                     projection};

    const auto result = subscriber.receiveNext(10ms);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), TargetTrackReceiveIssue::timedOut);
    EXPECT_TRUE(projection.targetTracks().empty());
}

} // namespace
