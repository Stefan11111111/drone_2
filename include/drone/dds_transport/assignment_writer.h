#ifndef DRONE_DDS_TRANSPORT_ASSIGNMENT_WRITER_H
#define DRONE_DDS_TRANSPORT_ASSIGNMENT_WRITER_H

#include "drone/dds_transport/assignment_topic.h"
#include "drone/domain/assignment.h"

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

class AssignmentWriter final : private eprosima::fastdds::dds::DataWriterListener
{
  public:
    explicit AssignmentWriter(eprosima::fastdds::dds::DomainParticipant &participant);
    ~AssignmentWriter() noexcept override;

    AssignmentWriter(const AssignmentWriter &) = delete;
    AssignmentWriter &operator=(const AssignmentWriter &) = delete;
    AssignmentWriter(AssignmentWriter &&) = delete;
    AssignmentWriter &operator=(AssignmentWriter &&) = delete;

    [[nodiscard]] bool waitForReaderMatch(std::chrono::milliseconds timeout);
    void write(const domain::Assignment &assignment);

  private:
    void
    on_publication_matched(eprosima::fastdds::dds::DataWriter *writer,
                           const eprosima::fastdds::dds::PublicationMatchedStatus &status) override;

    eprosima::fastdds::dds::DomainParticipant &participant_;
    AssignmentTopic topic_;
    std::mutex matchMutex_;
    std::condition_variable matchChanged_;
    int currentMatchCount_{0};
    eprosima::fastdds::dds::Publisher *publisher_{nullptr};
    eprosima::fastdds::dds::DataWriter *writer_{nullptr};
};

} // namespace drone::dds_transport

#endif // DRONE_DDS_TRANSPORT_ASSIGNMENT_WRITER_H
