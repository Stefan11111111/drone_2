#include "drone/console_core/drone_projection.h"
#include "drone/console_core/outcome_projection.h"
#include "drone/console_core/target_projection.h"
#include "drone/console_dds_adapter/explosion_event_subscriber.h"
#include "drone/console_ui_adapter/terminal_view.h"
#include "drone/dds_transport/domain_participant_owner.h"
#include "drone/dds_transport/explosion_event_writer.h"
#include "drone/domain/drone_id.h"
#include "drone/domain/drone_state.h"
#include "drone/domain/explosion_event.h"
#include "drone/domain/explosion_event_id.h"
#include "drone/domain/position.h"
#include "drone/domain/target_id.h"
#include "drone/domain/target_track.h"
#include "drone/domain/timestamp.h"

#include <fastdds/dds/core/Types.hpp>

#include <chrono>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace
{

using drone::console::DroneProjection;
using drone::console::OutcomeProjection;
using drone::console::OutcomeUpdateResult;
using drone::console::TargetProjection;
using drone::console_dds::ExplosionEventSubscriber;
using drone::console_ui::TerminalView;
using drone::dds_transport::DomainParticipantOwner;
using drone::dds_transport::ExplosionEventWriter;
using drone::domain::DroneId;
using drone::domain::DroneState;
using drone::domain::DroneStatus;
using drone::domain::ExplosionEvent;
using drone::domain::ExplosionEventId;
using drone::domain::Position;
using drone::domain::TargetId;
using drone::domain::TargetTrack;
using drone::domain::Timestamp;
using eprosima::fastdds::dds::DomainId_t;
using namespace std::chrono_literals;

constexpr DomainId_t outcomeDeliveryDomainId{194};
constexpr DomainId_t restartingConsoleDomainId{195};
constexpr auto discoveryTimeout{5s};
constexpr auto dataTimeout{5s};

void seedCorrelation(TargetProjection &targets, DroneProjection &drones)
{
    ASSERT_EQ(targets.onTargetTrack(
                  TargetTrack{TargetId{42}, Position{10.0, 20.0, 30.0}, Timestamp{1'000ms}}),
              drone::console::TargetUpdateResult::added);
    ASSERT_EQ(
        drones.onDroneState(DroneState{DroneId{7}, Position{10.0, 20.0, 30.0}, Timestamp{2'000ms},
                                       DroneStatus::interceptionSucceeded, TargetId{42}}),
        drone::console::DroneUpdateResult::added);
}

[[nodiscard]] ExplosionEvent correlatedEvent()
{
    return {ExplosionEventId{101}, DroneId{7}, TargetId{42}, Position{10.0, 20.0, 30.0},
            Timestamp{2'000ms}};
}

TEST(ConsoleOutcomeDdsAdapter,
     GivenValidDuplicateAndUnrelatedEvents_WhenDeliveredByDds_ThenOnlyCorrelatedOutcomeIsRendered)
{
    TargetProjection targets;
    DroneProjection drones;
    seedCorrelation(targets, drones);
    OutcomeProjection outcomes{targets, drones};
    ExplosionEventSubscriber subscriber{outcomeDeliveryDomainId, "drone_step_43_console", outcomes};
    DomainParticipantOwner writerParticipant{outcomeDeliveryDomainId, "drone_step_43_interceptor"};
    ExplosionEventWriter writer{writerParticipant.participant()};
    ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout));

    const auto event = correlatedEvent();
    writer.write(event);
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), OutcomeUpdateResult::recorded);
    writer.write(event);
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), OutcomeUpdateResult::duplicate);
    writer.write(ExplosionEvent{ExplosionEventId{102}, DroneId{7}, TargetId{43},
                                Position{10.0, 20.0, 30.0}, Timestamp{2'000ms}});
    ASSERT_EQ(subscriber.receiveNext(dataTimeout), OutcomeUpdateResult::unrelated);

    std::ostringstream output;
    TerminalView view{output};
    view.render(targets, drones, outcomes);

    EXPECT_EQ(outcomes.outcomes().size(), 1U);
    EXPECT_NE(output.str().find("Interception outcomes (1)\n"), std::string::npos);
    EXPECT_NE(output.str().find("101 | 7 | 42 | 10.00 | 20.00 | 30.00 | 2000\n"),
              std::string::npos);
}

TEST(ConsoleOutcomeDdsAdapter,
     GivenAWriterRemainsAlive_WhenTheConsoleReaderRestarts_ThenRetainedOutcomeIsCorrelatedAgain)
{
    DomainParticipantOwner writerParticipant{restartingConsoleDomainId,
                                             "drone_step_43_retained_writer"};
    ExplosionEventWriter writer{writerParticipant.participant()};
    {
        TargetProjection targets;
        DroneProjection drones;
        seedCorrelation(targets, drones);
        OutcomeProjection outcomes{targets, drones};
        ExplosionEventSubscriber subscriber{restartingConsoleDomainId,
                                            "drone_step_43_first_console", outcomes};
        ASSERT_TRUE(writer.waitForReaderMatch(discoveryTimeout));
        writer.write(correlatedEvent());
        ASSERT_EQ(subscriber.receiveNext(dataTimeout), OutcomeUpdateResult::recorded);
    }

    TargetProjection restartedTargets;
    DroneProjection restartedDrones;
    seedCorrelation(restartedTargets, restartedDrones);
    OutcomeProjection restartedOutcomes{restartedTargets, restartedDrones};
    ExplosionEventSubscriber restartedSubscriber{
        restartingConsoleDomainId, "drone_step_43_restarted_console", restartedOutcomes};

    ASSERT_TRUE(restartedSubscriber.waitForWriterMatch(discoveryTimeout));
    EXPECT_EQ(restartedSubscriber.receiveNext(dataTimeout), OutcomeUpdateResult::recorded);
    EXPECT_EQ(restartedOutcomes.outcomes(), (std::vector{correlatedEvent()}));
}

} // namespace
