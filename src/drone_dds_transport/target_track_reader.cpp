#include "drone/dds_transport/target_track_reader.h"

#include "drone/dds_transport/target_track_mapping.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>

#include <target_track.hpp>

#include <chrono>
#include <expected>
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

    reader_ = subscriber_->create_datareader(&topic_.topic(), targetTrackReaderQos());
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

bool TargetTrackReader::waitForData(const std::chrono::milliseconds timeout)
{
    return reader_->wait_for_unread_message(toDdsDuration(timeout));
}

std::expected<domain::TargetTrack, TargetTrackMappingError> TargetTrackReader::take()
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
        throw std::runtime_error{"Taken TargetTrack sample contains no valid data"};
    }

    return fromWireTargetTrack(wireTrack);
}

} // namespace drone::dds_transport
