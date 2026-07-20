#ifndef DRONE_OBSERVER_CORE_TARGET_TRACKER_H
#define DRONE_OBSERVER_CORE_TARGET_TRACKER_H

#include "drone/observer_core/detection_input_port.h"
#include "drone/observer_core/target_track_output_port.h"

namespace drone::observer
{

class TargetTracker final : public DetectionInputPort
{
  public:
    explicit TargetTracker(TargetTrackOutputPort &targetTrackOutput);

    void onDetection(const Detection &detection) override;

  private:
    TargetTrackOutputPort &targetTrackOutput_;
};

} // namespace drone::observer

#endif // DRONE_OBSERVER_CORE_TARGET_TRACKER_H
