#include "process_test_support.h"

#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/drone_state_mapping.h"
#include "drone/dds_transport/drone_state_topic.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>

#include <target_track.hpp>

#include <chrono>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::droneStateReaderQos;
using drone::dds_transport::DroneStateTopic;
using drone::dds_transport::fromWireDroneState;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::Position;
using drone::domain::Timestamp;
using drone::test::ChildOutput;
using drone::test::ChildProcess;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t processDomainId{189};
constexpr auto dataTimeout{5s};
constexpr auto processExitTimeout{2s};

class WireDroneStateReader final
{
  public:
    explicit WireDroneStateReader(eprosima::fastdds::dds::DomainParticipant &participant)
        : participant_{participant}, topic_{participant}
    {
        subscriber_ =
            participant_.create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
        if (subscriber_ == nullptr)
        {
            throw std::runtime_error{"Could not create the probe DroneState Subscriber"};
        }

        reader_ = subscriber_->create_datareader(&topic_.topic(), droneStateReaderQos());
        if (reader_ == nullptr)
        {
            static_cast<void>(participant_.delete_subscriber(subscriber_));
            subscriber_ = nullptr;
            throw std::runtime_error{"Could not create the probe DroneState DataReader"};
        }
    }

    ~WireDroneStateReader() noexcept
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

    WireDroneStateReader(const WireDroneStateReader &) = delete;
    WireDroneStateReader &operator=(const WireDroneStateReader &) = delete;
    WireDroneStateReader(WireDroneStateReader &&) = delete;
    WireDroneStateReader &operator=(WireDroneStateReader &&) = delete;

    [[nodiscard]] std::optional<DroneState> receive(const std::chrono::milliseconds timeout)
    {
        const auto duration =
            eprosima::fastdds::dds::Duration_t{std::chrono::duration<long double>{timeout}.count()};
        if (!reader_->wait_for_unread_message(duration))
        {
            return std::nullopt;
        }

        drone::dds::DroneState wireState;
        eprosima::fastdds::dds::SampleInfo sampleInfo{};
        if (reader_->take_next_sample(&wireState, &sampleInfo) !=
                eprosima::fastdds::dds::RETCODE_OK ||
            !sampleInfo.valid_data)
        {
            return std::nullopt;
        }

        const auto state = fromWireDroneState(wireState);
        if (!state.has_value())
        {
            return std::nullopt;
        }
        return *state;
    }

  private:
    eprosima::fastdds::dds::DomainParticipant &participant_;
    DroneStateTopic topic_;
    eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};
    eprosima::fastdds::dds::DataReader *reader_{nullptr};
};

TEST(InterceptorPublisher,
     GivenASeparateInterceptorProcess_WhenItsWriterMatches_ThenAvailableStateIsReceived)
{
    DomainParticipantOwner readerParticipant{processDomainId, "drone_step_30_probe"};
    WireDroneStateReader reader{readerParticipant.participant()};
    ChildProcess interceptor{
        INTERCEPTOR_EXECUTABLE_PATH, {std::to_string(processDomainId)}, ChildOutput::capture};

    const auto received = reader.receive(dataTimeout);

    ASSERT_TRUE(received.has_value()) << "No valid DroneState reached the probe within "
                                      << dataTimeout.count() << " ms.\nInterceptor log:\n"
                                      << interceptor.capturedOutput();
    EXPECT_EQ(received, (DroneState{DroneId{1}, Position{0.0, 0.0, 0.0}, Timestamp{0ms},
                                    DroneStatus::available, std::nullopt}));
    EXPECT_TRUE(interceptor.terminateAndWait(processExitTimeout))
        << "The interceptor process could not be cleaned up.\nInterceptor log:\n"
        << interceptor.capturedOutput();
}

} // namespace
