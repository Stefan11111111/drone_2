#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_reader.h"
#include "drone/dds_transport/target_track_topic.h"
#include "drone/dds_transport/target_track_writer.h"

#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <chrono>
#include <iostream>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::targetTrackBestEffortWriterQosForDiscoveryExperiment;
using drone::dds_transport::TargetTrackReader;
using drone::dds_transport::TargetTrackWriter;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t compatibleDiscoveryDomainId{182};
constexpr DomainId_t incompatibleDiscoveryDomainId{183};
constexpr auto discoveryTimeout{5s};

TEST(TargetTrackDiscovery,
     GivenCompatibleTopicTypeAndQos_WhenEndpointsAreDiscovered_ThenBothReportAMatch)
{
    DomainParticipantOwner writerParticipant{compatibleDiscoveryDomainId,
                                             "drone_step_20_compatible_writer"};
    DomainParticipantOwner readerParticipant{compatibleDiscoveryDomainId,
                                             "drone_step_20_compatible_reader"};
    TargetTrackReader reader{readerParticipant.participant()};
    TargetTrackWriter writer{writerParticipant.participant()};

    ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout))
        << "No publication match callback arrived within " << discoveryTimeout.count() << " ms";
    ASSERT_TRUE(reader.waitForWriterMatch(discoveryTimeout))
        << "No subscription match callback arrived within " << discoveryTimeout.count() << " ms";

    const auto writerStatus = writer.discoveryStatus();
    const auto readerStatus = reader.discoveryStatus();
    EXPECT_EQ(writerStatus.currentMatchCount, 1);
    EXPECT_EQ(readerStatus.currentMatchCount, 1);
    EXPECT_EQ(writerStatus.incompatibleQosCount, 0U);
    EXPECT_EQ(readerStatus.incompatibleQosCount, 0U);

    std::cout << "MATCH: writer readers=" << writerStatus.currentMatchCount
              << ", reader writers=" << readerStatus.currentMatchCount
              << ", incompatible QoS callbacks=0\n";
}

TEST(
    TargetTrackDiscovery,
    GivenBestEffortWriterAndReliableReader_WhenEndpointsAreDiscovered_ThenBothReportReliabilityIncompatibility)
{
    DomainParticipantOwner writerParticipant{incompatibleDiscoveryDomainId,
                                             "drone_step_20_incompatible_writer"};
    DomainParticipantOwner readerParticipant{incompatibleDiscoveryDomainId,
                                             "drone_step_20_incompatible_reader"};
    TargetTrackReader reader{readerParticipant.participant()};
    TargetTrackWriter writer{writerParticipant.participant(),
                             targetTrackBestEffortWriterQosForDiscoveryExperiment()};

    ASSERT_TRUE(writer.waitForIncompatibleQos(discoveryTimeout))
        << "No offered-incompatible-QoS callback arrived within " << discoveryTimeout.count()
        << " ms";
    ASSERT_TRUE(reader.waitForIncompatibleQos(discoveryTimeout))
        << "No requested-incompatible-QoS callback arrived within " << discoveryTimeout.count()
        << " ms";

    const auto writerStatus = writer.discoveryStatus();
    const auto readerStatus = reader.discoveryStatus();
    EXPECT_EQ(writerStatus.currentMatchCount, 0);
    EXPECT_EQ(readerStatus.currentMatchCount, 0);
    EXPECT_GE(writerStatus.incompatibleQosCount, 1U);
    EXPECT_GE(readerStatus.incompatibleQosCount, 1U);
    EXPECT_EQ(writerStatus.lastIncompatibleQosPolicy,
              eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);
    EXPECT_EQ(readerStatus.lastIncompatibleQosPolicy,
              eprosima::fastdds::dds::RELIABILITY_QOS_POLICY_ID);

    std::cout << "NON-MATCH: writer readers=0, reader writers=0, last incompatible policy="
                 "RELIABILITY\n";
}

} // namespace
