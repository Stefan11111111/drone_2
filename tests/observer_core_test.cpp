#include "drone/observer_core/target_tracker.h"

#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"
#include "drone/observer_core/detection.h"
#include "drone/observer_core/detection_input_port.h"
#include "drone/observer_core/target_track_output_port.h"

#include <chrono>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using drone::observer::Detection;
using drone::observer::DetectionInputPort;
using drone::observer::TargetTracker;
using drone::observer::TargetTrackOutputPort;
using namespace std::chrono_literals;

class CapturingTargetTrackOutput final : public TargetTrackOutputPort
{
  public:
    void publish(const TargetTrack &targetTrack) override
    {
        publishedTracks.push_back(targetTrack);
    }

    std::vector<TargetTrack> publishedTracks;
};

TEST(ObserverCore,
     GivenADetection_WhenSubmittedThroughTheInputPort_ThenTheExpectedTrackUpdateIsPublished)
{
    CapturingTargetTrackOutput output;
    TargetTracker tracker{output};
    DetectionInputPort &input = tracker;
    const Detection detection{.targetId = TargetId{42},
                              .position = Position{125.0, -30.5, 850.0},
                              .detectedAt = Timestamp{1'500ms}};

    input.onDetection(detection);

    ASSERT_EQ(output.publishedTracks.size(), 1U);
    EXPECT_EQ(output.publishedTracks.front(),
              (TargetTrack{detection.targetId, detection.position, detection.detectedAt}));
}

TEST(ObserverCore,
     GivenSuccessiveDetections_WhenSubmitted_ThenEachCurrentTrackStateIsPublishedInOrder)
{
    CapturingTargetTrackOutput output;
    TargetTracker tracker{output};
    const Detection first{.targetId = TargetId{42},
                          .position = Position{10.0, 20.0, 30.0},
                          .detectedAt = Timestamp{1'000ms}};
    const Detection second{.targetId = TargetId{42},
                           .position = Position{11.0, 22.0, 33.0},
                           .detectedAt = Timestamp{1'100ms}};

    tracker.onDetection(first);
    tracker.onDetection(second);

    ASSERT_EQ(output.publishedTracks.size(), 2U);
    EXPECT_EQ(output.publishedTracks[0],
              (TargetTrack{first.targetId, first.position, first.detectedAt}));
    EXPECT_EQ(output.publishedTracks[1],
              (TargetTrack{second.targetId, second.position, second.detectedAt}));
}

} // namespace
