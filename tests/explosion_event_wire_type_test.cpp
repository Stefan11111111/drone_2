#include <target_track.hpp>
#include <target_trackPubSubTypes.hpp>

#include <cstdint>

#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <gtest/gtest.h>

namespace
{

using drone::dds::CartesianPosition;
using drone::dds::ExplosionEvent;
using drone::dds::ExplosionEventPubSubType;
using eprosima::fastdds::rtps::InstanceHandle_t;

struct EventValues
{
    std::uint64_t eventId;
    std::uint64_t droneId;
    std::uint64_t targetId;
    double x;
    std::int64_t occurredAtMs;
};

[[nodiscard]] ExplosionEvent makeEvent(const EventValues values)
{
    CartesianPosition position;
    position.x(values.x);
    position.y(-30.5);
    position.z(850.0);

    ExplosionEvent event;
    event.event_id(values.eventId);
    event.drone_id(values.droneId);
    event.target_id(values.targetId);
    event.position(position);
    event.occurred_at_ms(values.occurredAtMs);
    return event;
}

[[nodiscard]] InstanceHandle_t computeKey(ExplosionEventPubSubType &topicType,
                                          const ExplosionEvent &event)
{
    InstanceHandle_t handle;
    EXPECT_TRUE(topicType.compute_key(&event, handle));
    return handle;
}

TEST(ExplosionEventWireType,
     GivenGeneratedExplosionEvent_WhenPopulated_ThenItCarriesEveryCorrelationFact)
{
    const ExplosionEvent event{makeEvent(
        {.eventId = 301, .droneId = 7, .targetId = 42, .x = 125.0, .occurredAtMs = 2'000})};

    EXPECT_EQ(event.event_id(), 301U);
    EXPECT_EQ(event.drone_id(), 7U);
    EXPECT_EQ(event.target_id(), 42U);
    EXPECT_DOUBLE_EQ(event.position().x(), 125.0);
    EXPECT_DOUBLE_EQ(event.position().y(), -30.5);
    EXPECT_DOUBLE_EQ(event.position().z(), 850.0);
    EXPECT_EQ(event.occurred_at_ms(), 2'000);
}

TEST(ExplosionEventWireType,
     GivenGeneratedEventTopicType_WhenKeysAreComputed_ThenEventIdIdentifiesTheOccurrence)
{
    ExplosionEventPubSubType topicType;
    const ExplosionEvent firstOutcome{makeEvent(
        {.eventId = 301, .droneId = 7, .targetId = 42, .x = 125.0, .occurredAtMs = 2'000})};
    const ExplosionEvent sameIdentityDifferentPayload{makeEvent(
        {.eventId = 301, .droneId = 8, .targetId = 43, .x = 126.0, .occurredAtMs = 2'001})};
    const ExplosionEvent differentOutcome{makeEvent(
        {.eventId = 302, .droneId = 7, .targetId = 42, .x = 125.0, .occurredAtMs = 2'000})};

    EXPECT_EQ(computeKey(topicType, firstOutcome),
              computeKey(topicType, sameIdentityDifferentPayload));
    EXPECT_NE(computeKey(topicType, firstOutcome), computeKey(topicType, differentOutcome));
}

} // namespace
