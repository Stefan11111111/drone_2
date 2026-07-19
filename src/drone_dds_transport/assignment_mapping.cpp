#include "drone/dds_transport/assignment_mapping.h"

#include "drone/domain/drone_id.h"
#include "drone/domain/target_id.h"

#include <expected>

namespace drone::dds_transport
{

dds::Assignment toWireAssignment(const domain::Assignment &assignment)
{
    dds::Assignment wireAssignment;
    wireAssignment.drone_id(assignment.droneId().value());
    wireAssignment.target_id(assignment.targetId().value());
    return wireAssignment;
}

std::expected<domain::Assignment, AssignmentMappingError>
fromWireAssignment(const dds::Assignment &assignment)
{
    if (assignment.drone_id() == 0)
    {
        return std::unexpected{AssignmentMappingError::zeroDroneId};
    }
    if (assignment.target_id() == 0)
    {
        return std::unexpected{AssignmentMappingError::zeroTargetId};
    }

    return domain::Assignment{domain::DroneId{assignment.drone_id()},
                              domain::TargetId{assignment.target_id()}};
}

} // namespace drone::dds_transport
