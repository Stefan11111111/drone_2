#include "drone/dds_transport/interception_command_topic.h"

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

constexpr auto *interceptionCommandTopicName{"drone.interception_commands"};
constexpr std::int32_t maximumCommandInstances{256};
constexpr std::int32_t maximumCommandSamples{256};
constexpr std::int32_t maximumSamplesPerCommand{1};

template <typename EndpointQos> void applyInterceptionCommandQos(EndpointQos &qos)
{
    using namespace eprosima::fastdds::dds;

    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    qos.durability().kind = VOLATILE_DURABILITY_QOS;
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 1;
    qos.resource_limits().max_instances = maximumCommandInstances;
    qos.resource_limits().max_samples = maximumCommandSamples;
    qos.resource_limits().max_samples_per_instance = maximumSamplesPerCommand;
}

} // namespace

InterceptionCommandTopic::InterceptionCommandTopic(
    eprosima::fastdds::dds::DomainParticipant &participant)
    : participant_{participant}, type_{new dds::InterceptionCommandPubSubType}
{
    if (type_.register_type(&participant_) != eprosima::fastdds::dds::RETCODE_OK)
    {
        throw std::runtime_error{"Fast DDS could not register the InterceptionCommand wire type"};
    }
    typeRegistered_ = true;

    topic_ = participant_.create_topic(interceptionCommandTopicName, type_.get_type_name(),
                                       eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        static_cast<void>(participant_.unregister_type(type_.get_type_name()));
        typeRegistered_ = false;
        throw std::runtime_error{"Fast DDS could not create Topic 'drone.interception_commands'"};
    }
}

InterceptionCommandTopic::~InterceptionCommandTopic() noexcept
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

eprosima::fastdds::dds::Topic &InterceptionCommandTopic::topic() noexcept
{
    return *topic_;
}

eprosima::fastdds::dds::DataWriterQos interceptionCommandWriterQos()
{
    eprosima::fastdds::dds::DataWriterQos qos;
    applyInterceptionCommandQos(qos);
    return qos;
}

eprosima::fastdds::dds::DataReaderQos interceptionCommandReaderQos()
{
    eprosima::fastdds::dds::DataReaderQos qos;
    applyInterceptionCommandQos(qos);
    return qos;
}

} // namespace drone::dds_transport
