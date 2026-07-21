#include "drone/application_support/shutdown_monitor.h"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <stdexcept>
#include <thread>

namespace drone::application
{
namespace
{

using namespace std::chrono_literals;

constexpr auto shutdownResponseInterval{10ms};

} // namespace

volatile std::sig_atomic_t ShutdownMonitor::shutdownRequested_{};

ShutdownMonitor::ShutdownMonitor()
{
    shutdownRequested_ = 0;
    previousInterruptHandler_ = std::signal(SIGINT, &ShutdownMonitor::requestShutdown);
    if (previousInterruptHandler_ == SIG_ERR)
    {
        throw std::runtime_error{"Could not install the SIGINT shutdown handler"};
    }

    previousTerminationHandler_ = std::signal(SIGTERM, &ShutdownMonitor::requestShutdown);
    if (previousTerminationHandler_ == SIG_ERR)
    {
        static_cast<void>(std::signal(SIGINT, previousInterruptHandler_));
        throw std::runtime_error{"Could not install the SIGTERM shutdown handler"};
    }
}

ShutdownMonitor::~ShutdownMonitor() noexcept
{
    static_cast<void>(std::signal(SIGTERM, previousTerminationHandler_));
    static_cast<void>(std::signal(SIGINT, previousInterruptHandler_));
}

bool ShutdownMonitor::requested() noexcept
{
    return shutdownRequested_ != 0;
}

bool ShutdownMonitor::waitFor(std::chrono::milliseconds interval) noexcept
{
    while (!requested() && interval > std::chrono::milliseconds::zero())
    {
        const auto wait = std::min(interval, shutdownResponseInterval);
        std::this_thread::sleep_for(wait);
        interval -= wait;
    }
    return requested();
}

void ShutdownMonitor::requestShutdown(const int signal) noexcept
{
    static_cast<void>(signal);
    shutdownRequested_ = 1;
}

} // namespace drone::application
