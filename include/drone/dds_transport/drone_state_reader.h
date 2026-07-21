#ifndef DRONE_DDS_TRANSPORT_DRONE_STATE_READER_H
#define DRONE_DDS_TRANSPORT_DRONE_STATE_READER_H

#include "drone/dds_transport/drone_state_mapping.h"
#include "drone/dds_transport/drone_state_topic.h"
#include "drone/domain/drone_state.h"

#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include <chrono>
#include <condition_variable>
#include <expected>
#include <mutex>
#include <optional>

namespace eprosima::fastdds::dds
{

class DataReader;
class DomainParticipant;
class Subscriber;

} // namespace eprosima::fastdds::dds

namespace drone::dds_transport
{

class DroneStateReader final : private eprosima::fastdds::dds::DataReaderListener
{
  public:
    explicit DroneStateReader(eprosima::fastdds::dds::DomainParticipant &participant);
    ~DroneStateReader() noexcept override;

    DroneStateReader(const DroneStateReader &) = delete;
    DroneStateReader &operator=(const DroneStateReader &) = delete;
    DroneStateReader(DroneStateReader &&) = delete;
    DroneStateReader &operator=(DroneStateReader &&) = delete;

    [[nodiscard]] bool waitForWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] bool waitForData(std::chrono::milliseconds timeout);
    [[nodiscard]]
    std::expected<std::optional<domain::DroneState>, DroneStateMappingError> takeNext();

  private:
    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader *reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    DroneStateTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    int currentMatchCount_{0};
    eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};
    eprosima::fastdds::dds::DataReader *reader_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_DRONE_STATE_READER_H
