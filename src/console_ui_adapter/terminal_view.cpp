#include "drone/console_ui_adapter/terminal_view.h"

#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <iomanip>
#include <locale>
#include <ostream>
#include <sstream>
#include <string_view>
#include <utility>

namespace drone::console_ui
{
namespace
{

[[nodiscard]] std::string_view statusName(const domain::DroneStatus status)
{
    using enum domain::DroneStatus;
    switch (status)
    {
    case available:
        return "available";
    case assigned:
        return "assigned";
    case intercepting:
        return "intercepting";
    case interceptionSucceeded:
        return "succeeded";
    case interceptionFailed:
        return "failed";
    }
    std::unreachable();
}

void renderTargets(std::ostringstream &snapshot, const console::TargetProjection &projection)
{
    const auto tracks = projection.targetTracks();
    snapshot << "Targets (" << tracks.size() << ")\n"
             << "ID | X (m) | Y (m) | Z (m) | Measured (ms)\n";

    if (tracks.empty())
    {
        snapshot << "<none>\n";
        return;
    }

    snapshot << std::fixed << std::setprecision(2);
    for (const auto &track : tracks)
    {
        const auto &position = track.position();
        snapshot << track.targetId().value() << " | " << position.xMeters() << " | "
                 << position.yMeters() << " | " << position.zMeters() << " | "
                 << track.measuredAt().timeSinceUnixEpoch().count() << '\n';
    }
}

void renderDrones(std::ostringstream &snapshot, const console::DroneProjection &projection)
{
    const auto states = projection.droneStates();
    snapshot << "Drones (" << states.size() << ")\n"
             << "ID | X (m) | Y (m) | Z (m) | Status | Target | Reported (ms)\n";

    if (states.empty())
    {
        snapshot << "<none>\n";
        return;
    }

    snapshot << std::fixed << std::setprecision(2);
    for (const auto &state : states)
    {
        const auto &position = state.position();
        snapshot << state.droneId().value() << " | " << position.xMeters() << " | "
                 << position.yMeters() << " | " << position.zMeters() << " | "
                 << statusName(state.status()) << " | ";
        if (state.assignedTargetId().has_value())
        {
            snapshot << state.assignedTargetId()->value();
        }
        else
        {
            snapshot << '-';
        }
        snapshot << " | " << state.reportedAt().timeSinceUnixEpoch().count() << '\n';
    }
}

} // namespace

TerminalView::TerminalView(std::ostream &output) noexcept : output_{output}
{
}

void TerminalView::render(const console::TargetProjection &targets,
                          const console::DroneProjection &drones) const
{
    std::ostringstream snapshot;
    snapshot.imbue(std::locale::classic());
    renderTargets(snapshot, targets);
    snapshot << '\n';
    renderDrones(snapshot, drones);
    snapshot << '\n';
    output_ << snapshot.str();
    output_.flush();
}

} // namespace drone::console_ui
