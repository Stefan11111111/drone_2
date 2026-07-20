#include "drone/observer_core/target_tracker.h"

#include "drone/domain/target_track.h"

namespace drone::observer
{

TargetTracker::TargetTracker(TargetTrackOutputPort &targetTrackOutput)
    : targetTrackOutput_{targetTrackOutput}
{
}

void TargetTracker::onDetection(const Detection &detection)
{
    const domain::TargetTrack targetTrack{detection.targetId, detection.position,
                                          detection.detectedAt};
    targetTrackOutput_.publish(targetTrack);
}

} // namespace drone::observer
