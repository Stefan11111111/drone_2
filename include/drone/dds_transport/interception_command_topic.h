#ifndef DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_TOPIC_H
#define DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_TOPIC_H

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

class InterceptionCommandTopic final
{
  public:
    explicit InterceptionCommandTopic(eprosima::fastdds::dds::DomainParticipant &participant);
    ~InterceptionCommandTopic() noexcept;

    InterceptionCommandTopic(const InterceptionCommandTopic &) = delete;
    InterceptionCommandTopic &operator=(const InterceptionCommandTopic &) = delete;
    InterceptionCommandTopic(InterceptionCommandTopic &&) = delete;
    InterceptionCommandTopic &operator=(InterceptionCommandTopic &&) = delete;

    [[nodiscard]] eprosima::fastdds::dds::Topic &topic() noexcept;

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    eprosima::fastdds::dds::TypeSupport type_;
    eprosima::fastdds::dds::Topic *topic_{nullptr};
    bool typeRegistered_{false};
};

[[nodiscard]] eprosima::fastdds::dds::DataWriterQos interceptionCommandWriterQos();
[[nodiscard]] eprosima::fastdds::dds::DataReaderQos interceptionCommandReaderQos();

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_TOPIC_H
