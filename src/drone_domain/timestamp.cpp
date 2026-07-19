#include "drone/domain/timestamp.h"

#include <stdexcept>

namespace drone::domain
{

Timestamp::Timestamp(const Duration timeSinceUnixEpoch) : timeSinceUnixEpoch_{timeSinceUnixEpoch}
{
    if (timeSinceUnixEpoch.count() < 0)
    {
        throw std::invalid_argument{"A timestamp cannot be before the Unix epoch"};
    }
}

Timestamp::Duration Timestamp::timeSinceUnixEpoch() const noexcept
{
    return timeSinceUnixEpoch_;
}

} // namespace drone::domain
