# 09 — Explosion event

## Concept

State describes what is true now and is normally replaced by a newer snapshot for the same object.
An event records that something happened at a particular place and time. A later drone-state sample
can replace an earlier one, but it does not make a completed interception occurrence cease to have
happened.

Distributed delivery can expose the same event more than once, so an event has its own identity.
That identity will let a future console recognize a duplicate without treating two different
interceptions as one. The event also names the drone and target so the outcome can be correlated
with their projected state even when updates arrive asynchronously.

## In this project

`include/drone/domain/explosion_event.h` defines `ExplosionEvent` from an `ExplosionEventId`,
`DroneId`, `TargetId`, `Position`, and occurrence `Timestamp`. Together these facts say which
occurrence happened, which drone and target it involved, and where and when it happened. They do
not expose a simulated effect, vehicle, radar, UI, or DDS type, so the same operational event can
describe a future hardware-backed interception.

The constituent domain values enforce the event's representational invariants: all identities are
nonzero, coordinates are finite, and occurrence time cannot precede the Unix epoch. The event adds
no speculative state-machine rule; later interceptor-core code will decide when it is valid to emit
one. This step adds no IDL, Topic, QoS, DataWriter, or DataReader.

## Try it

Build and run only the explosion-event tests from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target explosion_event_test
ctest --preset development -R '^ExplosionEvent\.'
```

The tests change each field independently to make the event's value semantics observable. They
also reject zero event, drone, and target identities, a non-finite position, and a time before the
epoch. Compare the event with the latest-state contracts by running:

```bash
ctest --preset development -R '^(TargetTrack|DroneState|ExplosionEvent)\.'
```

## Takeaway

A state sample answers “what is true now?”, while an event answers “what happened?”. A distinct
event identity supports duplicate handling, and explicit drone and target identities let a console
correlate the outcome without depending on in-process objects or simulation-specific details.
