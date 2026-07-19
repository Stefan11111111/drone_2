#include "drone/dds_transport/target_track_mapping.h"

#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <target_track.hpp>

#include <array>
#include <chrono>
#include <limits>

#include <gtest/gtest.h>

namespace
{

using drone::dds::CartesianPosition;
using drone::dds::TargetTrack;
using drone::dds_transport::fromWireTargetTrack;
using drone::dds_transport::TargetTrackMappingError;
using drone::dds_transport::toWireTargetTrack;
using drone::domain::Position;
using drone::domain::TargetId;
using DomainTargetTrack = drone::domain::TargetTrack;
using drone::domain::Timestamp;
using namespace std::chrono_literals;

[[nodiscard]] TargetTrack makeWireTrack()
{
    CartesianPosition position;
    position.x(125.25);
    position.y(-30.5);
    position.z(850.75);

    TargetTrack track;
    track.target_id(42);
    track.position(position);
    track.measured_at_ms(1'500);
    return track;
}

TEST(TargetTrackMapping, GivenValidDomainTrack_WhenMappedToWireAndBack_ThenNoDataIsLost)
{
    const DomainTargetTrack original{TargetId{42}, Position{125.25, -30.5, 850.75},
                                     Timestamp{1'500ms}};

    const auto mapped = fromWireTargetTrack(toWireTargetTrack(original));

    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(*mapped, original);
}

TEST(TargetTrackMapping, GivenZeroTargetId_WhenMappedFromWire_ThenTheSampleIsRejected)
{
    TargetTrack wireTrack{makeWireTrack()};
    wireTrack.target_id(0);

    const auto mapped = fromWireTargetTrack(wireTrack);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), TargetTrackMappingError::zeroTargetId);
}

TEST(TargetTrackMapping, GivenNonFiniteCoordinate_WhenMappedFromWire_ThenTheSampleIsRejected)
{
    using CoordinateSetter = void (*)(CartesianPosition &, double);
    constexpr std::array<CoordinateSetter, 3> coordinateSetters{
        [](CartesianPosition &position, const double value) { position.x(value); },
        [](CartesianPosition &position, const double value) { position.y(value); },
        [](CartesianPosition &position, const double value) { position.z(value); },
    };
    constexpr std::array nonFiniteValues{
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN(),
    };

    for (const auto setCoordinate : coordinateSetters)
    {
        for (const double value : nonFiniteValues)
        {
            TargetTrack wireTrack{makeWireTrack()};
            setCoordinate(wireTrack.position(), value);

            const auto mapped = fromWireTargetTrack(wireTrack);

            ASSERT_FALSE(mapped.has_value());
            EXPECT_EQ(mapped.error(), TargetTrackMappingError::nonFinitePosition);
        }
    }
}

TEST(TargetTrackMapping, GivenNegativeTimestamp_WhenMappedFromWire_ThenTheSampleIsRejected)
{
    TargetTrack wireTrack{makeWireTrack()};
    wireTrack.measured_at_ms(-1);

    const auto mapped = fromWireTargetTrack(wireTrack);

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error(), TargetTrackMappingError::timestampBeforeUnixEpoch);
}

} // namespace
