#ifndef DRONE_OBSERVER_CORE_TARGET_TRACK_OUTPUT_PORT_H
#define DRONE_OBSERVER_CORE_TARGET_TRACK_OUTPUT_PORT_H

#include "drone/domain/target_track.h"

namespace drone::observer
{

class TargetTrackOutputPort
{
  public:
    virtual ~TargetTrackOutputPort() = default;

    virtual void publish(const domain::TargetTrack &targetTrack) = 0;
};

} // namespace drone::observer

#endif // DRONE_OBSERVER_CORE_TARGET_TRACK_OUTPUT_PORT_H
