#ifndef DRONE_DOMAIN_POSITION_H
#define DRONE_DOMAIN_POSITION_H

namespace drone::domain
{

class Position final
{
  public:
    Position(double xMeters, double yMeters, double zMeters);

    [[nodiscard]] double xMeters() const noexcept;
    [[nodiscard]] double yMeters() const noexcept;
    [[nodiscard]] double zMeters() const noexcept;

    bool operator==(const Position &) const = default;

  private:
    double xMeters_;
    double yMeters_;
    double zMeters_;
};

} // namespace drone::domain

#endif // DRONE_DOMAIN_POSITION_H
