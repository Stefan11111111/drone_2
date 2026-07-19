#include "drone/domain/target_id.h"

#include <stdexcept>

namespace drone::domain
{

TargetId::TargetId(const Value value) : value_{value}
{
    if (value == 0)
    {
        throw std::invalid_argument{"A target identifier must be nonzero"};
    }
}

TargetId::Value TargetId::value() const noexcept
{
    return value_;
}

} // namespace drone::domain
