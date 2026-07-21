#ifndef DRONE_DDS_TRANSPORT_DRONE_STATE_WRITER_H
#define DRONE_DDS_TRANSPORT_DRONE_STATE_WRITER_H

#include "drone/dds_transport/drone_state_topic.h"
#include "drone/domain/drone_state.h"

#include <fastdds/dds/publisher/DataWriterListener.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace eprosima::fastdds::dds
{

class DataWriter;
class DomainParticipant;
class Publisher;

} // namespace eprosima::fastdds::dds

namespace drone::dds_transport
{

class DroneStateWriter final : private eprosima::fastdds::dds::DataWriterListener
{
  public:
    explicit DroneStateWriter(eprosima::fastdds::dds::DomainParticipant &participant);
    ~DroneStateWriter() noexcept override;

    DroneStateWriter(const DroneStateWriter &) = delete;
    DroneStateWriter &operator=(const DroneStateWriter &) = delete;
    DroneStateWriter(DroneStateWriter &&) = delete;
    DroneStateWriter &operator=(DroneStateWriter &&) = delete;

    [[nodiscard]] bool waitForReaderMatch(std::chrono::milliseconds timeout);
    void write(const domain::DroneState &state);

  private:
    void
    on_publication_matched(eprosima::fastdds::dds::DataWriter *writer,
                           const eprosima::fastdds::dds::PublicationMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    DroneStateTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    int currentMatchCount_{0};
    eprosima::fastdds::dds::Publisher *publisher_{nullptr};
    eprosima::fastdds::dds::DataWriter *writer_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_DRONE_STATE_WRITER_H
