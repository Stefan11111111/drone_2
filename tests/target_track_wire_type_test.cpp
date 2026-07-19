#include <target_track.hpp>
#include <target_trackPubSubTypes.hpp>

#include <cstdint>

#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <gtest/gtest.h>

namespace
{

using drone::dds::CartesianPosition;
using drone::dds::TargetTrack;
using drone::dds::TargetTrackPubSubType;
using eprosima::fastdds::rtps::InstanceHandle_t;

[[nodiscard]] TargetTrack makeTrack(const std::uint64_t targetId)
{
    CartesianPosition position;
    position.x(125.0);
    position.y(-30.5);
    position.z(850.0);

    TargetTrack track;
    track.target_id(targetId);
    track.position(position);
    track.measured_at_ms(1'500);
    return track;
}

[[nodiscard]] InstanceHandle_t computeKey(TargetTrackPubSubType &topicType,
                                          const TargetTrack &track)
{
    InstanceHandle_t handle;
    EXPECT_TRUE(topicType.compute_key(&track, handle));
    return handle;
}

TEST(TargetTrackWireType, GivenGeneratedTargetTrack_WhenPopulated_ThenAllIdlFieldsAreAvailable)
{
    const TargetTrack track{makeTrack(42)};

    EXPECT_EQ(track.target_id(), 42U);
    EXPECT_DOUBLE_EQ(track.position().x(), 125.0);
    EXPECT_DOUBLE_EQ(track.position().y(), -30.5);
    EXPECT_DOUBLE_EQ(track.position().z(), 850.0);
    EXPECT_EQ(track.measured_at_ms(), 1'500);
}

TEST(TargetTrackWireType,
     GivenGeneratedTopicDataType_WhenKeysAreComputed_ThenOnlyTargetIdIdentifiesTheInstance)
{
    TargetTrackPubSubType topicType;
    const TargetTrack firstMeasurement{makeTrack(42)};
    TargetTrack newerMeasurement{makeTrack(42)};
    newerMeasurement.position().x(130.0);
    const TargetTrack differentTarget{makeTrack(43)};

    EXPECT_EQ(computeKey(topicType, firstMeasurement), computeKey(topicType, newerMeasurement));
    EXPECT_NE(computeKey(topicType, firstMeasurement), computeKey(topicType, differentTarget));
}

} // namespace
