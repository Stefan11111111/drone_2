#include "drone/console_ui_adapter/terminal_action.h"

#include "drone/domain/drone_id.h"

#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

namespace drone::console_ui
{
namespace
{

constexpr std::string_view startPrefix{"start "};

[[nodiscard]] std::optional<domain::DroneId> parseStartAction(const std::string_view line)
{
    if (!line.starts_with(startPrefix))
    {
        return std::nullopt;
    }

    const auto identifier = line.substr(startPrefix.size());
    std::uint64_t value{};
    const auto [parsedEnd, error] = std::from_chars(identifier.begin(), identifier.end(), value);
    if (error != std::errc{} || parsedEnd != identifier.end() || value == 0)
    {
        return std::nullopt;
    }
    return domain::DroneId{value};
}

[[nodiscard]] TerminalActionResult toTerminalResult(const console::StartInterceptionResult result)
{
    using enum console::StartInterceptionResult;
    switch (result)
    {
    case started:
        return TerminalActionResult::started;
    case unknownDrone:
        return TerminalActionResult::unknownDrone;
    case droneNotAssigned:
        return TerminalActionResult::droneNotAssigned;
    case duplicate:
        return TerminalActionResult::duplicate;
    }
    std::unreachable();
}

} // namespace

TerminalAction::TerminalAction(console::InterceptionCommandUseCase &useCase) noexcept
    : useCase_{useCase}
{
}

TerminalActionResult TerminalAction::execute(const std::string_view line)
{
    const auto droneId = parseStartAction(line);
    if (!droneId.has_value())
    {
        return TerminalActionResult::invalidSyntax;
    }
    return toTerminalResult(useCase_.start(*droneId));
}

} // namespace drone::console_ui
