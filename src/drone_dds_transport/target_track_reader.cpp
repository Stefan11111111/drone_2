#include "drone/dds_transport/target_track_reader.h"

#include "drone/dds_transport/target_track_mapping.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>

#include <target_track.hpp>

#include <chrono>
#include <expected>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>

namespace drone::dds_transport
{
namespace
{

[[nodiscard]] eprosima::fastdds::dds::Duration_t
toDdsDuration(const std::chrono::milliseconds timeout)
{
    if (timeout < std::chrono::milliseconds::zero())
    {
        throw std::invalid_argument{"A TargetTrack data wait timeout must not be negative"};
    }

    return eprosima::fastdds::dds::Duration_t{std::chrono::duration<long double>{timeout}.count()};
}

[[noreturn]] void throwTakeFailure(const eprosima::fastdds::dds::ReturnCode_t returnCode)
{
    if (returnCode == eprosima::fastdds::dds::RETCODE_NO_DATA)
    {
        throw std::runtime_error{"No unread TargetTrack sample is available to take"};
    }
    throw std::runtime_error{"Fast DDS could not take TargetTrack sample; return code " +
                             std::to_string(returnCode)};
}

} // namespace

TargetTrackReader::TargetTrackReader(eprosima::fastdds::dds::DomainParticipant &participant)
    : participant_{participant}, topic_{participant}
{
    subscriber_ = participant_.create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error{"Fast DDS could not create the TargetTrack Subscriber"};
    }

    reader_ = subscriber_->create_datareader(&topic_.topic(), targetTrackReaderQos(), this);
    if (reader_ == nullptr)
    {
        static_cast<void>(participant_.delete_subscriber(subscriber_));
        subscriber_ = nullptr;
        throw std::runtime_error{"Fast DDS could not create the TargetTrack DataReader"};
    }
}

TargetTrackReader::~TargetTrackReader() noexcept
{
    if (reader_ != nullptr)
    {
        static_cast<void>(subscriber_->delete_datareader(reader_));
    }
    if (subscriber_ != nullptr)
    {
        static_cast<void>(participant_.delete_subscriber(subscriber_));
    }
}

void TargetTrackReader::on_subscription_matched(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::SubscriptionMatchedStatus &status)
{
    static_cast<void>(reader);
    {
        const std::scoped_lock lock{discoveryMutex_};
        discoveryStatus_.totalMatchCount = status.total_count;
        discoveryStatus_.currentMatchCount = status.current_count;
    }
    discoveryChanged_.notify_all();
}

void TargetTrackReader::on_requested_incompatible_qos(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::RequestedIncompatibleQosStatus &status)
{
    static_cast<void>(reader);
    {
        const std::scoped_lock lock{discoveryMutex_};
        discoveryStatus_.incompatibleQosCount = status.total_count;
        discoveryStatus_.lastIncompatibleQosPolicy = status.last_policy_id;
    }
    discoveryChanged_.notify_all();
}

TargetTrackDiscoveryStatus TargetTrackReader::discoveryStatus() const
{
    const std::scoped_lock lock{discoveryMutex_};
    return discoveryStatus_;
}

bool TargetTrackReader::waitForWriterMatch(const std::chrono::milliseconds timeout)
{
    std::unique_lock lock{discoveryMutex_};
    return discoveryChanged_.wait_for(lock, timeout,
                                      [this] { return discoveryStatus_.currentMatchCount > 0; });
}

bool TargetTrackReader::waitForIncompatibleQos(const std::chrono::milliseconds timeout)
{
    std::unique_lock lock{discoveryMutex_};
    return discoveryChanged_.wait_for(lock, timeout,
                                      [this] { return discoveryStatus_.incompatibleQosCount > 0; });
}

bool TargetTrackReader::waitForData(const std::chrono::milliseconds timeout)
{
    return reader_->wait_for_unread_message(toDdsDuration(timeout));
}

std::expected<std::optional<domain::TargetTrack>, TargetTrackMappingError>
TargetTrackReader::takeNext()
{
    dds::TargetTrack wireTrack;
    eprosima::fastdds::dds::SampleInfo sampleInfo{};
    const auto returnCode = reader_->take_next_sample(&wireTrack, &sampleInfo);
    if (returnCode != eprosima::fastdds::dds::RETCODE_OK)
    {
        throwTakeFailure(returnCode);
    }
    if (!sampleInfo.valid_data)
    {
        return std::optional<domain::TargetTrack>{};
    }

    auto mappedTrack = fromWireTargetTrack(wireTrack);
    if (!mappedTrack.has_value())
    {
        return std::unexpected{mappedTrack.error()};
    }
    return std::optional<domain::TargetTrack>{*mappedTrack};
}

std::expected<domain::TargetTrack, TargetTrackMappingError> TargetTrackReader::take()
{
    auto sample = takeNext();
    if (!sample.has_value())
    {
        return std::unexpected{sample.error()};
    }
    if (!sample->has_value())
    {
        throw std::runtime_error{"Taken TargetTrack sample contains no valid data"};
    }
    return **sample;
}

} // namespace drone::dds_transport
