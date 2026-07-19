#include "drone/domain/drone_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/timestamp.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <gtest/gtest.h>

namespace
{

using drone::domain::DroneId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

static_assert(!std::is_constructible_v<TargetId, DroneId>);
static_assert(!std::is_constructible_v<DroneId, TargetId>);

TEST(DomainValues, GivenTargetIdentifiers_WhenCompared_ThenTheyHaveValueSemantics)
{
    constexpr std::uint64_t firstValue{41};
    constexpr std::uint64_t secondValue{42};

    const TargetId first{firstValue};
    const TargetId firstCopy{firstValue};
    const TargetId second{secondValue};

    EXPECT_EQ(first.value(), firstValue);
    EXPECT_EQ(first, firstCopy);
    EXPECT_LT(first, second);
}

TEST(DomainValues, GivenDroneIdentifiers_WhenCompared_ThenTheyHaveValueSemantics)
{
    constexpr std::uint64_t firstValue{7};
    constexpr std::uint64_t secondValue{8};

    const DroneId first{firstValue};
    const DroneId firstCopy{firstValue};
    const DroneId second{secondValue};

    EXPECT_EQ(first.value(), firstValue);
    EXPECT_EQ(first, firstCopy);
    EXPECT_LT(first, second);
}

TEST(DomainValues, GivenZeroIdentifiers_WhenConstructed_ThenTheyAreRejected)
{
    EXPECT_THROW((TargetId{0}), std::invalid_argument);
    EXPECT_THROW((DroneId{0}), std::invalid_argument);
}

TEST(DomainValues, GivenFiniteCoordinates_WhenPositionIsConstructed_ThenItPreservesThem)
{
    constexpr double xMeters{12.5};
    constexpr double yMeters{-4.25};
    constexpr double zMeters{120.0};

    const Position position{xMeters, yMeters, zMeters};

    EXPECT_DOUBLE_EQ(position.xMeters(), xMeters);
    EXPECT_DOUBLE_EQ(position.yMeters(), yMeters);
    EXPECT_DOUBLE_EQ(position.zMeters(), zMeters);
    EXPECT_EQ(position, (Position{xMeters, yMeters, zMeters}));
}

TEST(DomainValues, GivenNonFiniteCoordinate_WhenPositionIsConstructed_ThenItIsRejected)
{
    constexpr double zero{0.0};
    const double infinity{std::numeric_limits<double>::infinity()};
    const double notANumber{std::numeric_limits<double>::quiet_NaN()};

    EXPECT_THROW((Position{notANumber, zero, zero}), std::invalid_argument);
    EXPECT_THROW((Position{zero, infinity, zero}), std::invalid_argument);
    EXPECT_THROW((Position{zero, zero, -infinity}), std::invalid_argument);
}

TEST(DomainValues, GivenTimestamps_WhenCompared_ThenTheyAreOrderedByMillisecondsSinceEpoch)
{
    const Timestamp earlier{1'000ms};
    const Timestamp earlierCopy{1'000ms};
    const Timestamp later{1'001ms};

    EXPECT_EQ(earlier.timeSinceUnixEpoch(), 1'000ms);
    EXPECT_EQ(earlier, earlierCopy);
    EXPECT_LT(earlier, later);
}

TEST(DomainValues, GivenTimeBeforeUnixEpoch_WhenTimestampIsConstructed_ThenItIsRejected)
{
    EXPECT_THROW((Timestamp{-1ms}), std::invalid_argument);
}

} // namespace
