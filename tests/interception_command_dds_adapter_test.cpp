#include "drone/console_dds_adapter/interception_command_publisher.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/interception_command.h"
#include "drone/domain/interception_command_id.h"
#include "drone/domain/target_id.h"
#include "drone/interceptor_core/interception_command_input_port.h"
#include "drone/interceptor_dds_adapter/interception_command_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::console_dds::InterceptionCommandPublisher;
using drone::domain::DroneId;
using drone::domain::InterceptionCommand;
using drone::domain::InterceptionCommandId;
using drone::domain::TargetId;
using drone::interceptor::InterceptionCommandInputPort;
using drone::interceptor_dds::InterceptionCommandDelivery;
using drone::interceptor_dds::InterceptionCommandReceiveIssue;
using drone::interceptor_dds::InterceptionCommandSubscriber;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t commandDeliveryDomainId{180};
constexpr DomainId_t boundedShutdownDomainId{179};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};

class CapturingCommandInput final : public InterceptionCommandInputPort
{
  public:
    void onInterceptionCommand(const InterceptionCommand &command) override
    {
        commands.push_back(command);
    }

    std::vector<InterceptionCommand> commands;
};

TEST(InterceptionCommandDdsAdapter,
     GivenMatchedConsoleAndInterceptorAdapters_WhenACommandIsPublished_ThenCoreInputReceivesIt)
{
    CapturingCommandInput input;
    InterceptionCommandSubscriber subscriber{commandDeliveryDomainId, "drone_step_37_interceptor",
                                             input};
    InterceptionCommandPublisher publisher{commandDeliveryDomainId, "drone_step_37_console"};
    ASSERT_TRUE(publisher.waitForInterceptorMatch(discoveryTimeout));
    const InterceptionCommand sent{InterceptionCommandId{101}, DroneId{7}, TargetId{42}};

    publisher.publish(sent);

    const auto result = subscriber.receiveNext(dataTimeout);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, InterceptionCommandDelivery::delivered);
    EXPECT_EQ(input.commands, (std::vector{sent}));
}

TEST(InterceptionCommandDdsAdapter,
     GivenACommandIdentityDeliveredTwice_WhenReceived_ThenCoreInputSeesTheIntentOnce)
{
    CapturingCommandInput input;
    InterceptionCommandSubscriber subscriber{commandDeliveryDomainId,
                                             "drone_step_37_duplicate_interceptor", input};
    InterceptionCommandPublisher publisher{commandDeliveryDomainId,
                                           "drone_step_37_duplicate_console"};
    ASSERT_TRUE(publisher.waitForInterceptorMatch(discoveryTimeout));
    const InterceptionCommand sent{InterceptionCommandId{102}, DroneId{7}, TargetId{42}};
    publisher.publish(sent);
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), InterceptionCommandDelivery::delivered);

    publisher.publish(sent);

    EXPECT_EQ(subscriber.receiveNext(dataTimeout), InterceptionCommandDelivery::duplicate);
    EXPECT_EQ(input.commands, (std::vector{sent}));
}

TEST(InterceptionCommandDdsAdapter,
     GivenNoCommandSample_WhenTheBoundedWaitExpires_ThenTheSubscriberCanShutDown)
{
    CapturingCommandInput input;
    InterceptionCommandSubscriber subscriber{boundedShutdownDomainId,
                                             "drone_step_37_bounded_interceptor", input};

    const auto result = subscriber.receiveNext(10ms);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), InterceptionCommandReceiveIssue::timedOut);
    EXPECT_TRUE(input.commands.empty());
}

} // namespace
