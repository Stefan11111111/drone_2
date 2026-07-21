#ifndef DRONE_CONSOLE_UI_ADAPTER_TERMINAL_VIEW_H
#define DRONE_CONSOLE_UI_ADAPTER_TERMINAL_VIEW_H

#include "drone/console_core/target_projection.h"

#include <iosfwd>

namespace drone::console_ui
{

class TerminalView final
{
  public:
    explicit TerminalView(std::ostream &output) noexcept;

    void render(const console::TargetProjection &projection) const;

  private:
    std::ostream &output_;
};

} // namespace drone::console_ui

#endif // DRONE_CONSOLE_UI_ADAPTER_TERMINAL_VIEW_H
