#include "drone/domain/explosion_event.h"

#include <chrono>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <gtest/gtest.h>

namespace
{

using drone::domain::DroneId;
using drone::domain::ExplosionEvent;
using drone::domain::ExplosionEventId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

static_assert(!std::is_constructible_v<ExplosionEventId, DroneId>);
static_assert(!std::is_constructible_v<ExplosionEventId, TargetId>);

TEST(ExplosionEvent,
     GivenOperationalFacts_WhenExplosionEventIsConstructed_ThenItPreservesTheOutcome)
{
    const ExplosionEventId eventId{301};
    const DroneId droneId{7};
    const TargetId targetId{42};
    const Position position{125.0, -30.5, 850.0};
    const Timestamp occurredAt{2'000ms};

    const ExplosionEvent event{eventId, droneId, targetId, position, occurredAt};

    EXPECT_EQ(event.eventId(), eventId);
    EXPECT_EQ(event.droneId(), droneId);
    EXPECT_EQ(event.targetId(), targetId);
    EXPECT_EQ(event.position(), position);
    EXPECT_EQ(event.occurredAt(), occurredAt);
}

TEST(ExplosionEvent, GivenEvents_WhenCompared_ThenEveryOperationalFactParticipatesInEquality)
{
    const ExplosionEvent event{ExplosionEventId{301}, DroneId{7}, TargetId{42},
                               Position{125.0, -30.5, 850.0}, Timestamp{2'000ms}};

    EXPECT_EQ(event, (ExplosionEvent{ExplosionEventId{301}, DroneId{7}, TargetId{42},
                                     Position{125.0, -30.5, 850.0}, Timestamp{2'000ms}}));
    EXPECT_NE(event, (ExplosionEvent{ExplosionEventId{302}, DroneId{7}, TargetId{42},
                                     Position{125.0, -30.5, 850.0}, Timestamp{2'000ms}}));
    EXPECT_NE(event, (ExplosionEvent{ExplosionEventId{301}, DroneId{8}, TargetId{42},
                                     Position{125.0, -30.5, 850.0}, Timestamp{2'000ms}}));
    EXPECT_NE(event, (ExplosionEvent{ExplosionEventId{301}, DroneId{7}, TargetId{43},
                                     Position{125.0, -30.5, 850.0}, Timestamp{2'000ms}}));
    EXPECT_NE(event, (ExplosionEvent{ExplosionEventId{301}, DroneId{7}, TargetId{42},
                                     Position{126.0, -30.5, 850.0}, Timestamp{2'000ms}}));
    EXPECT_NE(event, (ExplosionEvent{ExplosionEventId{301}, DroneId{7}, TargetId{42},
                                     Position{125.0, -30.5, 850.0}, Timestamp{2'001ms}}));
}

TEST(ExplosionEvent, GivenNonzeroEventIdentifiers_WhenCompared_ThenTheyHaveValueSemantics)
{
    const ExplosionEventId eventId{301};

    EXPECT_EQ(eventId.value(), 301U);
    EXPECT_EQ(eventId, ExplosionEventId{301});
    EXPECT_LT(eventId, ExplosionEventId{302});
}

TEST(ExplosionEvent, GivenInvalidOperationalFacts_WhenConstructed_ThenTheyAreRejected)
{
    constexpr double zero{0.0};
    const double infinity{std::numeric_limits<double>::infinity()};

    EXPECT_THROW((ExplosionEventId{0}), std::invalid_argument);
    EXPECT_THROW((ExplosionEvent{ExplosionEventId{301}, DroneId{0}, TargetId{42},
                                 Position{zero, zero, zero}, Timestamp{2'000ms}}),
                 std::invalid_argument);
    EXPECT_THROW((ExplosionEvent{ExplosionEventId{301}, DroneId{7}, TargetId{0},
                                 Position{zero, zero, zero}, Timestamp{2'000ms}}),
                 std::invalid_argument);
    EXPECT_THROW((ExplosionEvent{ExplosionEventId{301}, DroneId{7}, TargetId{42},
                                 Position{infinity, zero, zero}, Timestamp{2'000ms}}),
                 std::invalid_argument);
    EXPECT_THROW((ExplosionEvent{ExplosionEventId{301}, DroneId{7}, TargetId{42},
                                 Position{zero, zero, zero}, Timestamp{-1ms}}),
                 std::invalid_argument);
}

} // namespace
