#ifndef DRONE_OBSERVER_CORE_DETECTION_H
#define DRONE_OBSERVER_CORE_DETECTION_H

#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

namespace drone::observer
{

struct Detection final
{
    domain::TargetId targetId;
    domain::Position position;
    domain::Timestamp detectedAt;

    bool operator==(const Detection &) const = default;
};

} // namespace drone::observer

#endif // DRONE_OBSERVER_CORE_DETECTION_H
