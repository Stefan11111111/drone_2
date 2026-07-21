#include "drone/dds_transport/explosion_event_topic.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <target_trackPubSubTypes.hpp>

#include <cstdint>
#include <stdexcept>

namespace drone::dds_transport
{
namespace
{

constexpr auto *explosionEventTopicName{"drone.explosion_events"};
constexpr std::int32_t maximumEventInstances{256};
constexpr std::int32_t maximumEventSamples{256};
constexpr std::int32_t maximumSamplesPerEvent{1};

template <typename EndpointQos> void applyExplosionEventQos(EndpointQos &qos)
{
    using namespace eprosima::fastdds::dds;

    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 1;
    qos.resource_limits().max_instances = maximumEventInstances;
    qos.resource_limits().max_samples = maximumEventSamples;
    qos.resource_limits().max_samples_per_instance = maximumSamplesPerEvent;
}

} // namespace

ExplosionEventTopic::ExplosionEventTopic(eprosima::fastdds::dds::DomainParticipant &participant)
    : participant_{participant}, type_{new dds::ExplosionEventPubSubType}
{
    if (type_.register_type(&participant_) != eprosima::fastdds::dds::RETCODE_OK)
    {
        throw std::runtime_error{"Fast DDS could not register the ExplosionEvent wire type"};
    }
    typeRegistered_ = true;

    topic_ = participant_.create_topic(explosionEventTopicName, type_.get_type_name(),
                                       eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        static_cast<void>(participant_.unregister_type(type_.get_type_name()));
        typeRegistered_ = false;
        throw std::runtime_error{"Fast DDS could not create Topic 'drone.explosion_events'"};
    }
}

ExplosionEventTopic::~ExplosionEventTopic() noexcept
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

eprosima::fastdds::dds::Topic &ExplosionEventTopic::topic() noexcept
{
    return *topic_;
}

eprosima::fastdds::dds::DataWriterQos explosionEventWriterQos()
{
    eprosima::fastdds::dds::DataWriterQos qos;
    applyExplosionEventQos(qos);
    return qos;
}

eprosima::fastdds::dds::DataReaderQos explosionEventReaderQos()
{
    eprosima::fastdds::dds::DataReaderQos qos;
    applyExplosionEventQos(qos);
    return qos;
}

} // namespace drone::dds_transport
