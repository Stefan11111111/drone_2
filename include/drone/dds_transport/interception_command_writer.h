#ifndef DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_WRITER_H
#define DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_WRITER_H

#include "drone/dds_transport/interception_command_topic.h"
#include "drone/domain/interception_command.h"

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

class InterceptionCommandWriter final : private eprosima::fastdds::dds::DataWriterListener
{
  public:
    explicit InterceptionCommandWriter(eprosima::fastdds::dds::DomainParticipant &participant);
    ~InterceptionCommandWriter() noexcept override;

    InterceptionCommandWriter(const InterceptionCommandWriter &) = delete;
    InterceptionCommandWriter &operator=(const InterceptionCommandWriter &) = delete;
    InterceptionCommandWriter(InterceptionCommandWriter &&) = delete;
    InterceptionCommandWriter &operator=(InterceptionCommandWriter &&) = delete;

    [[nodiscard]] bool waitForReaderMatch(std::chrono::milliseconds timeout);
    void write(const domain::InterceptionCommand &command);

  private:
    void
    on_publication_matched(eprosima::fastdds::dds::DataWriter *writer,
                           const eprosima::fastdds::dds::PublicationMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    InterceptionCommandTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    int currentMatchCount_{0};
    eprosima::fastdds::dds::Publisher *publisher_{nullptr};
    eprosima::fastdds::dds::DataWriter *writer_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_INTERCEPTION_COMMAND_WRITER_H
