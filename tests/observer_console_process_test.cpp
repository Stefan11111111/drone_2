#include "process_test_support.h"

#include <chrono>
#include <cstdint>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

namespace
{

using drone::test::ChildOutput;
using drone::test::ChildProcess;
using namespace std::chrono_literals;

constexpr std::uint32_t processScenarioDomainId{188};
constexpr auto discoveryTimeout{5s};
constexpr auto targetUpdatesTimeout{3s};
constexpr auto processExitTimeout{2s};
constexpr std::size_t requiredPositionCount{2};
constexpr std::string_view discoveryLog{"console: matched observer TargetTrack writer\n"};
constexpr std::string_view targetRowPrefix{"1 | "};

using RenderedPosition = std::tuple<double, double, double>;

[[nodiscard]] std::vector<RenderedPosition> extractRenderedPositions(const std::string &output)
{
    std::vector<RenderedPosition> positions;
    std::istringstream lines{output};
    lines.imbue(std::locale::classic());

    std::string line;
    while (std::getline(lines, line))
    {
        if (!line.starts_with(targetRowPrefix))
        {
            continue;
        }

        std::istringstream row{line};
        row.imbue(std::locale::classic());
        std::uint64_t targetId{};
        double xMeters{};
        double yMeters{};
        double zMeters{};
        std::int64_t measuredAtMilliseconds{};
        char separator{};
        if (row >> targetId >> separator >> xMeters >> separator >> yMeters >> separator >>
                zMeters >> separator >> measuredAtMilliseconds &&
            targetId == 1)
        {
            positions.emplace_back(xMeters, yMeters, zMeters);
        }
    }
    return positions;
}

[[nodiscard]] bool containsChangingPosition(const std::vector<RenderedPosition> &positions)
{
    if (positions.size() < requiredPositionCount)
    {
        return false;
    }

    for (std::size_t index = 1; index < positions.size(); ++index)
    {
        if (positions[index] != positions.front())
        {
            return true;
        }
    }
    return false;
}

TEST(
    CrossProcessTargets,
    GivenSeparateObserverAndConsoleProcesses_WhenDdsEndpointsMatch_ThenChangingPositionsReachTheConsole)
{
    const auto domainId = std::to_string(processScenarioDomainId);
    ChildProcess console{CONSOLE_EXECUTABLE_PATH, {"--domain-id", domainId}, ChildOutput::capture};
    ChildProcess observer{OBSERVER_EXECUTABLE_PATH,
                          {"--domain-id", domainId, "--tick-count", "100"},
                          ChildOutput::capture};

    ASSERT_TRUE(console.waitForOutput(discoveryLog, discoveryTimeout))
        << "The console did not report a matched observer within " << discoveryTimeout.count()
        << " ms.\nConsole log:\n"
        << console.capturedOutput() << "\nObserver log:\n"
        << observer.capturedOutput();
    ASSERT_TRUE(console.waitForOutputOccurrences(targetRowPrefix, requiredPositionCount,
                                                 targetUpdatesTimeout))
        << "The console did not render " << requiredPositionCount
        << " target updates after discovery within " << targetUpdatesTimeout.count()
        << " ms.\nConsole log:\n"
        << console.capturedOutput() << "\nObserver log:\n"
        << observer.capturedOutput();

    const auto positions = extractRenderedPositions(console.capturedOutput());
    EXPECT_TRUE(containsChangingPosition(positions))
        << "The console rendered target rows, but fewer than two positions differed.\nConsole "
           "log:\n"
        << console.capturedOutput();

    EXPECT_TRUE(observer.terminateAndWait(processExitTimeout))
        << "The observer child process could not be cleaned up within "
        << processExitTimeout.count() << " ms.\nObserver log:\n"
        << observer.capturedOutput();
    EXPECT_TRUE(console.terminateAndWait(processExitTimeout))
        << "The console child process could not be cleaned up within " << processExitTimeout.count()
        << " ms.\nConsole log:\n"
        << console.capturedOutput();
}

} // namespace
