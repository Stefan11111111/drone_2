# 23 — Observer publisher

## Concept

A DDS `DataWriter` publishes typed samples to a Topic. The observer core does not call Fast DDS
directly: it calls its `TargetTrackOutputPort`, and an output adapter turns that application-level
call into a write of the generated wire type. The existing transport mapping performs the explicit
domain-to-wire conversion before Fast DDS serializes the sample.

`TargetTrack` is keyed by `target_id`. Repeated writes for target 1 are therefore updates to one DDS
instance rather than unrelated messages. The Topic catalog's `KEEP_LAST(1)` policy describes the
latest retained state for that keyed instance, while readers that are already following the stream
can take each successive update.

## In this project

`observer_dds_adapter::TargetTrackPublisher` implements the observer core's
`TargetTrackOutputPort`. Its direct RAII members own one transport participant and its target-track
writer. The writer owns the endpoints and delegates mapping, the `drone.target_tracks` name, and QoS
to the existing transport support. This keeps Fast DDS and generated types outside `observer_core`.

The `observer` executable is the composition root. It configures the DDS adapter and wires it to
`TargetTracker` and `SimulatedRadar`, then schedules explicit radar ticks. The adapter owns its one
participant and writer. Its default domain is 0 and it runs continuously; optional positional
`domain-id` and `tick-count` arguments make a bounded probe possible. It contains no tracking or DDS
mapping rules.

The `ObserverPublisher` process test starts that executable separately, creates a probe reader in
the same domain, waits for DDS endpoint matching, and then waits for three data notifications. It
asserts that the key stays the same while measurement times and positions advance. Every wait is
bounded, and the child is terminated and reaped even when an assertion fails.

## Try it

Run the focused process experiment from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target observer_publisher_process_test
ctest --preset development -R '^ObserverPublisher\.' -V
```

The verbose output shows the separate observer publishing updates. To experiment manually with a
finite publisher in DDS domain 0, run `./build/development/observer 0 20` while another target-track
reader is active. Changing the domain ID on only one side prevents discovery and the process test's
match wait reports a bounded, specific failure.

## Takeaway

One observer port call now crosses the adapter boundary, becomes a generated keyed sample, and is
distributed by Fast DDS. The observer's tracking behavior remains ordinary middleware-independent
C++, while the adapter owns the decision to publish that state through DDS.
