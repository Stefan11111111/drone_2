#include "drone/console_ui_adapter/terminal_view.h"

#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <iomanip>
#include <locale>
#include <ostream>
#include <sstream>

namespace drone::console_ui
{

TerminalView::TerminalView(std::ostream &output) noexcept : output_{output}
{
}

void TerminalView::render(const console::TargetProjection &projection) const
{
    const auto tracks = projection.targetTracks();
    std::ostringstream snapshot;
    snapshot.imbue(std::locale::classic());
    snapshot << "Targets (" << tracks.size() << ")\n"
             << "ID | X (m) | Y (m) | Z (m) | Measured (ms)\n";

    if (tracks.empty())
    {
        snapshot << "<none>\n";
    }
    else
    {
        snapshot << std::fixed << std::setprecision(2);
        for (const auto &track : tracks)
        {
            const auto &position = track.position();
            snapshot << track.targetId().value() << " | " << position.xMeters() << " | "
                     << position.yMeters() << " | " << position.zMeters() << " | "
                     << track.measuredAt().timeSinceUnixEpoch().count() << '\n';
        }
    }

    snapshot << '\n';
    output_ << snapshot.str();
    output_.flush();
}

} // namespace drone::console_ui
