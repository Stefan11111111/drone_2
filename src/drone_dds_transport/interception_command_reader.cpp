#include "drone/dds_transport/interception_command_reader.h"

#include "drone/dds_transport/interception_command_mapping.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Time_t.hpp>
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
        throw std::invalid_argument{
            "An InterceptionCommand data wait timeout must not be negative"};
    }
    return eprosima::fastdds::dds::Duration_t{std::chrono::duration<long double>{timeout}.count()};
}

} // namespace

InterceptionCommandReader::InterceptionCommandReader(
    eprosima::fastdds::dds::DomainParticipant &participant)
    : participant_{participant}, topic_{participant}
{
    subscriber_ = participant_.create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error{"Fast DDS could not create the InterceptionCommand Subscriber"};
    }

    reader_ = subscriber_->create_datareader(&topic_.topic(), interceptionCommandReaderQos(), this);
    if (reader_ == nullptr)
    {
        static_cast<void>(participant_.delete_subscriber(subscriber_));
        subscriber_ = nullptr;
        throw std::runtime_error{"Fast DDS could not create the InterceptionCommand DataReader"};
    }
}

InterceptionCommandReader::~InterceptionCommandReader() noexcept
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

void InterceptionCommandReader::on_subscription_matched(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::SubscriptionMatchedStatus &status)
{
    static_cast<void>(reader);
    {
        const std::scoped_lock lock{matchMutex_};
        currentMatchCount_ = status.current_count;
    }
    matchChanged_.notify_all();
}

bool InterceptionCommandReader::waitForWriterMatch(const std::chrono::milliseconds timeout)
{
    std::unique_lock lock{matchMutex_};
    return matchChanged_.wait_for(lock, timeout, [this] { return currentMatchCount_ > 0; });
}

bool InterceptionCommandReader::waitForData(const std::chrono::milliseconds timeout)
{
    return reader_->wait_for_unread_message(toDdsDuration(timeout));
}

std::expected<std::optional<domain::InterceptionCommand>, InterceptionCommandMappingError>
InterceptionCommandReader::takeNext()
{
    dds::InterceptionCommand wireCommand;
    eprosima::fastdds::dds::SampleInfo sampleInfo{};
    const auto returnCode = reader_->take_next_sample(&wireCommand, &sampleInfo);
    if (returnCode != eprosima::fastdds::dds::RETCODE_OK)
    {
        throw std::runtime_error{
            "Fast DDS could not take InterceptionCommand sample; return code " +
            std::to_string(returnCode)};
    }
    if (!sampleInfo.valid_data)
    {
        return std::optional<domain::InterceptionCommand>{};
    }

    auto mappedCommand = fromWireInterceptionCommand(wireCommand);
    if (!mappedCommand.has_value())
    {
        return std::unexpected{mappedCommand.error()};
    }
    return std::optional<domain::InterceptionCommand>{*mappedCommand};
}

} // namespace drone::dds_transport
