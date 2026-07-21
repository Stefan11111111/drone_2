#ifndef DRONE_CONSOLE_UI_ADAPTER_TERMINAL_ACTION_H
#define DRONE_CONSOLE_UI_ADAPTER_TERMINAL_ACTION_H

#include "drone/console_core/interception_command_use_case.h"

#include <cstdint>
#include <string_view>

namespace drone::console_ui
{

enum class TerminalActionResult : std::uint8_t
{
    started,
    invalidSyntax,
    unknownDrone,
    droneNotAssigned,
    duplicate,
};

class TerminalAction final
{
  public:
    explicit TerminalAction(console::InterceptionCommandUseCase &useCase) noexcept;

    [[nodiscard]] TerminalActionResult execute(std::string_view line);

  private:
    console::InterceptionCommandUseCase &useCase_;
};

} // namespace drone::console_ui

#endif // DRONE_CONSOLE_UI_ADAPTER_TERMINAL_ACTION_H
