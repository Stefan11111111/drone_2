#include "drone/domain/interception_command_id.h"

#include <stdexcept>

namespace drone::domain
{

InterceptionCommandId::InterceptionCommandId(const Value value) : value_{value}
{
    if (value == 0)
    {
        throw std::invalid_argument{"An interception command identifier must be nonzero"};
    }
}

InterceptionCommandId::Value InterceptionCommandId::value() const noexcept
{
    return value_;
}

} // namespace drone::domain
