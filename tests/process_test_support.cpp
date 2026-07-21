#include "process_test_support.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <limits>
#include <poll.h>
#include <spawn.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

namespace drone::test
{
namespace
{

using namespace std::chrono_literals;

class Descriptor final
{
  public:
    explicit Descriptor(const int value = -1) noexcept : value_{value}
    {
    }

    ~Descriptor() noexcept
    {
        if (value_ >= 0)
        {
            static_cast<void>(::close(value_));
        }
    }

    Descriptor(const Descriptor &) = delete;
    Descriptor &operator=(const Descriptor &) = delete;
    Descriptor(Descriptor &&) = delete;
    Descriptor &operator=(Descriptor &&) = delete;

    [[nodiscard]] int get() const noexcept
    {
        return value_;
    }

    [[nodiscard]] int release() noexcept
    {
        return std::exchange(value_, -1);
    }

  private:
    int value_;
};

class SpawnFileActions final
{
  public:
    SpawnFileActions()
    {
        const auto error = ::posix_spawn_file_actions_init(&actions_);
        if (error != 0)
        {
            throw std::system_error{error, std::generic_category(),
                                    "Could not initialize child process file actions"};
        }
    }

    ~SpawnFileActions() noexcept
    {
        static_cast<void>(::posix_spawn_file_actions_destroy(&actions_));
    }

    SpawnFileActions(const SpawnFileActions &) = delete;
    SpawnFileActions &operator=(const SpawnFileActions &) = delete;
    SpawnFileActions(SpawnFileActions &&) = delete;
    SpawnFileActions &operator=(SpawnFileActions &&) = delete;

    void duplicate(const int source, const int destination)
    {
        check(::posix_spawn_file_actions_adddup2(&actions_, source, destination),
              "Could not configure child process output redirection");
    }

    void close(const int descriptor)
    {
        check(::posix_spawn_file_actions_addclose(&actions_, descriptor),
              "Could not configure a child process descriptor close");
    }

    [[nodiscard]] const posix_spawn_file_actions_t *get() const noexcept
    {
        return &actions_;
    }

  private:
    static void check(const int error, const std::string_view message)
    {
        if (error != 0)
        {
            throw std::system_error{error, std::generic_category(), std::string{message}};
        }
    }

    posix_spawn_file_actions_t actions_{};
};

void makeNonBlocking(const int descriptor)
{
    const auto statusFlags = ::fcntl(descriptor, F_GETFL);
    if (statusFlags < 0 || ::fcntl(descriptor, F_SETFL, statusFlags | O_NONBLOCK) < 0)
    {
        throw std::system_error{errno, std::generic_category(),
                                "Could not make the child output pipe non-blocking"};
    }

    const auto descriptorFlags = ::fcntl(descriptor, F_GETFD);
    if (descriptorFlags < 0 || ::fcntl(descriptor, F_SETFD, descriptorFlags | FD_CLOEXEC) < 0)
    {
        throw std::system_error{errno, std::generic_category(),
                                "Could not prevent the child output pipe from leaking"};
    }
}

[[nodiscard]] std::vector<char *> makeSpawnArguments(std::vector<std::string> &ownedArguments)
{
    std::vector<char *> spawnArguments;
    spawnArguments.reserve(ownedArguments.size() + 1);
    for (auto &argument : ownedArguments)
    {
        spawnArguments.push_back(argument.data());
    }
    spawnArguments.push_back(nullptr);
    return spawnArguments;
}

[[nodiscard]] std::size_t occurrenceCount(const std::string_view text,
                                          const std::string_view expected)
{
    std::size_t count{};
    std::size_t position{};
    while ((position = text.find(expected, position)) != std::string_view::npos)
    {
        ++count;
        position += expected.size();
    }
    return count;
}

[[nodiscard]] int pollTimeout(const std::chrono::steady_clock::time_point deadline)
{
    const auto remaining =
        std::chrono::ceil<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
    if (remaining <= 0ms)
    {
        return 0;
    }
    return static_cast<int>(std::min(remaining.count(), static_cast<decltype(remaining.count())>(
                                                            std::numeric_limits<int>::max())));
}

} // namespace

ChildProcess::ChildProcess(const std::string_view executablePath,
                           const std::vector<std::string> &arguments, const ChildOutput childOutput)
{
    std::vector<std::string> ownedArguments;
    ownedArguments.reserve(arguments.size() + 1);
    ownedArguments.emplace_back(executablePath);
    ownedArguments.insert(ownedArguments.end(), arguments.begin(), arguments.end());
    auto spawnArguments = makeSpawnArguments(ownedArguments);

    std::array<int, 2> pipeDescriptors{-1, -1};
    if (childOutput == ChildOutput::capture && ::pipe(pipeDescriptors.data()) != 0)
    {
        throw std::system_error{errno, std::generic_category(),
                                "Could not create the child output pipe"};
    }
    Descriptor outputReader{pipeDescriptors[0]};
    Descriptor outputWriter{pipeDescriptors[1]};

    const posix_spawn_file_actions_t *fileActionsPointer{};
    SpawnFileActions fileActions;
    if (childOutput == ChildOutput::capture)
    {
        makeNonBlocking(outputReader.get());
        fileActions.duplicate(outputWriter.get(), STDOUT_FILENO);
        fileActions.duplicate(outputWriter.get(), STDERR_FILENO);
        fileActions.close(outputReader.get());
        fileActions.close(outputWriter.get());
        fileActionsPointer = fileActions.get();
    }

    const std::string executable{executablePath};
    const auto error = ::posix_spawn(&processId_, executable.c_str(), fileActionsPointer, nullptr,
                                     spawnArguments.data(), environ);
    if (error != 0)
    {
        processId_ = -1;
        throw std::system_error{error, std::generic_category(),
                                "Could not start child process " + executable};
    }

    if (childOutput == ChildOutput::capture)
    {
        outputDescriptor_ = outputReader.release();
    }
}

ChildProcess::~ChildProcess() noexcept
{
    static_cast<void>(terminateAndWait(2s));
    if (outputDescriptor_ >= 0)
    {
        static_cast<void>(::close(outputDescriptor_));
    }
}

bool ChildProcess::waitForOutput(const std::string_view expected,
                                 const std::chrono::milliseconds timeout)
{
    return waitForOutputOccurrences(expected, 1, timeout);
}

bool ChildProcess::waitForOutputOccurrences(const std::string_view expected,
                                            const std::size_t count,
                                            const std::chrono::milliseconds timeout)
{
    if (outputDescriptor_ < 0)
    {
        throw std::logic_error{"Child process output capture was not requested"};
    }
    if (expected.empty())
    {
        throw std::invalid_argument{"Expected child output must not be empty"};
    }
    if (timeout < 0ms)
    {
        throw std::invalid_argument{"Child output wait timeout must not be negative"};
    }

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (occurrenceCount(capturedOutput_, expected) < count)
    {
        pollfd descriptor{.fd = outputDescriptor_, .events = POLLIN, .revents = 0};
        int result{};
        do
        {
            result = ::poll(&descriptor, 1, pollTimeout(deadline));
        } while (result < 0 && errno == EINTR);

        if (result < 0)
        {
            throw std::system_error{errno, std::generic_category(),
                                    "Could not wait for child process output"};
        }
        if (result == 0)
        {
            return false;
        }

        drainOutput();
        if ((descriptor.revents & (POLLERR | POLLNVAL)) != 0)
        {
            return false;
        }
        if ((descriptor.revents & POLLHUP) != 0 && outputDescriptor_ < 0)
        {
            return occurrenceCount(capturedOutput_, expected) >= count;
        }
    }
    return true;
}

bool ChildProcess::terminateAndWait(const std::chrono::milliseconds timeout) noexcept
{
    if (processId_ <= 0)
    {
        drainOutput();
        return true;
    }

    if (!sendSignal(SIGTERM))
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

bool ChildProcess::sendSignal(const int signal) const noexcept
{
    if (processId_ <= 0)
    {
        return false;
    }
    return ::kill(processId_, signal) == 0 || errno == ESRCH;
}

std::optional<int> ChildProcess::exitCode() const noexcept
{
    return exitCode_;
}

const std::string &ChildProcess::capturedOutput() noexcept
{
    drainOutput();
    return capturedOutput_;
}

void ChildProcess::drainOutput() noexcept
{
    if (outputDescriptor_ < 0)
    {
        return;
    }

    std::array<char, 4'096> buffer{};
    while (true)
    {
        const auto bytesRead = ::read(outputDescriptor_, buffer.data(), buffer.size());
        if (bytesRead > 0)
        {
            try
            {
                capturedOutput_.append(buffer.data(), static_cast<std::size_t>(bytesRead));
            }
            catch (...)
            {
                static_cast<void>(::close(outputDescriptor_));
                outputDescriptor_ = -1;
                return;
            }
            continue;
        }
        if (bytesRead == 0)
        {
            static_cast<void>(::close(outputDescriptor_));
            outputDescriptor_ = -1;
            return;
        }
        if (errno == EINTR)
        {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }

        static_cast<void>(::close(outputDescriptor_));
        outputDescriptor_ = -1;
        return;
    }
}

bool ChildProcess::waitForExit(const std::chrono::milliseconds timeout) noexcept
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    do
    {
        drainOutput();
        int status{};
        const auto result = ::waitpid(processId_, &status, WNOHANG);
        if (result == processId_)
        {
            processId_ = -1;
            recordExitStatus(status);
            drainOutput();
            return true;
        }
        if (result < 0 && errno != EINTR)
        {
            processId_ = -1;
            drainOutput();
            return errno == ECHILD;
        }
        std::this_thread::sleep_for(10ms);
    } while (std::chrono::steady_clock::now() < deadline);
    return false;
}

bool ChildProcess::waitForExitBlocking() noexcept
{
    int status{};
    pid_t result{};
    do
    {
        result = ::waitpid(processId_, &status, 0);
    } while (result < 0 && errno == EINTR);

    processId_ = -1;
    if (result > 0)
    {
        recordExitStatus(status);
    }
    drainOutput();
    return result > 0 || (result < 0 && errno == ECHILD);
}

void ChildProcess::recordExitStatus(const int status) noexcept
{
    if (WIFEXITED(status))
    {
        exitCode_ = WEXITSTATUS(status);
        return;
    }
    exitCode_.reset();
}

} // namespace drone::test
