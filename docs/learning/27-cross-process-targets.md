# 27 — Cross-process targets

## Concept

Moving a DDS writer and reader into separate processes changes ownership and failure boundaries, not
the data contract. Each process owns its own `DomainParticipant` and endpoints. Fast DDS
[participant and endpoint discovery](https://fast-dds.docs.eprosima.com/en/3.3.x/fastdds/discovery/discovery.html)
finds compatible peers in the same domain, after which the target's IDL wire type, Topic name, and
QoS determine the data path just as they did in one process.

A process test needs observable readiness because operating-system scheduling and DDS discovery are
asynchronous. A fixed startup delay can be too short on a slow machine and wastes time on a fast one.
Waiting on the reader's publication-match condition instead makes the test advance when discovery
actually completes.

## In this project

`observer_console_process_test` starts the `console` and `observer` executables as two child
processes in DDS domain 188. They share no application objects: the observer reaches the console
only through the `drone.target_tracks` Fast DDS Topic.

The console composition loop calls `TargetTrackSubscriber::waitForWriterMatch` before receiving data
and emits `console: matched observer TargetTrack writer` when Fast DDS reports a compatible writer.
The test captures both processes' standard output and error streams, waits for that discovery marker,
then waits for two rendered target rows. It parses the rows and proves that at least two positions
differ.

All waits have deadlines. Shared `process_test_support` terminates and reaps both children even when
an assertion fails, escalates from `SIGTERM` to `SIGKILL` after a bounded grace period, and includes
the captured process logs in failure output. The CTest case also has an overall 15-second timeout.
No fixed startup sleep stands in for DDS discovery or data readiness.

## Try it

Run the focused separate-process scenario from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target observer_console_process_test
ctest --preset development -R '^CrossProcessTargets\.' -V
```

For a manual version, start `./build/development/console 0` in one terminal and then run
`./build/development/observer 0 20` in another. The console first reports its empty projection, then
its DDS writer match, followed by changing positions for target 1.

## Takeaway

DDS discovery is the readiness signal between independently scheduled processes. The observer and
console now demonstrate the same typed publish/subscribe path across a real process boundary, with
bounded automation that exposes discovery, delivered state, failures, and cleanup.
