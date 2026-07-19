#include "drone/domain/position.h"

#include <cmath>
#include <stdexcept>
#include <string>
#include <string_view>

namespace drone::domain
{
namespace
{

double requireFinite(const double coordinate, const std::string_view coordinateName)
{
    if (!std::isfinite(coordinate))
    {
        throw std::invalid_argument{std::string{coordinateName} + " coordinate must be finite"};
    }

    return coordinate;
}

} // namespace

Position::Position(const double xMeters, const double yMeters, const double zMeters)
    : xMeters_{requireFinite(xMeters, "x")}, yMeters_{requireFinite(yMeters, "y")},
      zMeters_{requireFinite(zMeters, "z")}
{
}

double Position::xMeters() const noexcept
{
    return xMeters_;
}

double Position::yMeters() const noexcept
{
    return yMeters_;
}

double Position::zMeters() const noexcept
{
    return zMeters_;
}

} // namespace drone::domain
