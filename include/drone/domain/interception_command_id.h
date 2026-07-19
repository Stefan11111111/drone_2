#ifndef DRONE_DOMAIN_INTERCEPTION_COMMAND_ID_H
#define DRONE_DOMAIN_INTERCEPTION_COMMAND_ID_H

#include <compare>
#include <cstdint>

namespace drone::domain
{

class InterceptionCommandId final
{
  public:
    using Value = std::uint64_t;

    explicit InterceptionCommandId(Value value);

    [[nodiscard]] Value value() const noexcept;

    auto operator<=>(const InterceptionCommandId &) const = default;

  private:
    Value value_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_INTERCEPTION_COMMAND_ID_H
