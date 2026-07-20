#ifndef DRONE_DDS_TRANSPORT_TARGET_TRACK_TOPIC_H
#define DRONE_DDS_TRANSPORT_TARGET_TRACK_TOPIC_H

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

class TargetTrackTopic final
{
  public:
    explicit TargetTrackTopic(eprosima::fastdds::dds::DomainParticipant &participant);
    ~TargetTrackTopic() noexcept;

    TargetTrackTopic(const TargetTrackTopic &) = delete;
    TargetTrackTopic &operator=(const TargetTrackTopic &) = delete;
    TargetTrackTopic(TargetTrackTopic &&) = delete;
    TargetTrackTopic &operator=(TargetTrackTopic &&) = delete;

    [[nodiscard]] eprosima::fastdds::dds::Topic &topic() noexcept;

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    eprosima::fastdds::dds::TypeSupport type_;
    eprosima::fastdds::dds::Topic *topic_{nullptr};
    bool typeRegistered_{false};
};

[[nodiscard]] eprosima::fastdds::dds::DataWriterQos targetTrackWriterQos();
[[nodiscard]] eprosima::fastdds::dds::DataReaderQos targetTrackReaderQos();

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_TARGET_TRACK_TOPIC_H
