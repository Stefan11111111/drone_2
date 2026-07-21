#ifndef DRONE_DDS_TRANSPORT_ASSIGNMENT_TOPIC_H
#define DRONE_DDS_TRANSPORT_ASSIGNMENT_TOPIC_H

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

class AssignmentTopic final
{
  public:
    explicit AssignmentTopic(eprosima::fastdds::dds::DomainParticipant &participant);
    ~AssignmentTopic() noexcept;

    AssignmentTopic(const AssignmentTopic &) = delete;
    AssignmentTopic &operator=(const AssignmentTopic &) = delete;
    AssignmentTopic(AssignmentTopic &&) = delete;
    AssignmentTopic &operator=(AssignmentTopic &&) = delete;

    [[nodiscard]] eprosima::fastdds::dds::Topic &topic() noexcept;

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    eprosima::fastdds::dds::TypeSupport type_;
    eprosima::fastdds::dds::Topic *topic_{nullptr};
    bool typeRegistered_{false};
};

[[nodiscard]] eprosima::fastdds::dds::DataWriterQos assignmentWriterQos();
[[nodiscard]] eprosima::fastdds::dds::DataReaderQos assignmentReaderQos();

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_ASSIGNMENT_TOPIC_H
