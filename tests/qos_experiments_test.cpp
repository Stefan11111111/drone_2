#include "drone/dds_transport/assignment_reader.h"
#include "drone/dds_transport/assignment_topic.h"
#include "drone/dds_transport/assignment_writer.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/drone_state_topic.h"
#include "drone/dds_transport/explosion_event_topic.h"
#include "drone/dds_transport/interception_command_topic.h"
#include "drone/dds_transport/target_track_reader.h"
#include "drone/dds_transport/target_track_topic.h"
#include "drone/dds_transport/target_track_writer.h"
#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::AssignmentReader;
using drone::dds_transport::assignmentReaderQos;
using drone::dds_transport::AssignmentWriter;
using drone::dds_transport::assignmentWriterQos;
using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::droneStateReaderQos;
using drone::dds_transport::droneStateWriterQos;
using drone::dds_transport::explosionEventReaderQos;
using drone::dds_transport::explosionEventWriterQos;
using drone::dds_transport::interceptionCommandReaderQos;
using drone::dds_transport::interceptionCommandWriterQos;
using drone::dds_transport::TargetTrackReader;
using drone::dds_transport::targetTrackReaderQos;
using drone::dds_transport::TargetTrackWriter;
using drone::dds_transport::targetTrackWriterQos;
using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t retainedHistoryDomainId{201};
constexpr DomainId_t volatileHistoryDomainId{202};
constexpr DomainId_t incompatibleDurabilityDomainId{203};
constexpr DomainId_t resourceLimitDomainId{204};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};
constexpr auto noDataTimeout{250ms};

template <typename EndpointQos, typename DurabilityKind>
void expectCatalogQos(const EndpointQos &qos, const DurabilityKind durability,
                      const std::int32_t maximumInstances)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_EQ(qos.reliability().kind, RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(qos.durability().kind, durability);
    EXPECT_EQ(qos.history().kind, KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(qos.history().depth, 1);
    EXPECT_EQ(qos.resource_limits().max_instances, maximumInstances);
    EXPECT_EQ(qos.resource_limits().max_samples, maximumInstances);
    EXPECT_EQ(qos.resource_limits().max_samples_per_instance, 1);
}

TEST(QosCatalogAudit,
     GivenFinalTopicCatalog_WhenEndpointQosIsBuilt_ThenEveryWriterAndReaderMatchesIt)
{
    using namespace eprosima::fastdds::dds;

    SCOPED_TRACE("target track writer");
    expectCatalogQos(targetTrackWriterQos(), TRANSIENT_LOCAL_DURABILITY_QOS, 64);
    SCOPED_TRACE("target track reader");
    expectCatalogQos(targetTrackReaderQos(), TRANSIENT_LOCAL_DURABILITY_QOS, 64);

    SCOPED_TRACE("drone state writer");
    expectCatalogQos(droneStateWriterQos(), TRANSIENT_LOCAL_DURABILITY_QOS, 16);
    SCOPED_TRACE("drone state reader");
    expectCatalogQos(droneStateReaderQos(), TRANSIENT_LOCAL_DURABILITY_QOS, 16);

    SCOPED_TRACE("assignment writer");
    expectCatalogQos(assignmentWriterQos(), VOLATILE_DURABILITY_QOS, 16);
    SCOPED_TRACE("assignment reader");
    expectCatalogQos(assignmentReaderQos(), VOLATILE_DURABILITY_QOS, 16);

    SCOPED_TRACE("interception command writer");
    expectCatalogQos(interceptionCommandWriterQos(), VOLATILE_DURABILITY_QOS, 256);
    SCOPED_TRACE("interception command reader");
    expectCatalogQos(interceptionCommandReaderQos(), VOLATILE_DURABILITY_QOS, 256);

    SCOPED_TRACE("explosion event writer");
    expectCatalogQos(explosionEventWriterQos(), TRANSIENT_LOCAL_DURABILITY_QOS, 256);
    SCOPED_TRACE("explosion event reader");
    expectCatalogQos(explosionEventReaderQos(), TRANSIENT_LOCAL_DURABILITY_QOS, 256);
}

TEST(QosExperiments,
     GivenTransientLocalKeepLastHistory_WhenAReaderJoinsLate_ThenOnlyTheLatestSamplePerKeyArrives)
{
    DomainParticipantOwner writerParticipant{retainedHistoryDomainId,
                                             "drone_step_46_history_writer"};
    TargetTrackWriter writer{writerParticipant.participant()};
    const TargetTrack superseded{TargetId{1}, Position{1.0, 0.0, 100.0}, Timestamp{100ms}};
    const TargetTrack latestForFirstTarget{TargetId{1}, Position{2.0, 0.0, 100.0},
                                           Timestamp{200ms}};
    const TargetTrack onlyForSecondTarget{TargetId{2}, Position{9.0, 0.0, 100.0}, Timestamp{300ms}};
    writer.write(superseded);
    writer.write(latestForFirstTarget);
    writer.write(onlyForSecondTarget);

    DomainParticipantOwner readerParticipant{retainedHistoryDomainId,
                                             "drone_step_46_history_late_reader"};
    TargetTrackReader reader{readerParticipant.participant()};
    ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout))
        << "The transient-local writer did not match the late reader within "
        << discoveryTimeout.count() << " ms";

    std::vector<TargetTrack> received;
    for (std::size_t sampleIndex{0}; sampleIndex < 2; ++sampleIndex)
    {
        ASSERT_TRUE(reader.waitForData(dataTimeout))
            << "Expected two retained keyed samples, but sample " << sampleIndex + 1
            << " did not arrive within " << dataTimeout.count() << " ms";
        const auto sample = reader.take();
        ASSERT_TRUE(sample.has_value()) << "A retained TargetTrack failed domain validation";
        received.push_back(*sample);
    }

    EXPECT_EQ(received.size(), 2U);
    EXPECT_TRUE(std::ranges::contains(received, latestForFirstTarget));
    EXPECT_TRUE(std::ranges::contains(received, onlyForSecondTarget));
    EXPECT_FALSE(std::ranges::contains(received, superseded));
    EXPECT_FALSE(reader.waitForData(noDataTimeout))
        << "KEEP_LAST(1) retained more than one sample for a target key";
}

TEST(
    QosExperiments,
    GivenVolatileAssignmentHistory_WhenAReaderJoinsLate_ThenOldIntentIsNotReplayedButNewIntentArrives)
{
    DomainParticipantOwner writerParticipant{volatileHistoryDomainId,
                                             "drone_step_46_volatile_writer"};
    AssignmentWriter writer{writerParticipant.participant()};
    writer.write(Assignment{DroneId{1}, TargetId{10}});

    DomainParticipantOwner readerParticipant{volatileHistoryDomainId,
                                             "drone_step_46_volatile_late_reader"};
    AssignmentReader reader{readerParticipant.participant()};
    ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout))
        << "The volatile Assignment endpoints did not match within " << discoveryTimeout.count()
        << " ms";
    EXPECT_FALSE(reader.waitForData(noDataTimeout))
        << "A volatile reader received operator intent written before it matched";

    const Assignment fresh{DroneId{1}, TargetId{11}};
    writer.write(fresh);
    ASSERT_TRUE(reader.waitForData(dataTimeout))
        << "A fresh Assignment did not arrive within " << dataTimeout.count() << " ms";
    const auto received = reader.takeNext();
    ASSERT_TRUE(received.has_value()) << "The fresh Assignment failed wire validation";
    ASSERT_TRUE(received->has_value()) << "The fresh Assignment sample contained no valid data";
    EXPECT_EQ(**received, fresh);
}

TEST(
    QosExperiments,
    GivenVolatileWriterAndTransientLocalReader_WhenDiscovered_ThenBothReportDurabilityIncompatibility)
{
    DomainParticipantOwner writerParticipant{incompatibleDurabilityDomainId,
                                             "drone_step_46_volatile_target_writer"};
    DomainParticipantOwner readerParticipant{incompatibleDurabilityDomainId,
                                             "drone_step_46_transient_target_reader"};
    TargetTrackReader reader{readerParticipant.participant()};
    auto writerQos = targetTrackWriterQos();
    writerQos.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    TargetTrackWriter writer{writerParticipant.participant(), writerQos};

    ASSERT_TRUE(writer.waitForIncompatibleQos(discoveryTimeout))
        << "The writer did not report offered Durability incompatibility within "
        << discoveryTimeout.count() << " ms";
    ASSERT_TRUE(reader.waitForIncompatibleQos(discoveryTimeout))
        << "The reader did not report requested Durability incompatibility within "
        << discoveryTimeout.count() << " ms";

    const auto writerStatus = writer.discoveryStatus();
    const auto readerStatus = reader.discoveryStatus();
    EXPECT_EQ(writerStatus.currentMatchCount, 0);
    EXPECT_EQ(readerStatus.currentMatchCount, 0);
    EXPECT_EQ(writerStatus.lastIncompatibleQosPolicy,
              eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
    EXPECT_EQ(readerStatus.lastIncompatibleQosPolicy,
              eprosima::fastdds::dds::DURABILITY_QOS_POLICY_ID);
}

TEST(QosExperiments,
     GivenTargetWriterAtCatalogCapacity_WhenANewKeyIsWritten_ThenTheResourceErrorIsVisible)
{
    DomainParticipantOwner writerParticipant{resourceLimitDomainId,
                                             "drone_step_46_resource_writer"};
    TargetTrackWriter writer{writerParticipant.participant()};
    for (std::uint64_t targetValue{1}; targetValue <= 64; ++targetValue)
    {
        writer.write(TargetTrack{
            TargetId{targetValue}, Position{static_cast<double>(targetValue), 0.0, 100.0},
            Timestamp{std::chrono::milliseconds{static_cast<std::int64_t>(targetValue)}}});
    }

    try
    {
        writer.write(TargetTrack{TargetId{65}, Position{65.0, 0.0, 100.0}, Timestamp{65ms}});
        FAIL() << "The 65th target instance exceeded the catalog limit without an error";
    }
    catch (const std::runtime_error &error)
    {
        EXPECT_NE(std::string_view{error.what()}.find("return code"), std::string_view::npos)
            << "The capacity failure did not identify the Fast DDS write error: " << error.what();
    }
}

} // namespace
