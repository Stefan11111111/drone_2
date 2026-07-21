#include "drone/dds_transport/domain_participant_owner.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

#include <stdexcept>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using eprosima::fastdds::dds::DomainId_t;
using eprosima::fastdds::dds::DomainParticipantFactory;

constexpr DomainId_t successfulDomainId{180};
constexpr DomainId_t partialStartupDomainId{181};
constexpr DomainId_t invalidDefaultPortDomainId{233};

static_assert(!std::is_copy_constructible_v<DomainParticipantOwner>);
static_assert(!std::is_copy_assignable_v<DomainParticipantOwner>);

TEST(DomainParticipantOwner,
     GivenExplicitDomainAndName_WhenOwnedParticipantLeavesScope_ThenNoParticipantRemains)
{
    auto *const factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(factory, nullptr);
    ASSERT_TRUE(factory->lookup_participants(successfulDomainId).empty());

    {
        const DomainParticipantOwner owner{successfulDomainId, "drone_step_18_success"};

        EXPECT_EQ(owner.participant().get_domain_id(), successfulDomainId);
        EXPECT_EQ(owner.participant().get_qos().name().to_string(), "drone_step_18_success");
        EXPECT_EQ(factory->lookup_participants(successfulDomainId).size(), 1U);
        EXPECT_EQ(factory->lookup_participant(successfulDomainId), &owner.participant());
    }

    EXPECT_TRUE(factory->lookup_participants(successfulDomainId).empty());
}

TEST(DomainParticipantOwner,
     GivenDomainWhoseDefaultPortsCannotBeAssigned_WhenConstructionFails_ThenNoParticipantRemains)
{
    auto *const factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(factory, nullptr);
    ASSERT_TRUE(factory->lookup_participants(invalidDefaultPortDomainId).empty());

    EXPECT_THROW((DomainParticipantOwner{invalidDefaultPortDomainId, "drone_step_18_failure"}),
                 std::invalid_argument);

    EXPECT_TRUE(factory->lookup_participants(invalidDefaultPortDomainId).empty());
}

TEST(DomainParticipantOwner, GivenEmptyOrOversizedName_WhenConstructed_ThenItIsRejectedBeforeDds)
{
    auto *const factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(factory, nullptr);
    ASSERT_TRUE(factory->lookup_participants(successfulDomainId).empty());

    EXPECT_THROW((DomainParticipantOwner{successfulDomainId, ""}), std::invalid_argument);
    EXPECT_THROW((DomainParticipantOwner{successfulDomainId, std::string(256, 'x')}),
                 std::invalid_argument);

    EXPECT_TRUE(factory->lookup_participants(successfulDomainId).empty());
}

TEST(DomainParticipantOwner,
     GivenAContainedEntityFromPartialStartup_WhenOwnerLeavesScope_ThenFallbackCleanupRemovesIt)
{
    auto *const factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(factory, nullptr);
    ASSERT_TRUE(factory->lookup_participants(partialStartupDomainId).empty());

    {
        DomainParticipantOwner owner{partialStartupDomainId, "drone_partial_startup"};
        ASSERT_NE(
            owner.participant().create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT),
            nullptr);
    }

    EXPECT_TRUE(factory->lookup_participants(partialStartupDomainId).empty());
}

} // namespace
