#ifndef DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_WRITER_H
#define DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_WRITER_H

#include "drone/dds_transport/explosion_event_topic.h"
#include "drone/domain/explosion_event.h"

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

class ExplosionEventWriter final : private eprosima::fastdds::dds::DataWriterListener
{
  public:
    explicit ExplosionEventWriter(eprosima::fastdds::dds::DomainParticipant &participant);
    ~ExplosionEventWriter() noexcept override;

    ExplosionEventWriter(const ExplosionEventWriter &) = delete;
    ExplosionEventWriter &operator=(const ExplosionEventWriter &) = delete;
    ExplosionEventWriter(ExplosionEventWriter &&) = delete;
    ExplosionEventWriter &operator=(ExplosionEventWriter &&) = delete;

    [[nodiscard]] bool waitForReaderMatch(std::chrono::milliseconds timeout);
    void write(const domain::ExplosionEvent &event);

  private:
    void
    on_publication_matched(eprosima::fastdds::dds::DataWriter *writer,
                           const eprosima::fastdds::dds::PublicationMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    ExplosionEventTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    int currentMatchCount_{0};
    eprosima::fastdds::dds::Publisher *publisher_{nullptr};
    eprosima::fastdds::dds::DataWriter *writer_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_EXPLOSION_EVENT_WRITER_H
