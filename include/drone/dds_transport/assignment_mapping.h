#ifndef DRONE_DDS_TRANSPORT_ASSIGNMENT_MAPPING_H
#define DRONE_DDS_TRANSPORT_ASSIGNMENT_MAPPING_H

#include "drone/domain/assignment.h"

#include <target_track.hpp>

#include <cstdint>
#include <expected>

namespace drone::dds_transport
{

enum class AssignmentMappingError : std::uint8_t
{
    zeroDroneId,
    zeroTargetId,
};

[[nodiscard]] dds::Assignment toWireAssignment(const domain::Assignment &assignment);

[[nodiscard]] std::expected<domain::Assignment, AssignmentMappingError>
fromWireAssignment(const dds::Assignment &assignment);

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_ASSIGNMENT_MAPPING_H
