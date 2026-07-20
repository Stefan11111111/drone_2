#ifndef DRONE_DDS_TRANSPORT_TARGET_TRACK_READER_H
#define DRONE_DDS_TRANSPORT_TARGET_TRACK_READER_H

#include "drone/dds_transport/target_track_mapping.h"
#include "drone/dds_transport/target_track_topic.h"
#include "drone/domain/target_track.h"

#include <chrono>
#include <expected>

namespace eprosima::fastdds::dds
{

class DataReader;
class DomainParticipant;
class Subscriber;

} // namespace eprosima::fastdds::dds

namespace drone::dds_transport
{

class TargetTrackReader final
{
  public:
    explicit TargetTrackReader(eprosima::fastdds::dds::DomainParticipant &participant);
    ~TargetTrackReader() noexcept;

    TargetTrackReader(const TargetTrackReader &) = delete;
    TargetTrackReader &operator=(const TargetTrackReader &) = delete;
    TargetTrackReader(TargetTrackReader &&) = delete;
    TargetTrackReader &operator=(TargetTrackReader &&) = delete;

    [[nodiscard]] bool waitForData(std::chrono::milliseconds timeout);
    [[nodiscard]] std::expected<domain::TargetTrack, TargetTrackMappingError> take();

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    TargetTrackTopic topic_;
    eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};
    eprosima::fastdds::dds::DataReader *reader_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_TARGET_TRACK_READER_H
