#include "drone/console_dds_adapter/assignment_publisher.h"
#include "drone/domain/assignment.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/target_id.h"
#include "drone/interceptor_core/assignment_input_port.h"
#include "drone/interceptor_dds_adapter/assignment_subscriber.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::console_dds::AssignmentPublisher;
using drone::domain::Assignment;
using drone::domain::DroneId;
using drone::domain::TargetId;
using drone::interceptor::AssignmentInputPort;
using drone::interceptor_dds::AssignmentSubscriber;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t assignmentDeliveryDomainId{183};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};

class CapturingAssignmentInput final : public AssignmentInputPort
{
  public:
    void onAssignment(const Assignment &assignment) override
    {
        assignments.push_back(assignment);
    }

    std::vector<Assignment> assignments;
};

TEST(AssignmentDdsAdapter,
     GivenMatchedConsoleAndInterceptorAdapters_WhenAnAssignmentIsPublished_ThenCoreInputReceivesIt)
{
    CapturingAssignmentInput input;
    AssignmentSubscriber subscriber{assignmentDeliveryDomainId, "drone_step_34_interceptor", input};
    AssignmentPublisher publisher{assignmentDeliveryDomainId, "drone_step_34_console"};
    ASSERT_TRUE(publisher.waitForInterceptorMatch(discoveryTimeout))
        << "No interceptor Assignment DataReader matched within " << discoveryTimeout.count()
        << " ms";

    const Assignment sent{DroneId{7}, TargetId{42}};
    publisher.publish(sent);

    const auto received = subscriber.receiveNext(dataTimeout);
    ASSERT_TRUE(received.has_value())
        << "No valid Assignment reached interceptor core within " << dataTimeout.count() << " ms";
    EXPECT_EQ(input.assignments, (std::vector{sent}));
}

} // namespace
