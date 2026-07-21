#ifndef DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_TOPIC_H
#define DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_TOPIC_H

#include <fastdds/dds/topic/TypeSupport.hpp>

namespace eprosima::fastdds::dds
{

class DataReaderQos;
class DataWriterQos;
class DomainParticipant;
class Topic;

} // namespace eprosima::fastdds::dds

namespace drone::dds_transport
{

class ExplosionEventTopic final
{
  public:
    explicit ExplosionEventTopic(eprosima::fastdds::dds::DomainParticipant &participant);
    ~ExplosionEventTopic() noexcept;

    ExplosionEventTopic(const ExplosionEventTopic &) = delete;
    ExplosionEventTopic &operator=(const ExplosionEventTopic &) = delete;
    ExplosionEventTopic(ExplosionEventTopic &&) = delete;
    ExplosionEventTopic &operator=(ExplosionEventTopic &&) = delete;

    [[nodiscard]] eprosima::fastdds::dds::Topic &topic() noexcept;

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    eprosima::fastdds::dds::TypeSupport type_;
    eprosima::fastdds::dds::Topic *topic_{nullptr};
    bool typeRegistered_{false};
};

[[nodiscard]] eprosima::fastdds::dds::DataWriterQos explosionEventWriterQos();
[[nodiscard]] eprosima::fastdds::dds::DataReaderQos explosionEventReaderQos();

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_TOPIC_H
