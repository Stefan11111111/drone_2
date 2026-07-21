#include "drone/dds_transport/drone_state_reader.h"

#include "drone/dds_transport/drone_state_mapping.h"

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
        throw std::invalid_argument{"A DroneState data wait timeout must not be negative"};
    }
    return eprosima::fastdds::dds::Duration_t{std::chrono::duration<long double>{timeout}.count()};
}

[[noreturn]] void throwTakeFailure(const eprosima::fastdds::dds::ReturnCode_t returnCode)
{
    throw std::runtime_error{"Fast DDS could not take DroneState sample; return code " +
                             std::to_string(returnCode)};
}

} // namespace

DroneStateReader::DroneStateReader(eprosima::fastdds::dds::DomainParticipant &participant)
    : participant_{participant}, topic_{participant}
{
    subscriber_ = participant_.create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error{"Fast DDS could not create the DroneState Subscriber"};
    }

    reader_ = subscriber_->create_datareader(&topic_.topic(), droneStateReaderQos(), this);
    if (reader_ == nullptr)
    {
        static_cast<void>(participant_.delete_subscriber(subscriber_));
        subscriber_ = nullptr;
        throw std::runtime_error{"Fast DDS could not create the DroneState DataReader"};
    }
}

DroneStateReader::~DroneStateReader() noexcept
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

void DroneStateReader::on_subscription_matched(
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

bool DroneStateReader::waitForWriterMatch(const std::chrono::milliseconds timeout)
{
    std::unique_lock lock{matchMutex_};
    return matchChanged_.wait_for(lock, timeout, [this] { return currentMatchCount_ > 0; });
}

bool DroneStateReader::waitForData(const std::chrono::milliseconds timeout)
{
    return reader_->wait_for_unread_message(toDdsDuration(timeout));
}

std::expected<std::optional<domain::DroneState>, DroneStateMappingError>
DroneStateReader::takeNext()
{
    dds::DroneState wireState;
    eprosima::fastdds::dds::SampleInfo sampleInfo{};
    const auto returnCode = reader_->take_next_sample(&wireState, &sampleInfo);
    if (returnCode != eprosima::fastdds::dds::RETCODE_OK)
    {
        throwTakeFailure(returnCode);
    }
    if (!sampleInfo.valid_data)
    {
        return std::optional<domain::DroneState>{};
    }

    auto mappedState = fromWireDroneState(wireState);
    if (!mappedState.has_value())
    {
        return std::unexpected{mappedState.error()};
    }
    return std::optional<domain::DroneState>{*mappedState};
}

} // namespace drone::dds_transport
