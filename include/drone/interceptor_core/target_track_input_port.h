#ifndef DRONE_INTERCEPTOR_CORE_TARGET_TRACK_INPUT_PORT_H
#define DRONE_INTERCEPTOR_CORE_TARGET_TRACK_INPUT_PORT_H

#include "drone/domain/target_track.h"

#include <cstdint>

namespace drone::interceptor
{

enum class TargetTrackHandlingResult : std::uint8_t
{
    accepted,
    updated,
    unrelated,
    duplicate,
    stale,
    conflicting,
};

class TargetTrackInputPort
{
  public:
    virtual ~TargetTrackInputPort() = default;

    [[nodiscard]] virtual TargetTrackHandlingResult
    onTargetTrack(const domain::TargetTrack &track) = 0;
};

} // namespace drone::interceptor

#endif // DRONE_INTERCEPTOR_CORE_TARGET_TRACK_INPUT_PORT_H
