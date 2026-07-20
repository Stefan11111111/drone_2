#include "drone/dds_transport/target_track_topic.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <target_trackPubSubTypes.hpp>

#include <stdexcept>

namespace drone::dds_transport
{
namespace
{

constexpr auto *targetTrackTopicName{"drone.target_tracks"};
constexpr std::int32_t maximumTargetInstances{64};
constexpr std::int32_t maximumTargetSamples{64};
constexpr std::int32_t maximumSamplesPerTarget{1};

template <typename EndpointQos> void applyTargetTrackQos(EndpointQos &qos)
{
    using namespace eprosima::fastdds::dds;

    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 1;
    qos.resource_limits().max_instances = maximumTargetInstances;
    qos.resource_limits().max_samples = maximumTargetSamples;
    qos.resource_limits().max_samples_per_instance = maximumSamplesPerTarget;
}

} // namespace

TargetTrackTopic::TargetTrackTopic(eprosima::fastdds::dds::DomainParticipant &participant)
    : participant_{participant}, type_{new dds::TargetTrackPubSubType}
{
    if (type_.register_type(&participant_) != eprosima::fastdds::dds::RETCODE_OK)
    {
        throw std::runtime_error{"Fast DDS could not register the TargetTrack wire type"};
    }
    typeRegistered_ = true;

    topic_ = participant_.create_topic(targetTrackTopicName, type_.get_type_name(),
                                       eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        static_cast<void>(participant_.unregister_type(type_.get_type_name()));
        typeRegistered_ = false;
        throw std::runtime_error{"Fast DDS could not create Topic 'drone.target_tracks'"};
    }
}

TargetTrackTopic::~TargetTrackTopic() noexcept
{
    if (topic_ != nullptr)
    {
        static_cast<void>(participant_.delete_topic(topic_));
    }
    if (typeRegistered_)
    {
        static_cast<void>(participant_.unregister_type(type_.get_type_name()));
    }
}

eprosima::fastdds::dds::Topic &TargetTrackTopic::topic() noexcept
{
    return *topic_;
}

eprosima::fastdds::dds::DataWriterQos targetTrackWriterQos()
{
    eprosima::fastdds::dds::DataWriterQos qos;
    applyTargetTrackQos(qos);
    return qos;
}

eprosima::fastdds::dds::DataReaderQos targetTrackReaderQos()
{
    eprosima::fastdds::dds::DataReaderQos qos;
    applyTargetTrackQos(qos);
    return qos;
}

} // namespace drone::dds_transport
