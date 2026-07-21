#include "process_test_support.h"

#include <chrono>
#include <csignal>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::test::ChildOutput;
using drone::test::ChildProcess;
using namespace std::chrono_literals;

constexpr auto outputTimeout{2s};
constexpr auto processExitTimeout{2s};

struct InvalidConfigurationCase final
{
    std::string_view executablePath;
    std::vector<std::string> arguments;
    std::string_view expectedError;
};

struct ExpectedLifecycleLogs final
{
    std::string_view readiness;
    std::string_view shutdown;
};

void expectGracefulShutdown(const std::string_view executablePath,
                            const std::vector<std::string> &arguments,
                            const ExpectedLifecycleLogs expectedLogs)
{
    ChildProcess process{executablePath, arguments, ChildOutput::capture};
    ASSERT_TRUE(process.waitForOutput(expectedLogs.readiness, outputTimeout))
        << "Process did not become ready.\nProcess log:\n"
        << process.capturedOutput();
    ASSERT_TRUE(process.sendSignal(SIGTERM)) << "Could not send SIGTERM to the child process.";
    ASSERT_TRUE(process.waitForExit(processExitTimeout))
        << "Process did not stop within " << processExitTimeout.count()
        << " ms after SIGTERM.\nProcess log:\n"
        << process.capturedOutput();
    EXPECT_EQ(process.exitCode(), 0) << "SIGTERM did not produce a normal successful exit.";
    EXPECT_NE(process.capturedOutput().find(expectedLogs.shutdown), std::string::npos)
        << "Process did not report completed cleanup.\nProcess log:\n"
        << process.capturedOutput();
}

TEST(ProcessConfiguration,
     GivenInvalidDomainParticipantAndTickOptions_WhenProcessesStart_ThenErrorsAreActionable)
{
    const std::vector<InvalidConfigurationCase> cases{
        {.executablePath = OBSERVER_EXECUTABLE_PATH,
         .arguments = {"--domain-id", "233"},
         .expectedError = "observer: --domain-id must not exceed 232"},
        {.executablePath = CONSOLE_EXECUTABLE_PATH,
         .arguments = {"--participant-name", ""},
         .expectedError = "console: --participant-name must not be empty"},
        {.executablePath = INTERCEPTOR_EXECUTABLE_PATH,
         .arguments = {"--tick-ms", "0"},
         .expectedError = "interceptor: --tick-ms must be positive"},
    };

    for (const auto &testCase : cases)
    {
        ChildProcess process{testCase.executablePath, testCase.arguments, ChildOutput::capture};
        ASSERT_TRUE(process.waitForOutput(testCase.expectedError, outputTimeout))
            << "Expected configuration error was not reported.\nProcess log:\n"
            << process.capturedOutput();
        ASSERT_TRUE(process.waitForExit(processExitTimeout))
            << "Invalid configuration left a child process running.\nProcess log:\n"
            << process.capturedOutput();
        EXPECT_EQ(process.exitCode(), 1);
    }
}

TEST(ProcessLifecycle,
     GivenConfiguredProcessesWaitingForTheirNextTick_WhenSigtermArrives_ThenEachShutsDownCleanly)
{
    expectGracefulShutdown(
        OBSERVER_EXECUTABLE_PATH,
        {"--domain-id", "197", "--participant-name", "observer_lifecycle", "--tick-ms", "60000"},
        {.readiness = "observer: publishing target 1 in DDS domain 197; participant "
                      "'observer_lifecycle'; tick interval 60000 ms",
         .shutdown = "observer: shutdown complete\n"});

    expectGracefulShutdown(
        CONSOLE_EXECUTABLE_PATH,
        {"--domain-id", "198", "--participant-name", "console_lifecycle", "--tick-ms", "60000"},
        {.readiness = "console: readers ready for targets and drones; participant "
                      "'console_lifecycle' in DDS domain 198; poll interval 60000 ms",
         .shutdown = "console: shutdown complete\n"});

    expectGracefulShutdown(
        INTERCEPTOR_EXECUTABLE_PATH,
        {"--domain-id", "199", "--participant-name", "interceptor_lifecycle", "--tick-ms", "60000"},
        {.readiness = "interceptor: published available state for drone 1; participant "
                      "'interceptor_lifecycle' in DDS domain 199; control interval 60000 ms",
         .shutdown = "interceptor: shutdown complete\n"});
}

} // namespace
