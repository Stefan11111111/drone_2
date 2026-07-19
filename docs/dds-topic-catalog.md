# DDS topic catalog

This catalog is the source of truth for the initial DDS topic names, wire-type names, keys, and
non-default QoS choices. Later transport code must use these values without adding role-specific
variants. All DataWriters and DataReaders for a row use the listed QoS unless a later documented
experiment changes the row.

## Initial topics

`max_instances / max_samples / max_samples_per_instance` below are the corresponding
`ResourceLimitsQosPolicy` values. `KEEP_LAST(1)` applies independently to each keyed instance.

| Topic name | Purpose and update pattern | Wire type | Key | Producer | Consumers | Reliability / durability / history | Resource limits |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `drone.target_tracks` | Latest measured target position; periodic and on changed detections | `drone::dds::TargetTrack` | `target_id` | Observer | Console, interceptor | `RELIABLE / TRANSIENT_LOCAL / KEEP_LAST(1)` | `64 / 64 / 1` |
| `drone.drone_states` | Latest interceptor position and lifecycle; on startup, movement, and transition | `drone::dds::DroneState` | `drone_id` | Interceptor | Console | `RELIABLE / TRANSIENT_LOCAL / KEEP_LAST(1)` | `16 / 16 / 1` |
| `drone.assignments` | Operator's latest drone-to-target assignment; on a validated assignment | `drone::dds::Assignment` | `drone_id` | Console | Interceptor | `RELIABLE / VOLATILE / KEEP_LAST(1)` | `16 / 16 / 1` |
| `drone.interception_commands` | Operator intent to start one assigned interception; once per validated command | `drone::dds::InterceptionCommand` | `command_id` | Console | Interceptor | `RELIABLE / VOLATILE / KEEP_LAST(1)` | `256 / 256 / 1` |
| `drone.explosion_events` | Completed interception occurrence; once per successful effect | `drone::dds::ExplosionEvent` | `event_id` | Interceptor | Console | `RELIABLE / TRANSIENT_LOCAL / KEEP_LAST(1)` | `256 / 256 / 1` |

These five rows are the complete initial cross-process contract: observed target state, interceptor
state, the two distinct operator actions, and the correlated outcome. The names and types describe
operational data only. Radar, terminal UI, simulated vehicle, and effect implementation details do
not cross the DDS boundary.

## Why these keys and policies

A key separates samples on one Topic into logical instances. Target and drone identifiers therefore
make each physical subject its own state stream. An assignment is the current intent for one drone,
so it uses the drone identifier. Commands and explosion events represent distinct occurrences, so
their own identities prevent two occurrences from collapsing into one instance and support
application-level duplicate detection.

All initial flows are low-volume and affect the visible operational result, so readers request
`RELIABLE` delivery. This asks Fast DDS to detect and retransmit missing samples; it does not mean
the receiving application accepted a command or performed an effect. The target update rate may
eventually justify `BEST_EFFORT`, but the reliable choice remains a hypothesis until the moving
target experiment measures its behavior.

Target and drone snapshots use `TRANSIENT_LOCAL` so a late-matching reader can receive each living
writer's latest state. Explosion events also use `TRANSIENT_LOCAL`, allowing a console that joins
after an outcome to see it while the interceptor writer remains alive. This is not process restart
persistence: no sample survives destruction of its writer. Assignments and interception commands
use `VOLATILE` because replaying old operator intent into a restarted interceptor could initiate an
action without fresh validation.

`KEEP_LAST(1)` retains only the newest sample for each key. That matches projected state and is also
enough for command or event keys, because each identity is immutable. The fixed resource limits make
the demonstration's capacity explicit and keep history bounded. Reaching a cap must be reported as
an operational error; transport code must not silently discard a new instance or manufacture a
valid value. The limits of 64 targets, 16 drones/assignments, and 256 commands/events are initial
scenario bounds to confirm under the later resource-limit experiments.

All unlisted QoS policies keep the Fast DDS 3.3.0 defaults. In particular, this step proposes no
deadline, lifespan, ownership, transport priority, or middleware timestamp ordering. Domain
timestamps and command/event identities remain responsible for stale and duplicate decisions in
application code.

## Matching and experiments

A writer and reader can exchange samples only after discovery finds compatible endpoints. They must
agree on the Topic name and registered wire type, and the writer's offered Reliability and
Durability must satisfy the reader's requested policies. Each endpoint will use the same row by
default; later learning steps deliberately vary policies only to make matching behavior observable.

The hypotheses requiring evidence are:

- step 20: compatible and intentionally incompatible endpoint matching;
- step 27: reliable, retained target updates across the first process boundary;
- step 32: drone-state behavior when the interceptor or console starts first;
- step 43: visibility of a retained outcome to a restarting console while the writer lives; and
- step 46: Reliability, Durability, History, incompatible-QoS, and resource-limit behavior before
  these policies are considered final.

The policy semantics and consistency rules are documented in the Fast DDS 3.3.0
[standard QoS policies](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/core/policy/standardQosPolicies.html).
Fast DDS also explains how keys divide a Topic into
[instances](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/topic/instances.html).
