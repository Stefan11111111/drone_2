#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/target_track_reader.h"
#include "drone/domain/target_track.h"

#include <fastdds/dds/core/Types.hpp>

#include <array>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <spawn.h>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtest/gtest.h>

extern char **environ;

namespace
{

using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::TargetTrackReader;
using drone::domain::TargetTrack;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t observerProcessDomainId{184};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{2s};
constexpr auto processExitTimeout{2s};
constexpr std::size_t requiredUpdateCount{3};

class ChildProcess final
{
  public:
    ChildProcess(const std::string &executablePath, const DomainId_t domainId,
                 const std::uint64_t tickCount)
    {
        auto executable = executablePath;
        auto domain = std::to_string(domainId);
        auto ticks = std::to_string(tickCount);
        std::array arguments{executable.data(), domain.data(), ticks.data(),
                             static_cast<char *>(nullptr)};

        const auto error = ::posix_spawn(&processId_, executable.c_str(), nullptr, nullptr,
                                         arguments.data(), environ);
        if (error != 0)
        {
            processId_ = -1;
            throw std::system_error{error, std::generic_category(),
                                    "Could not start the observer process"};
        }
    }

    ~ChildProcess() noexcept
    {
        static_cast<void>(terminateAndWait(processExitTimeout));
    }

    ChildProcess(const ChildProcess &) = delete;
    ChildProcess &operator=(const ChildProcess &) = delete;
    ChildProcess(ChildProcess &&) = delete;
    ChildProcess &operator=(ChildProcess &&) = delete;

    [[nodiscard]] bool terminateAndWait(const std::chrono::milliseconds timeout) noexcept
    {
        if (processId_ <= 0)
        {
            return true;
        }

        if (::kill(processId_, SIGTERM) != 0 && errno != ESRCH)
        {
            return false;
        }
        if (waitForExit(timeout))
        {
            return true;
        }

        static_cast<void>(::kill(processId_, SIGKILL));
        return waitForExitBlocking();
    }

  private:
    [[nodiscard]] bool waitForExit(const std::chrono::milliseconds timeout) noexcept
    {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        do
        {
            int status{};
            const auto result = ::waitpid(processId_, &status, WNOHANG);
            if (result == processId_)
            {
                processId_ = -1;
                return true;
            }
            if (result < 0 && errno != EINTR)
            {
                processId_ = -1;
                return errno == ECHILD;
            }
            std::this_thread::sleep_for(10ms);
        } while (std::chrono::steady_clock::now() < deadline);
        return false;
    }

    [[nodiscard]] bool waitForExitBlocking() noexcept
    {
        int status{};
        pid_t result{};
        do
        {
            result = ::waitpid(processId_, &status, 0);
        } while (result < 0 && errno == EINTR);

        processId_ = -1;
        return result > 0 || (result < 0 && errno == ECHILD);
    }

    pid_t processId_{-1};
};

TEST(ObserverPublisher,
     GivenASeparateObserverProcess_WhenItsWriterMatches_ThenMultipleMovingTracksAreReceived)
{
    ChildProcess observer{OBSERVER_EXECUTABLE_PATH, observerProcessDomainId, 100};
    DomainParticipantOwner probeParticipant{observerProcessDomainId, "drone_step_23_probe"};
    TargetTrackReader reader{probeParticipant.participant()};

    ASSERT_TRUE(reader.waitForWriterMatch(discoveryTimeout))
        << "No observer TargetTrack DataWriter matched the probe within "
        << discoveryTimeout.count() << " ms";

    std::vector<TargetTrack> receivedTracks;
    for (std::size_t index = 0; index < requiredUpdateCount; ++index)
    {
        ASSERT_TRUE(reader.waitForData(dataTimeout))
            << "Received only " << receivedTracks.size() << " of " << requiredUpdateCount
            << " expected observer updates before a " << dataTimeout.count() << " ms timeout";
        auto track = reader.take();
        ASSERT_TRUE(track.has_value())
            << "Observer process published a TargetTrack that failed domain mapping";
        receivedTracks.push_back(std::move(*track));
    }

    EXPECT_EQ(receivedTracks[0].targetId(), receivedTracks[1].targetId());
    EXPECT_EQ(receivedTracks[1].targetId(), receivedTracks[2].targetId());
    EXPECT_LT(receivedTracks[0].measuredAt(), receivedTracks[1].measuredAt());
    EXPECT_LT(receivedTracks[1].measuredAt(), receivedTracks[2].measuredAt());
    EXPECT_NE(receivedTracks[0].position(), receivedTracks[1].position());
    EXPECT_NE(receivedTracks[1].position(), receivedTracks[2].position());
    EXPECT_TRUE(observer.terminateAndWait(processExitTimeout))
        << "The observer child process could not be cleaned up within "
        << processExitTimeout.count() << " ms";
}

} // namespace
