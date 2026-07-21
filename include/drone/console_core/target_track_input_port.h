#ifndef DRONE_CONSOLE_CORE_TARGET_TRACK_INPUT_PORT_H
#define DRONE_CONSOLE_CORE_TARGET_TRACK_INPUT_PORT_H

#include "drone/domain/target_track.h"

#include <cstdint>

namespace drone::console
{

enum class TargetUpdateResult : std::uint8_t
{
    added,
    updated,
    duplicate,
    stale,
    conflicting,
};

class TargetTrackInputPort
{
  public:
    virtual ~TargetTrackInputPort() = default;

    virtual TargetUpdateResult onTargetTrack(const domain::TargetTrack &targetTrack) = 0;
};

} // namespace drone::console

#endif // DRONE_CONSOLE_CORE_TARGET_TRACK_INPUT_PORT_H
