#include "drone/domain/drone_id.h"
#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/interceptor_state_machine.h"
#include "drone/interceptor_dds_adapter/drone_state_publisher.h"
#include "drone/simulated_vehicle_adapter/simulated_vehicle.h"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <thread>

namespace
{

using namespace std::chrono_literals;

constexpr std::uint32_t defaultDomainId{0};
constexpr std::uint64_t maximumDefaultPortDomainId{232};

[[nodiscard]] std::uint32_t parseDomainId(const std::span<const char *const> arguments)
{
    if (arguments.size() > 2)
    {
        throw std::invalid_argument{"usage: interceptor [domain-id]"};
    }
    if (arguments.size() == 1)
    {
        return defaultDomainId;
    }

    std::uint64_t domainId{};
    const std::string_view text{arguments[1]};
    const auto [parsedEnd, error] = std::from_chars(text.begin(), text.end(), domainId);
    if (error != std::errc{} || parsedEnd != text.end())
    {
        throw std::invalid_argument{"domain-id must be an unsigned integer"};
    }
    if (domainId > maximumDefaultPortDomainId)
    {
        throw std::invalid_argument{"domain-id must not exceed 232"};
    }
    return static_cast<std::uint32_t>(domainId);
}

void run(const std::uint32_t domainId)
{
    drone::interceptor_dds::DroneStatePublisher statePublisher{domainId, "drone_interceptor_1"};
    drone::simulated_vehicle::SimulatedVehicle vehicle{
        {.initialPosition = drone::domain::Position{0.0, 0.0, 0.0},
         .startsAt = drone::domain::Timestamp{0ms},
         .maximumSpeedMetersPerSecond = 50.0}};
    drone::interceptor::InterceptorStateMachine stateMachine{drone::domain::DroneId{1}, vehicle,
                                                             vehicle, statePublisher};

    stateMachine.start();
    std::cout << "interceptor: published available state for drone 1 in DDS domain " << domainId
              << '\n'
              << std::flush;

    while (true)
    {
        std::this_thread::sleep_for(1s);
    }
}

} // namespace

int main(const int argumentCount, const char *const arguments[])
{
    try
    {
        run(parseDomainId(
            std::span<const char *const>{arguments, static_cast<std::size_t>(argumentCount)}));
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "interceptor: " << error.what() << '\n';
        return 1;
    }
}
