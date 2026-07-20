#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_reader.h"
#include "drone/dds_transport/target_track_writer.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::TargetTrackReader;
using drone::dds_transport::TargetTrackWriter;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t targetTrackRoundTripDomainId{181};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};

TEST(TargetTrackPublishSubscribe,
     GivenTwoParticipants_WhenOneTrackIsWrittenAndTaken_ThenTheDomainValueRoundTrips)
{
    DomainParticipantOwner writerParticipant{targetTrackRoundTripDomainId, "drone_step_19_writer"};
    DomainParticipantOwner readerParticipant{targetTrackRoundTripDomainId, "drone_step_19_reader"};
    TargetTrackReader reader{readerParticipant.participant()};
    TargetTrackWriter writer{writerParticipant.participant()};

    ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout))
        << "Timed out after " << discoveryTimeout.count()
        << " ms waiting for the TargetTrack DataWriter and DataReader to match";

    const TargetTrack sent{TargetId{42}, Position{125.25, -30.5, 850.75}, Timestamp{1'500ms}};
    writer.write(sent);

    ASSERT_TRUE(reader.waitForData(dataTimeout))
        << "Timed out after " << dataTimeout.count()
        << " ms waiting for an unread TargetTrack sample after a successful write";
    const auto received = reader.take();

    ASSERT_TRUE(received.has_value()) << "The received wire sample failed domain validation";
    EXPECT_EQ(*received, sent);
}

} // namespace
