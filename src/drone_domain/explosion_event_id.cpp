#include "drone/domain/explosion_event_id.h"

#include <stdexcept>

namespace drone::domain
{

ExplosionEventId::ExplosionEventId(const Value value) : value_{value}
{
    if (value == 0)
    {
        throw std::invalid_argument{"An explosion event identifier must be nonzero"};
    }
}

ExplosionEventId::Value ExplosionEventId::value() const noexcept
{
    return value_;
}

} // namespace drone::domain
