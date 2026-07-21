#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_reader.h"
#include "drone/domain/target_track.h"
#include "process_test_support.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::TargetTrackReader;
using drone::domain::TargetTrack;
using drone::test::ChildProcess;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t observerProcessDomainId{184};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{2s};
constexpr auto processExitTimeout{2s};
constexpr std::size_t requiredUpdateCount{3};

[[nodiscard]] ::testing::AssertionResult
receiveObserverTracks(TargetTrackReader &reader, std::vector<TargetTrack> &receivedTracks)
{
    if (!reader.waitForWriterMatch(discoveryTimeout))
    {
        return ::testing::AssertionFailure()
               << "No observer TargetTrack DataWriter matched the probe within "
               << discoveryTimeout.count() << " ms";
    }

    receivedTracks.reserve(requiredUpdateCount);
    for (std::size_t index = 0; index < requiredUpdateCount; ++index)
    {
        if (!reader.waitForData(dataTimeout))
        {
            return ::testing::AssertionFailure()
                   << "Received only " << receivedTracks.size() << " of " << requiredUpdateCount
                   << " expected observer updates before a " << dataTimeout.count()
                   << " ms timeout";
        }

        const auto track = reader.take();
        if (!track.has_value())
        {
            return ::testing::AssertionFailure()
                   << "Observer process published a TargetTrack that failed domain mapping";
        }
        receivedTracks.push_back(*track);
    }

    return ::testing::AssertionSuccess();
}

[[nodiscard]] ::testing::AssertionResult
verifySuccessiveMovingTracks(const std::vector<TargetTrack> &receivedTracks)
{
    if (receivedTracks.size() != requiredUpdateCount)
    {
        return ::testing::AssertionFailure() << "Expected " << requiredUpdateCount
                                             << " tracks, received " << receivedTracks.size();
    }
    if (receivedTracks[0].targetId() != receivedTracks[1].targetId() ||
        receivedTracks[1].targetId() != receivedTracks[2].targetId())
    {
        return ::testing::AssertionFailure() << "Successive updates changed the target identifier";
    }
    if (receivedTracks[0].measuredAt() >= receivedTracks[1].measuredAt() ||
        receivedTracks[1].measuredAt() >= receivedTracks[2].measuredAt())
    {
        return ::testing::AssertionFailure()
               << "Successive updates did not have increasing measurement times";
    }
    if (receivedTracks[0].position() == receivedTracks[1].position() ||
        receivedTracks[1].position() == receivedTracks[2].position())
    {
        return ::testing::AssertionFailure() << "Successive updates did not move the target";
    }

    return ::testing::AssertionSuccess();
}

TEST(ObserverPublisher,
     GivenASeparateObserverProcess_WhenItsWriterMatches_ThenMultipleMovingTracksAreReceived)
{
    ChildProcess observer{
        OBSERVER_EXECUTABLE_PATH,
        {"--domain-id", std::to_string(observerProcessDomainId), "--tick-count", "100"}};
    DomainParticipantOwner probeParticipant{observerProcessDomainId, "drone_step_23_probe"};
    TargetTrackReader reader{probeParticipant.participant()};

    std::vector<TargetTrack> receivedTracks;
    ASSERT_TRUE(receiveObserverTracks(reader, receivedTracks));
    EXPECT_TRUE(verifySuccessiveMovingTracks(receivedTracks));
    EXPECT_TRUE(observer.terminateAndWait(processExitTimeout))
        << "The observer child process could not be cleaned up within "
        << processExitTimeout.count() << " ms";
}

} // namespace
