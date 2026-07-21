#ifndef DRONE_APPLICATION_SUPPORT_SHUTDOWN_MONITOR_H
#define DRONE_APPLICATION_SUPPORT_SHUTDOWN_MONITOR_H

#include <chrono>
#include <csignal>

namespace drone::application
{

class ShutdownMonitor final
{
  public:
    ShutdownMonitor();
    ~ShutdownMonitor() noexcept;

    ShutdownMonitor(const ShutdownMonitor &) = delete;
    ShutdownMonitor &operator=(const ShutdownMonitor &) = delete;
    ShutdownMonitor(ShutdownMonitor &&) = delete;
    ShutdownMonitor &operator=(ShutdownMonitor &&) = delete;

    [[nodiscard]] static bool requested() noexcept;
    [[nodiscard]] static bool waitFor(std::chrono::milliseconds interval) noexcept;

  private:
    using SignalHandler = void (*)(int);

    static void requestShutdown(int signal) noexcept;

    static volatile std::sig_atomic_t shutdownRequested_;
    SignalHandler previousInterruptHandler_{};
    SignalHandler previousTerminationHandler_{};
};

} // namespace drone::application

#endif // DRONE_APPLICATION_SUPPORT_SHUTDOWN_MONITOR_H
