# Console drone-state projection

**Concept:** One DomainParticipant can contain several independent DataReaders. Each reader matches
only endpoints with the same Topic name, wire type, and compatible QoS; receiving target state does
not couple it to receiving drone state.

**In this project:** `ConsoleSubscriber` owns one console participant with both a
`TargetTrackReader` and `DroneStateReader`. Samples cross separate console-core input ports into
`TargetProjection` and `DroneProjection`. `TerminalView` receives snapshots of both projections and
renders target and drone tables outside the DDS receive path.

**Try it:** run the three-process scenario:

```bash
cmake --build --preset development --target live_system_state_process_test
ctest --preset development -R '^LiveSystemState\.' --output-on-failure
```

The test waits for the console to construct both readers, launches the observer and interceptor,
then requires two distinct match logs and rendered state rows before bounded child cleanup.

**Takeaway:** a participant is an application's presence in a DDS domain, not a single Topic
connection. The console can build one application view from multiple independent data streams while
keeping projection and rendering free of middleware types.
