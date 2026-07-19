#include "drone/domain/target_track.h"

#include <chrono>

#include <gtest/gtest.h>

namespace
{

using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

TEST(TargetTrack, GivenValidMeasurement_WhenConstructed_ThenItPreservesTheTargetPositionAndTime)
{
    const TargetId targetId{42};
    const Position position{125.0, -30.5, 850.0};
    const Timestamp measuredAt{1'500ms};

    const TargetTrack track{targetId, position, measuredAt};

    EXPECT_EQ(track.targetId(), targetId);
    EXPECT_EQ(track.position(), position);
    EXPECT_EQ(track.measuredAt(), measuredAt);
}

TEST(TargetTrack, GivenTracks_WhenCompared_ThenEveryMeasurementFieldParticipatesInEquality)
{
    const TargetTrack track{TargetId{42}, Position{125.0, -30.5, 850.0}, Timestamp{1'500ms}};
    const TargetTrack copy{TargetId{42}, Position{125.0, -30.5, 850.0}, Timestamp{1'500ms}};
    const TargetTrack differentTarget{TargetId{43}, Position{125.0, -30.5, 850.0},
                                      Timestamp{1'500ms}};
    const TargetTrack differentPosition{TargetId{42}, Position{126.0, -30.5, 850.0},
                                        Timestamp{1'500ms}};
    const TargetTrack differentTime{TargetId{42}, Position{125.0, -30.5, 850.0},
                                    Timestamp{1'501ms}};

    EXPECT_EQ(track, copy);
    EXPECT_NE(track, differentTarget);
    EXPECT_NE(track, differentPosition);
    EXPECT_NE(track, differentTime);
}

} // namespace
