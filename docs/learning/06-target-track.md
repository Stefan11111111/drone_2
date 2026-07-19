# 06 — Target track

## Concept

A data-centric system shares facts about the world as data. An observer publishes the latest known
state of a target; it does not invoke a function inside the console or interceptor. Those consumers
can interpret the same target state independently and at their own pace.

A target track is one measured snapshot: which target was observed, where it was, and when that
position was measured. The timestamp lets future consumers distinguish a newer observation from a
stale one. DDS distribution and stale-update policy arrive in later steps; this step defines only
the participant-neutral fact they will carry.

## In this project

`include/drone/domain/target_track.h` defines `TargetTrack` from the existing `TargetId`, `Position`,
and `Timestamp` values. The observer can produce it, the console can eventually project it, and an
interceptor can eventually follow it without any role owning a middleware, radar, UI, or vehicle
type.

The constituent values already reject a zero identifier, non-finite coordinates, and time before
the Unix epoch. `TargetTrack` adds no artificial cross-field restriction: every valid target may be
measured at every valid position and time. Step 06 intentionally adds no IDL, generated type,
serialization, Topic, DataWriter, or DataReader.

## Try it

Build and run only the target-track tests from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target target_track_test
ctest --preset development -R '^TargetTrack\.'
```

The first test makes the three parts of a measurement visible through the value's accessors. The
second changes one part at a time to show that identity, position, and measurement time all affect
snapshot equality.

## Takeaway

The shared contract is a self-contained statement of observed target state. Later DDS code will
distribute this value between processes, but the operational meaning does not depend on how it is
transported or which kind of sensor produced it.
