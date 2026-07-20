#ifndef DRONE_OBSERVER_CORE_DETECTION_INPUT_PORT_H
#define DRONE_OBSERVER_CORE_DETECTION_INPUT_PORT_H

#include "drone/observer_core/detection.h"

namespace drone::observer
{

class DetectionInputPort
{
  public:
    virtual ~DetectionInputPort() = default;

    virtual void onDetection(const Detection &detection) = 0;
};

} // namespace drone::observer

#endif // DRONE_OBSERVER_CORE_DETECTION_INPUT_PORT_H
