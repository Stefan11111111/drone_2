#ifndef DRONE_DDS_TRANSPORT_TARGET_TRACK_DISCOVERY_STATUS_H
#define DRONE_DDS_TRANSPORT_TARGET_TRACK_DISCOVERY_STATUS_H

#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include <cstdint>

namespace drone::dds_transport
{

struct TargetTrackDiscoveryStatus
{
    std::int32_t totalMatchCount{0};
    std::int32_t currentMatchCount{0};
    std::uint32_t incompatibleQosCount{0};
    eprosima::fastdds::dds::QosPolicyId_t lastIncompatibleQosPolicy{
        eprosima::fastdds::dds::INVALID_QOS_POLICY_ID};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_TARGET_TRACK_DISCOVERY_STATUS_H
