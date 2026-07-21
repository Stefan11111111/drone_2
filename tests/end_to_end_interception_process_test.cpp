#include "process_test_support.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace
{

using drone::test::ChildOutput;
using drone::test::ChildProcess;
using namespace std::chrono_literals;

constexpr std::uint32_t endToEndDomainId{196};
constexpr auto readinessTimeout{3s};
constexpr auto stateAndActionTimeout{5s};
constexpr auto outcomeTimeout{30s};
constexpr auto processExitTimeout{2s};
constexpr std::string_view consoleReadyLog{"console: readers ready for targets and drones"};
constexpr std::string_view interceptorReadyLog{"interceptor: published available state"};
constexpr std::string_view targetRow{"1 | 0.00 | 0.00 | 1000.00 | 0\n"};
constexpr std::string_view availableDroneRow{"1 | 0.00 | 0.00 | 0.00 | available | - | 0\n"};
constexpr std::string_view assignmentLog{
    "console: automated assignment sent for drone 1 to target 1\n"};
constexpr std::string_view startLog{"console: automated interception start sent for drone 1\n"};
constexpr std::string_view pursuitLog{"interceptor: pursuit position "};
constexpr std::string_view correlatedOutcomeRow{"1 | 1 | 1 | "};

TEST(CompleteVisionScenario,
     GivenThreeSeparateParticipants_WhenOperatorActionsAreAutomated_ThenCorrelatedOutcomeIsShown)
{
    const auto domainId = std::to_string(endToEndDomainId);
    ChildProcess console{
        CONSOLE_EXECUTABLE_PATH, {domainId, "--auto-pursuit"}, ChildOutput::capture};
    ASSERT_TRUE(console.waitForOutput(consoleReadyLog, readinessTimeout))
        << "The console did not create its readers.\nConsole log:\n"
        << console.capturedOutput();
    ChildProcess interceptor{INTERCEPTOR_EXECUTABLE_PATH, {domainId}, ChildOutput::capture};
    ASSERT_TRUE(interceptor.waitForOutput(interceptorReadyLog, readinessTimeout))
        << "The interceptor did not publish available state.\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ChildProcess observer{OBSERVER_EXECUTABLE_PATH, {domainId, "400"}, ChildOutput::capture};

    ASSERT_TRUE(console.waitForOutput(targetRow, stateAndActionTimeout))
        << "The console did not render live target state.\nConsole log:\n"
        << console.capturedOutput() << "\nObserver log:\n"
        << observer.capturedOutput();
    ASSERT_TRUE(console.waitForOutput(availableDroneRow, stateAndActionTimeout))
        << "The console did not render available drone state.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ASSERT_TRUE(console.waitForOutput(assignmentLog, stateAndActionTimeout))
        << "The console did not publish the assignment.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ASSERT_TRUE(console.waitForOutput(startLog, stateAndActionTimeout))
        << "The console did not publish the start command.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ASSERT_TRUE(interceptor.waitForOutput(pursuitLog, stateAndActionTimeout))
        << "The interceptor did not begin pursuit.\nInterceptor log:\n"
        << interceptor.capturedOutput() << "\nObserver log:\n"
        << observer.capturedOutput();

    ASSERT_TRUE(console.waitForOutput(correlatedOutcomeRow, outcomeTimeout))
        << "No correlated explosion outcome reached the console within " << outcomeTimeout.count()
        << " ms.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput() << "\nObserver log:\n"
        << observer.capturedOutput();

    EXPECT_TRUE(observer.terminateAndWait(processExitTimeout))
        << "The observer could not be cleaned up.\nObserver log:\n"
        << observer.capturedOutput();
    EXPECT_TRUE(interceptor.terminateAndWait(processExitTimeout))
        << "The interceptor could not be cleaned up.\nInterceptor log:\n"
        << interceptor.capturedOutput();
    EXPECT_TRUE(console.terminateAndWait(processExitTimeout))
        << "The console could not be cleaned up.\nConsole log:\n"
        << console.capturedOutput();
}

} // namespace
