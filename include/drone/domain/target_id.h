#ifndef DRONE_DOMAIN_TARGET_ID_H
#define DRONE_DOMAIN_TARGET_ID_H

#include <compare>
#include <cstdint>

namespace drone::domain
{

class TargetId final
{
  public:
    using Value = std::uint64_t;

    explicit TargetId(Value value);

    [[nodiscard]] Value value() const noexcept;

    auto operator<=>(const TargetId &) const = default;

  private:
    Value value_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_TARGET_ID_H
