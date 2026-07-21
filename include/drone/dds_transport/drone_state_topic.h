#ifndef DRONE_DDS_TRANSPORT_DRONE_STATE_TOPIC_H
#define DRONE_DDS_TRANSPORT_DRONE_STATE_TOPIC_H

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

class DroneStateTopic final
{
  public:
    explicit DroneStateTopic(eprosima::fastdds::dds::DomainParticipant &participant);
    ~DroneStateTopic() noexcept;

    DroneStateTopic(const DroneStateTopic &) = delete;
    DroneStateTopic &operator=(const DroneStateTopic &) = delete;
    DroneStateTopic(DroneStateTopic &&) = delete;
    DroneStateTopic &operator=(DroneStateTopic &&) = delete;

    [[nodiscard]] eprosima::fastdds::dds::Topic &topic() noexcept;

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    eprosima::fastdds::dds::TypeSupport type_;
    eprosima::fastdds::dds::Topic *topic_{nullptr};
    bool typeRegistered_{false};
};

[[nodiscard]] eprosima::fastdds::dds::DataWriterQos droneStateWriterQos();
[[nodiscard]] eprosima::fastdds::dds::DataReaderQos droneStateReaderQos();

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_DRONE_STATE_TOPIC_H
