#include "drone/domain/drone_id.h"

#include <stdexcept>

namespace drone::domain
{

DroneId::DroneId(const Value value) : value_{value}
{
    if (value == 0)
    {
        throw std::invalid_argument{"A drone identifier must be nonzero"};
    }
}

DroneId::Value DroneId::value() const noexcept
{
    return value_;
}

} // namespace drone::domain
