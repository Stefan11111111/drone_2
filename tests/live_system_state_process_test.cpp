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

constexpr std::uint32_t processScenarioDomainId{190};
constexpr auto readinessTimeout{3s};
constexpr auto discoveryTimeout{5s};
constexpr auto stateTimeout{3s};
constexpr auto processExitTimeout{2s};
constexpr std::string_view consoleReadyLog{"console: readers ready for targets and drones"};
constexpr std::string_view targetDiscoveryLog{"console: matched observer TargetTrack writer\n"};
constexpr std::string_view droneDiscoveryLog{"console: matched interceptor DroneState writer\n"};
constexpr std::string_view targetRow{"1 | 0.00 | 0.00 | 1000.00 | 0\n"};
constexpr std::string_view droneRow{"1 | 0.00 | 0.00 | 0.00 | available | - | 0\n"};

TEST(LiveSystemState,
     GivenObserverConsoleAndInterceptorProcesses_WhenEndpointsMatch_ThenBothStatesAreShown)
{
    const auto domainId = std::to_string(processScenarioDomainId);
    ChildProcess console{CONSOLE_EXECUTABLE_PATH, {domainId}, ChildOutput::capture};
    ASSERT_TRUE(console.waitForOutput(consoleReadyLog, readinessTimeout))
        << "The console did not create both readers within " << readinessTimeout.count()
        << " ms.\nConsole log:\n"
        << console.capturedOutput();

    ChildProcess observer{OBSERVER_EXECUTABLE_PATH, {domainId, "100"}, ChildOutput::capture};
    ChildProcess interceptor{INTERCEPTOR_EXECUTABLE_PATH, {domainId}, ChildOutput::capture};

    ASSERT_TRUE(console.waitForOutput(targetDiscoveryLog, discoveryTimeout))
        << "The console did not match the observer.\nConsole log:\n"
        << console.capturedOutput() << "\nObserver log:\n"
        << observer.capturedOutput();
    ASSERT_TRUE(console.waitForOutput(droneDiscoveryLog, discoveryTimeout))
        << "The console did not match the interceptor.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
    EXPECT_TRUE(console.waitForOutput(targetRow, stateTimeout))
        << "The console did not render target state.\nConsole log:\n"
        << console.capturedOutput();
    EXPECT_TRUE(console.waitForOutput(droneRow, stateTimeout))
        << "The console did not render available drone state.\nConsole log:\n"
        << console.capturedOutput();

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
