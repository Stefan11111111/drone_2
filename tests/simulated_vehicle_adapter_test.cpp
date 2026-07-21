#include "drone/simulated_vehicle_adapter/simulated_vehicle.h"

#include "drone/domain/position.h"
#include "drone/domain/timestamp.h"
#include "drone/interceptor_core/flight_control_port.h"
#include "drone/interceptor_core/positioning_port.h"

#include <chrono>
#include <limits>
#include <stdexcept>

#include <gtest/gtest.h>

namespace
{

using drone::domain::Position;
using drone::domain::Timestamp;
using drone::interceptor::FlightControlPort;
using drone::interceptor::PositioningPort;
using drone::interceptor::PositionSample;
using drone::simulated_vehicle::Configuration;
using drone::simulated_vehicle::SimulatedVehicle;
using namespace std::chrono_literals;

[[nodiscard]] Configuration vehicleConfiguration()
{
    return {.initialPosition = Position{0.0, 0.0, 10.0},
            .startsAt = Timestamp{1'000ms},
            .maximumSpeedMetersPerSecond = 2.0};
}

TEST(SimulatedVehicle,
     GivenNoMovementRequest_WhenPositionIsRead_ThenTheInitialPositionAndTimeAreStable)
{
    SimulatedVehicle vehicle{vehicleConfiguration()};
    const PositioningPort &positioning = vehicle;

    EXPECT_EQ(positioning.currentPosition(), (PositionSample{.position = Position{0.0, 0.0, 10.0},
                                                             .measuredAt = Timestamp{1'000ms}}));
    EXPECT_EQ(positioning.currentPosition(), (PositionSample{.position = Position{0.0, 0.0, 10.0},
                                                             .measuredAt = Timestamp{1'000ms}}));
}

TEST(SimulatedVehicle, GivenADistantDestination_WhenOneMovementStepRuns_ThenTheVehicleMovesTowardIt)
{
    SimulatedVehicle vehicle{vehicleConfiguration()};
    FlightControlPort &flightControl = vehicle;

    flightControl.moveToward(Position{3.0, 4.0, 10.0}, 1s);

    const auto sample = vehicle.currentPosition();
    EXPECT_DOUBLE_EQ(sample.position.xMeters(), 1.2);
    EXPECT_DOUBLE_EQ(sample.position.yMeters(), 1.6);
    EXPECT_DOUBLE_EQ(sample.position.zMeters(), 10.0);
    EXPECT_EQ(sample.measuredAt, Timestamp{2'000ms});
}

TEST(SimulatedVehicle,
     GivenSeveralMovementSteps_WhenTheDestinationIsDistant_ThenEveryStepRespectsTheSpeedBound)
{
    SimulatedVehicle vehicle{vehicleConfiguration()};

    vehicle.moveToward(Position{100.0, 0.0, 10.0}, 250ms);
    EXPECT_EQ(vehicle.currentPosition().position, Position(0.5, 0.0, 10.0));

    vehicle.moveToward(Position{100.0, 0.0, 10.0}, 500ms);
    EXPECT_EQ(vehicle.currentPosition().position, Position(1.5, 0.0, 10.0));
    EXPECT_EQ(vehicle.currentPosition().measuredAt, Timestamp{1'750ms});
}

TEST(SimulatedVehicle,
     GivenAReachableDestination_WhenOneMovementStepRuns_ThenTheVehicleArrivesWithoutOvershoot)
{
    SimulatedVehicle vehicle{vehicleConfiguration()};
    const Position destination{0.3, 0.4, 10.0};

    vehicle.moveToward(destination, 1s);

    EXPECT_EQ(vehicle.currentPosition().position, destination);
    EXPECT_EQ(vehicle.currentPosition().measuredAt, Timestamp{2'000ms});
}

TEST(SimulatedVehicle,
     GivenTheCurrentPositionAsDestination_WhenMovementRuns_ThenOnlySimulationTimeAdvances)
{
    SimulatedVehicle vehicle{vehicleConfiguration()};

    vehicle.moveToward(Position{0.0, 0.0, 10.0}, 250ms);

    EXPECT_EQ(vehicle.currentPosition().position, Position(0.0, 0.0, 10.0));
    EXPECT_EQ(vehicle.currentPosition().measuredAt, Timestamp{1'250ms});
}

TEST(SimulatedVehicle, GivenInvalidSpeedOrTimeStep_WhenUsed_ThenItIsRejected)
{
    auto zeroSpeed = vehicleConfiguration();
    zeroSpeed.maximumSpeedMetersPerSecond = 0.0;
    EXPECT_THROW((SimulatedVehicle{zeroSpeed}), std::invalid_argument);

    auto nonFiniteSpeed = vehicleConfiguration();
    nonFiniteSpeed.maximumSpeedMetersPerSecond = std::numeric_limits<double>::infinity();
    EXPECT_THROW((SimulatedVehicle{nonFiniteSpeed}), std::invalid_argument);

    SimulatedVehicle vehicle{vehicleConfiguration()};
    EXPECT_THROW(vehicle.moveToward(Position{1.0, 0.0, 10.0}, 0ms), std::invalid_argument);
}

} // namespace
