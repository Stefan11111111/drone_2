#ifndef DRONE_DOMAIN_DRONE_ID_H
#define DRONE_DOMAIN_DRONE_ID_H

#include <compare>
#include <cstdint>

namespace drone::domain
{

class DroneId final
{
  public:
    using Value = std::uint64_t;

    explicit DroneId(Value value);

    [[nodiscard]] Value value() const noexcept;

    auto operator<=>(const DroneId &) const = default;

  private:
    Value value_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_DRONE_ID_H
