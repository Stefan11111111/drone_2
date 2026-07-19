#include "drone/dds_transport/drone_state_mapping.h"

#include "drone/domain/drone_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

#include <chrono>
#include <cmath>
#include <expected>
#include <optional>
#include <utility>

namespace drone::dds_transport
{
namespace
{

[[nodiscard]] dds::DroneStatus toWireStatus(const domain::DroneStatus status)
{
    switch (status)
    {
    case domain::DroneStatus::available:
        return dds::DroneStatus::DRONE_AVAILABLE;
    case domain::DroneStatus::assigned:
        return dds::DroneStatus::DRONE_ASSIGNED;
    case domain::DroneStatus::intercepting:
        return dds::DroneStatus::DRONE_INTERCEPTING;
    case domain::DroneStatus::interceptionSucceeded:
        return dds::DroneStatus::DRONE_INTERCEPTION_SUCCEEDED;
    case domain::DroneStatus::interceptionFailed:
        return dds::DroneStatus::DRONE_INTERCEPTION_FAILED;
    }

    std::unreachable();
}

[[nodiscard]] std::expected<domain::DroneStatus, DroneStateMappingError>
fromWireStatus(const dds::DroneStatus status)
{
    switch (status)
    {
    case dds::DroneStatus::DRONE_AVAILABLE:
        return domain::DroneStatus::available;
    case dds::DroneStatus::DRONE_ASSIGNED:
        return domain::DroneStatus::assigned;
    case dds::DroneStatus::DRONE_INTERCEPTING:
        return domain::DroneStatus::intercepting;
    case dds::DroneStatus::DRONE_INTERCEPTION_SUCCEEDED:
        return domain::DroneStatus::interceptionSucceeded;
    case dds::DroneStatus::DRONE_INTERCEPTION_FAILED:
        return domain::DroneStatus::interceptionFailed;
    }

    return std::unexpected{DroneStateMappingError::unknownStatus};
}

[[nodiscard]] bool hasFiniteCoordinates(const dds::CartesianPosition &position)
{
    return std::isfinite(position.x()) && std::isfinite(position.y()) &&
           std::isfinite(position.z());
}

[[nodiscard]] bool requiresAssignedTarget(const domain::DroneStatus status)
{
    return status != domain::DroneStatus::available;
}

} // namespace

dds::DroneState toWireDroneState(const domain::DroneState &state)
{
    dds::CartesianPosition wirePosition;
    wirePosition.x(state.position().xMeters());
    wirePosition.y(state.position().yMeters());
    wirePosition.z(state.position().zMeters());

    dds::DroneState wireState;
    wireState.drone_id(state.droneId().value());
    wireState.position(std::move(wirePosition));
    wireState.reported_at_ms(state.reportedAt().timeSinceUnixEpoch().count());
    wireState.status(toWireStatus(state.status()));
    if (state.assignedTargetId().has_value())
    {
        wireState.assigned_target_id(state.assignedTargetId()->value());
    }
    return wireState;
}

std::expected<domain::DroneState, DroneStateMappingError>
fromWireDroneState(const dds::DroneState &state)
{
    if (state.drone_id() == 0)
    {
        return std::unexpected{DroneStateMappingError::zeroDroneId};
    }
    if (!hasFiniteCoordinates(state.position()))
    {
        return std::unexpected{DroneStateMappingError::nonFinitePosition};
    }
    if (state.reported_at_ms() < 0)
    {
        return std::unexpected{DroneStateMappingError::timestampBeforeUnixEpoch};
    }

    const auto status = fromWireStatus(state.status());
    if (!status.has_value())
    {
        return std::unexpected{status.error()};
    }

    std::optional<domain::TargetId> assignedTargetId;
    if (state.assigned_target_id().has_value())
    {
        if (state.assigned_target_id().value() == 0)
        {
            return std::unexpected{DroneStateMappingError::zeroAssignedTargetId};
        }
        assignedTargetId.emplace(state.assigned_target_id().value());
    }
    if (requiresAssignedTarget(*status) != assignedTargetId.has_value())
    {
        return std::unexpected{DroneStateMappingError::assignmentDoesNotMatchStatus};
    }

    return domain::DroneState{
        domain::DroneId{state.drone_id()},
        domain::Position{state.position().x(), state.position().y(), state.position().z()},
        domain::Timestamp{std::chrono::milliseconds{state.reported_at_ms()}},
        *status,
        assignedTargetId,
    };
}

} // namespace drone::dds_transport
