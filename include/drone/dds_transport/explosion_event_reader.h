#ifndef DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_READER_H
#define DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_READER_H

#include "drone/dds_transport/explosion_event_mapping.h"
#include "drone/dds_transport/explosion_event_topic.h"
#include "drone/domain/explosion_event.h"

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

class ExplosionEventReader final : private eprosima::fastdds::dds::DataReaderListener
{
  public:
    explicit ExplosionEventReader(eprosima::fastdds::dds::DomainParticipant &participant);
    ~ExplosionEventReader() noexcept override;

    ExplosionEventReader(const ExplosionEventReader &) = delete;
    ExplosionEventReader &operator=(const ExplosionEventReader &) = delete;
    ExplosionEventReader(ExplosionEventReader &&) = delete;
    ExplosionEventReader &operator=(ExplosionEventReader &&) = delete;

    [[nodiscard]] bool waitForWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] bool waitForData(std::chrono::milliseconds timeout);
    [[nodiscard]]
    std::expected<std::optional<domain::ExplosionEvent>, ExplosionEventMappingError> takeNext();

  private:
    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader *reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    ExplosionEventTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    int currentMatchCount_{0};
    eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};
    eprosima::fastdds::dds::DataReader *reader_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_READER_H
