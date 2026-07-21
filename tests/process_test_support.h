#ifndef DRONE_TESTS_PROCESS_TEST_SUPPORT_H
#define DRONE_TESTS_PROCESS_TEST_SUPPORT_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <sys/types.h>

namespace drone::test
{

enum class ChildOutput : std::uint8_t
{
    inherit,
    capture,
};

class ChildProcess final
{
  public:
    ChildProcess(std::string_view executablePath, const std::vector<std::string> &arguments,
                 ChildOutput childOutput = ChildOutput::inherit);
    ~ChildProcess() noexcept;

    ChildProcess(const ChildProcess &) = delete;
    ChildProcess &operator=(const ChildProcess &) = delete;
    ChildProcess(ChildProcess &&) = delete;
    ChildProcess &operator=(ChildProcess &&) = delete;

    [[nodiscard]] bool waitForOutput(std::string_view expected, std::chrono::milliseconds timeout);
    [[nodiscard]] bool waitForOutputOccurrences(std::string_view expected, std::size_t count,
                                                std::chrono::milliseconds timeout);
    [[nodiscard]] bool sendSignal(int signal) const noexcept;
    [[nodiscard]] bool waitForExit(std::chrono::milliseconds timeout) noexcept;
    [[nodiscard]] bool terminateAndWait(std::chrono::milliseconds timeout) noexcept;
    [[nodiscard]] std::optional<int> exitCode() const noexcept;
    [[nodiscard]] const std::string &capturedOutput() noexcept;

  private:
    void drainOutput() noexcept;
    [[nodiscard]] bool waitForExitBlocking() noexcept;
    void recordExitStatus(int status) noexcept;

    pid_t processId_{-1};
    int outputDescriptor_{-1};
    std::optional<int> exitCode_;
    std::string capturedOutput_;
};

} // namespace drone::test

#endif // DRONE_TESTS_PROCESS_TEST_SUPPORT_H
