#include "drone/dds_transport/explosion_event_writer.h"

#include "drone/dds_transport/explosion_event_mapping.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

#include <chrono>
#include <mutex>
#include <stdexcept>
#include <string>

namespace drone::dds_transport
{

ExplosionEventWriter::ExplosionEventWriter(eprosima::fastdds::dds::DomainParticipant &participant)
    : participant_{participant}, topic_{participant}
{
    publisher_ = participant_.create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
    if (publisher_ == nullptr)
    {
        throw std::runtime_error{"Fast DDS could not create the ExplosionEvent Publisher"};
    }

    writer_ = publisher_->create_datawriter(&topic_.topic(), explosionEventWriterQos(), this);
    if (writer_ == nullptr)
    {
        static_cast<void>(participant_.delete_publisher(publisher_));
        publisher_ = nullptr;
        throw std::runtime_error{"Fast DDS could not create the ExplosionEvent DataWriter"};
    }
}

ExplosionEventWriter::~ExplosionEventWriter() noexcept
{
    if (writer_ != nullptr)
    {
        static_cast<void>(publisher_->delete_datawriter(writer_));
    }
    if (publisher_ != nullptr)
    {
        static_cast<void>(participant_.delete_publisher(publisher_));
    }
}

void ExplosionEventWriter::on_publication_matched(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::PublicationMatchedStatus &status)
{
    static_cast<void>(writer);
    {
        const std::scoped_lock lock{matchMutex_};
        currentMatchCount_ = status.current_count;
    }
    matchChanged_.notify_all();
}

bool ExplosionEventWriter::waitForReaderMatch(const std::chrono::milliseconds timeout)
{
    std::unique_lock lock{matchMutex_};
    return matchChanged_.wait_for(lock, timeout, [this] { return currentMatchCount_ > 0; });
}

void ExplosionEventWriter::write(const domain::ExplosionEvent &event)
{
    auto wireEvent = toWireExplosionEvent(event);
    const auto returnCode = writer_->write(&wireEvent);
    if (returnCode != eprosima::fastdds::dds::RETCODE_OK)
    {
        throw std::runtime_error{"Fast DDS could not write ExplosionEvent sample; return code " +
                                 std::to_string(returnCode)};
    }
}

} // namespace drone::dds_transport
