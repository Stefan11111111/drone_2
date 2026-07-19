#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <gtest/gtest.h>

namespace
{

constexpr eprosima::fastdds::dds::DomainId_t smoke_test_domain_id{0};

} // namespace

TEST(DomainParticipantSmoke,
     GivenNamedParticipantQos_WhenCreatedAndDeleted_ThenFastDdsReturnsSuccess)
{
    using eprosima::fastdds::dds::DomainParticipantFactory;
    using eprosima::fastdds::dds::DomainParticipantQos;
    using eprosima::fastdds::dds::RETCODE_OK;

    auto *const participant_factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(participant_factory, nullptr) << "Fast DDS did not provide its participant factory";

    DomainParticipantQos participant_qos;
    participant_qos.name("drone_step_04_smoke");

    auto *const participant =
        participant_factory->create_participant(smoke_test_domain_id, participant_qos);
    ASSERT_NE(participant, nullptr) << "Fast DDS could not create the named domain participant";

    EXPECT_EQ(participant_factory->delete_participant(participant), RETCODE_OK)
        << "Fast DDS could not cleanly delete the domain participant";
}
