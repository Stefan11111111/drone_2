#ifndef DRONE_DOMAIN_TIMESTAMP_H
#define DRONE_DOMAIN_TIMESTAMP_H

#include <chrono>
#include <compare>

namespace drone::domain
{

class Timestamp final
{
  public:
    using Duration = std::chrono::milliseconds;

    explicit Timestamp(Duration timeSinceUnixEpoch);

    [[nodiscard]] Duration timeSinceUnixEpoch() const noexcept;

    auto operator<=>(const Timestamp &) const = default;

  private:
    Duration timeSinceUnixEpoch_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_TIMESTAMP_H
