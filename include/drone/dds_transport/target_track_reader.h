#ifndef DRONE_DDS_TRANSPORT_TARGET_TRACK_READER_H
#define DRONE_DDS_TRANSPORT_TARGET_TRACK_READER_H

#include "drone/dds_transport/target_track_discovery_status.h"
#include "drone/dds_transport/target_track_mapping.h"
#include "drone/dds_transport/target_track_topic.h"
#include "drone/domain/target_track.h"

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

class TargetTrackReader final : private eprosima::fastdds::dds::DataReaderListener
{
  public:
    explicit TargetTrackReader(eprosima::fastdds::dds::DomainParticipant &participant);
    ~TargetTrackReader() noexcept override;

    TargetTrackReader(const TargetTrackReader &) = delete;
    TargetTrackReader &operator=(const TargetTrackReader &) = delete;
    TargetTrackReader(TargetTrackReader &&) = delete;
    TargetTrackReader &operator=(TargetTrackReader &&) = delete;

    [[nodiscard]] TargetTrackDiscoveryStatus discoveryStatus() const;
    [[nodiscard]] bool waitForWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] bool waitForIncompatibleQos(std::chrono::milliseconds timeout);
    [[nodiscard]] bool waitForData(std::chrono::milliseconds timeout);
    [[nodiscard]]
    std::expected<std::optional<domain::TargetTrack>, TargetTrackMappingError> takeNext();
    [[nodiscard]] std::expected<domain::TargetTrack, TargetTrackMappingError> take();

  private:
    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader *reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus &status) override;
    void on_requested_incompatible_qos(
        eprosima::fastdds::dds::DataReader *reader,
        const eprosima::fastdds::dds::RequestedIncompatibleQosStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    TargetTrackTopic topic_;
    mutable std::mutex discoveryMutex_;
    std::condition_variable discoveryChanged_;
    TargetTrackDiscoveryStatus discoveryStatus_;
    eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};
    eprosima::fastdds::dds::DataReader *reader_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_TARGET_TRACK_READER_H
