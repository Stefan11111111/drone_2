#ifndef DRONE_DOMAIN_EXPLOSION_EVENT_ID_H
#define DRONE_DOMAIN_EXPLOSION_EVENT_ID_H

#include <compare>
#include <cstdint>

namespace drone::domain
{

class ExplosionEventId final
{
  public:
    using Value = std::uint64_t;

    explicit ExplosionEventId(Value value);

    [[nodiscard]] Value value() const noexcept;

    auto operator<=>(const ExplosionEventId &) const = default;

  private:
    Value value_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_EXPLOSION_EVENT_ID_H
