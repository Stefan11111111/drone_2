#include "process_test_support.h"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::test::ChildOutput;
using drone::test::ChildProcess;
using namespace std::chrono_literals;

constexpr std::uint32_t pursuitDomainId{177};
constexpr auto readinessTimeout{3s};
constexpr auto actionTimeout{5s};
constexpr auto pursuitTimeout{5s};
constexpr auto processExitTimeout{2s};
constexpr std::size_t requiredPursuitSamples{8};
constexpr std::string_view consoleReadyLog{"console: readers ready for targets and drones"};
constexpr std::string_view interceptorReadyLog{"interceptor: published available state"};
constexpr std::string_view assignmentLog{
    "console: automated assignment sent for drone 1 to target 1\n"};
constexpr std::string_view startLog{"console: automated interception start sent for drone 1\n"};
constexpr std::string_view pursuitPrefix{"interceptor: pursuit position "};

struct Vector final
{
    double x;
    double y;
    double z;
};

struct PursuitSample final
{
    Vector dronePosition;
    Vector targetPosition;
    std::int64_t targetTimeMilliseconds;
};

struct Course final
{
    Vector movement;
    Vector targetPosition;
    std::int64_t targetTimeMilliseconds;
};

[[nodiscard]] std::vector<PursuitSample> extractPursuitSamples(const std::string &output)
{
    std::vector<PursuitSample> samples;
    std::istringstream lines{output};
    lines.imbue(std::locale::classic());

    std::string line;
    while (std::getline(lines, line))
    {
        if (!line.starts_with(pursuitPrefix))
        {
            continue;
        }

        std::istringstream row{line.substr(pursuitPrefix.size())};
        row.imbue(std::locale::classic());
        PursuitSample sample{};
        std::string targetLabel;
        std::string targetTimeLabel;
        if (row >> sample.dronePosition.x >> sample.dronePosition.y >> sample.dronePosition.z >>
                targetLabel >> sample.targetPosition.x >> sample.targetPosition.y >>
                sample.targetPosition.z >> targetTimeLabel >> sample.targetTimeMilliseconds &&
            targetLabel == "target" && targetTimeLabel == "target-time")
        {
            samples.push_back(sample);
        }
    }
    return samples;
}

[[nodiscard]] std::vector<Course> courses(const std::vector<PursuitSample> &samples)
{
    std::vector<Course> result;
    result.reserve(samples.size() - 1);
    for (std::size_t index = 1; index < samples.size(); ++index)
    {
        result.push_back(
            {.movement = {.x = samples[index].dronePosition.x - samples[index - 1].dronePosition.x,
                          .y = samples[index].dronePosition.y - samples[index - 1].dronePosition.y,
                          .z = samples[index].dronePosition.z - samples[index - 1].dronePosition.z},
             .targetPosition = samples[index].targetPosition,
             .targetTimeMilliseconds = samples[index].targetTimeMilliseconds});
    }
    return result;
}

[[nodiscard]] double crossMagnitude(const Vector &left, const Vector &right)
{
    const auto crossX = (left.y * right.z) - (left.z * right.y);
    const auto crossY = (left.z * right.x) - (left.x * right.z);
    const auto crossZ = (left.x * right.y) - (left.y * right.x);
    return std::hypot(crossX, crossY, crossZ);
}

[[nodiscard]] bool changedCourseForNewerTarget(const std::vector<PursuitSample> &samples)
{
    if (samples.size() < 3)
    {
        return false;
    }

    const auto observedCourses = courses(samples);
    for (std::size_t first = 0; first < observedCourses.size(); ++first)
    {
        for (std::size_t second = first + 1; second < observedCourses.size(); ++second)
        {
            if (observedCourses[first].targetTimeMilliseconds ==
                observedCourses[second].targetTimeMilliseconds)
            {
                continue;
            }
            if (observedCourses[first].targetPosition.x ==
                    observedCourses[second].targetPosition.x &&
                observedCourses[first].targetPosition.y ==
                    observedCourses[second].targetPosition.y &&
                observedCourses[first].targetPosition.z == observedCourses[second].targetPosition.z)
            {
                continue;
            }
            if (crossMagnitude(observedCourses[first].movement, observedCourses[second].movement) >
                1.0e-5)
            {
                return true;
            }
        }
    }
    return false;
}

TEST(MovingTargetPursuit,
     GivenAutomatedConsoleActions_WhenNewerTargetSamplesArrive_ThenDroneChangesCourse)
{
    const auto domainId = std::to_string(pursuitDomainId);
    ChildProcess console{
        CONSOLE_EXECUTABLE_PATH, {domainId, "--auto-pursuit"}, ChildOutput::capture};
    ASSERT_TRUE(console.waitForOutput(consoleReadyLog, readinessTimeout))
        << "The console did not become ready.\nConsole log:\n"
        << console.capturedOutput();
    ChildProcess interceptor{INTERCEPTOR_EXECUTABLE_PATH, {domainId}, ChildOutput::capture};
    ASSERT_TRUE(interceptor.waitForOutput(interceptorReadyLog, readinessTimeout))
        << "The interceptor did not become ready.\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ChildProcess observer{OBSERVER_EXECUTABLE_PATH, {domainId, "100"}, ChildOutput::capture};

    ASSERT_TRUE(console.waitForOutput(assignmentLog, actionTimeout))
        << "The automated console did not publish an assignment.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ASSERT_TRUE(console.waitForOutput(startLog, actionTimeout))
        << "The automated console did not publish a start command.\nConsole log:\n"
        << console.capturedOutput() << "\nInterceptor log:\n"
        << interceptor.capturedOutput();
    ASSERT_TRUE(
        interceptor.waitForOutputOccurrences(pursuitPrefix, requiredPursuitSamples, pursuitTimeout))
        << "The interceptor did not report " << requiredPursuitSamples
        << " pursuit ticks.\nInterceptor log:\n"
        << interceptor.capturedOutput() << "\nObserver log:\n"
        << observer.capturedOutput();

    const auto samples = extractPursuitSamples(interceptor.capturedOutput());
    EXPECT_TRUE(changedCourseForNewerTarget(samples))
        << "The interceptor moved, but no changed target sample produced a changed movement "
           "direction.\nInterceptor log:\n"
        << interceptor.capturedOutput();

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
