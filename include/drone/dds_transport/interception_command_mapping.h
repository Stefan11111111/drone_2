#ifndef DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_MAPPING_H
#define DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_MAPPING_H

#include "drone/domain/interception_command.h"

#include <target_track.hpp>

#include <cstdint>
#include <expected>

namespace drone::dds_transport
{

enum class InterceptionCommandMappingError : std::uint8_t
{
    zeroCommandId,
    zeroDroneId,
    zeroTargetId,
};

[[nodiscard]] dds::InterceptionCommand
toWireInterceptionCommand(const domain::InterceptionCommand &command);

[[nodiscard]] std::expected<domain::InterceptionCommand, InterceptionCommandMappingError>
fromWireInterceptionCommand(const dds::InterceptionCommand &command);

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_MAPPING_H
