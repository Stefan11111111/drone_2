# 42 — Publishing the explosion event

## Concept

A successful local effect becomes useful to the rest of the distributed system only when it is
translated into an operational event and published. The effect adapter reports success to core;
core supplies the drone, target, position, time, and stable event identity; the DDS adapter then
maps that domain value to its IDL type and writes it.

## In this project

On an accepted start command, `InterceptorStateMachine` reserves an `ExplosionEventId` with the
same numeric identity as the unique command. When the one-shot effect succeeds, core publishes an
`ExplosionEvent` through `ExplosionEventOutputPort`. A failed effect publishes only failed drone
state and no event.

`ExplosionEventPublisher` implements that core port. It owns an `ExplosionEventWriter`, whose
`ExplosionEventTopic` centralizes the catalog name and the
`RELIABLE / TRANSIENT_LOCAL / KEEP_LAST(1)` policy with limits of 256 event instances and samples.
The writer uses the existing explicit domain-to-wire mapping before calling Fast DDS.

## Try it

Run the focused boundary experiment:

```bash
cmake --build --preset development --target explosion_event_dds_adapter_test
ctest --preset development -R '^ExplosionEventDdsAdapter\.' --output-on-failure
```

The test completes one interception before creating its probe reader. Receiving the event after
that late match makes `TRANSIENT_LOCAL` history observable while the writer remains alive. A second
bounded read finds no sample, demonstrating that repeated ticks did not publish another event. A
separate assertion checks every catalog QoS field on both endpoint configurations.

## Takeaway

The operational event crosses three explicit boundaries: simulated effect result to core, domain
event to DDS adapter, and mapped wire sample to Fast DDS. That path preserves correlation facts and
keeps physical-effect details out of the distributed contract.
