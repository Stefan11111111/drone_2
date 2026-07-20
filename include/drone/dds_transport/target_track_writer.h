#ifndef DRONE_DDS_TRANSPORT_TARGET_TRACK_WRITER_H
#define DRONE_DDS_TRANSPORT_TARGET_TRACK_WRITER_H

#include "drone/dds_transport/target_track_topic.h"
#include "drone/domain/target_track.h"

#include <fastdds/dds/publisher/DataWriterListener.hpp>

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace eprosima::fastdds::dds
{

class DataWriter;
class DomainParticipant;
class Publisher;

} // namespace eprosima::fastdds::dds

namespace drone::dds_transport
{

class TargetTrackWriter final : private eprosima::fastdds::dds::DataWriterListener
{
  public:
    explicit TargetTrackWriter(eprosima::fastdds::dds::DomainParticipant &participant);
    ~TargetTrackWriter() noexcept override;

    TargetTrackWriter(const TargetTrackWriter &) = delete;
    TargetTrackWriter &operator=(const TargetTrackWriter &) = delete;
    TargetTrackWriter(TargetTrackWriter &&) = delete;
    TargetTrackWriter &operator=(TargetTrackWriter &&) = delete;

    [[nodiscard]] bool waitForReaderMatch(std::chrono::milliseconds timeout);
    void write(const domain::TargetTrack &track);

  private:
    void
    on_publication_matched(eprosima::fastdds::dds::DataWriter *writer,
                           const eprosima::fastdds::dds::PublicationMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    TargetTrackTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    std::int32_t matchedReaderCount_{0};
    eprosima::fastdds::dds::Publisher *publisher_{nullptr};
    eprosima::fastdds::dds::DataWriter *writer_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_TARGET_TRACK_WRITER_H
