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

constexpr std::uint32_t consoleFirstDomainId{191};
constexpr std::uint32_t interceptorFirstDomainId{192};
constexpr auto readinessTimeout{3s};
constexpr auto discoveryTimeout{5s};
constexpr auto stateTimeout{3s};
constexpr auto processExitTimeout{2s};
constexpr std::string_view consoleReadyLog{"console: readers ready for targets and drones"};
constexpr std::string_view interceptorPublishedLog{"interceptor: published available state"};
constexpr std::string_view droneDiscoveryLog{"console: matched interceptor DroneState writer\n"};
constexpr std::string_view availableDroneRow{"1 | 0.00 | 0.00 | 0.00 | available | - | 0\n"};

void expectAvailableState(ChildProcess &console, ChildProcess &interceptor)
{
    ASSERT_TRUE(console.waitForOutput(droneDiscoveryLog, discoveryTimeout))
        << "The console did not match the interceptor within " << discoveryTimeout.count()
        << " ms.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
    EXPECT_TRUE(console.waitForOutput(availableDroneRow, stateTimeout))
        << "The console matched but did not render the available state within "
        << stateTimeout.count() << " ms.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
}

void expectCleanShutdown(ChildProcess &console, ChildProcess &interceptor)
{
    EXPECT_TRUE(interceptor.terminateAndWait(processExitTimeout))
        << "The interceptor could not be cleaned up.\nInterceptor log:\n"
        << interceptor.capturedOutput();
    EXPECT_TRUE(console.terminateAndWait(processExitTimeout))
        << "The console could not be cleaned up.\nConsole log:\n"
        << console.capturedOutput();
}

TEST(LateJoinerDroneState,
     GivenConsoleReadersExistFirst_WhenInterceptorStarts_ThenLiveAvailableStateIsRendered)
{
    const auto domainId = std::to_string(consoleFirstDomainId);
    ChildProcess console{CONSOLE_EXECUTABLE_PATH, {"--domain-id", domainId}, ChildOutput::capture};
    ASSERT_TRUE(console.waitForOutput(consoleReadyLog, readinessTimeout))
        << "The console did not create its readers before the interceptor was launched.\nConsole "
           "log:\n"
        << console.capturedOutput();
    ChildProcess interceptor{
        INTERCEPTOR_EXECUTABLE_PATH, {"--domain-id", domainId}, ChildOutput::capture};

    expectAvailableState(console, interceptor);
    expectCleanShutdown(console, interceptor);
}

TEST(LateJoinerDroneState,
     GivenInterceptorWritesFirst_WhenConsoleStarts_ThenRetainedAvailableStateIsRendered)
{
    const auto domainId = std::to_string(interceptorFirstDomainId);
    ChildProcess interceptor{
        INTERCEPTOR_EXECUTABLE_PATH, {"--domain-id", domainId}, ChildOutput::capture};
    ASSERT_TRUE(interceptor.waitForOutput(interceptorPublishedLog, readinessTimeout))
        << "The interceptor did not publish before the console was launched.\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ChildProcess console{CONSOLE_EXECUTABLE_PATH, {"--domain-id", domainId}, ChildOutput::capture};
    ASSERT_TRUE(console.waitForOutput(consoleReadyLog, readinessTimeout))
        << "The late console did not create its readers.\nConsole log:\n"
        << console.capturedOutput();

    expectAvailableState(console, interceptor);
    expectCleanShutdown(console, interceptor);
}

} // namespace
