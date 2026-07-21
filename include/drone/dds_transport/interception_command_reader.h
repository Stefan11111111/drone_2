#ifndef DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_READER_H
#define DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_READER_H

#include "drone/dds_transport/interception_command_mapping.h"
#include "drone/dds_transport/interception_command_topic.h"
#include "drone/domain/interception_command.h"

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

class InterceptionCommandReader final : private eprosima::fastdds::dds::DataReaderListener
{
  public:
    explicit InterceptionCommandReader(eprosima::fastdds::dds::DomainParticipant &participant);
    ~InterceptionCommandReader() noexcept override;

    InterceptionCommandReader(const InterceptionCommandReader &) = delete;
    InterceptionCommandReader &operator=(const InterceptionCommandReader &) = delete;
    InterceptionCommandReader(InterceptionCommandReader &&) = delete;
    InterceptionCommandReader &operator=(InterceptionCommandReader &&) = delete;

    [[nodiscard]] bool waitForWriterMatch(std::chrono::milliseconds timeout);
    [[nodiscard]] bool waitForData(std::chrono::milliseconds timeout);
    [[nodiscard]] std::expected<std::optional<domain::InterceptionCommand>,
                                InterceptionCommandMappingError>
    takeNext();

  private:
    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader *reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    InterceptionCommandTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    int currentMatchCount_{0};
    eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};
    eprosima::fastdds::dds::DataReader *reader_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_READER_H
