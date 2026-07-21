# 44 — The complete DDS interception path

## Concept

An end-to-end DDS scenario is a chain of independently useful boundaries, not one remote function
call. Discovery connects compatible endpoints. Writers serialize domain intent or state through IDL
types. Readers take and validate samples. Adapters hand domain values into deterministic core logic,
and the UI renders a projection built from those values.

## In this project

`CompleteVisionScenario` starts the observer, console, and interceptor executables as separate child
processes on one explicit DDS domain. It waits for semantic milestones with bounded timeouts rather
than sleeping for assumed discovery delays:

1. The observer turns a simulated detection into a domain `TargetTrack`; its adapter maps that value
   to the IDL-generated `drone::dds::TargetTrack` and writes `drone.target_tracks`.
2. Discovery matches the console and interceptor readers by Topic name, generated type, and QoS.
   Their adapters take and map samples into console projection and interceptor pursuit core.
3. After projected target and available drone state are visible, the automated operator path emits
   distinct assignment and start-command domain values. DDS writers serialize them; interceptor
   readers map them and call the corresponding core input ports.
4. Interceptor core consumes newer target samples on deterministic ticks and asks the simulated
   vehicle adapter to move. Drone-state snapshots travel through their IDL type and Topic so the
   console continues to see lifecycle and position.
5. Arrival makes core request the simulated effect once. Success becomes a domain `ExplosionEvent`,
   maps to the IDL-generated event, and is written on `drone.explosion_events`.
6. The console event reader takes and maps that sample. `OutcomeProjection` correlates its drone and
   target identifiers, and `TerminalView` renders the completed occurrence.

## Try it

Run only the complete scenario from the repository root:

```bash
cmake --build --preset development --target end_to_end_interception_process_test
ctest --preset development -R '^CompleteVisionScenario\.' --output-on-failure
```

The scenario is bounded at 45 seconds and normally completes in about 25 seconds with the current
1,000-metre target altitude and 50 m/s interceptor speed. Any failed milestone reports the captured
logs from the relevant processes, and the shared process harness terminates remaining children even
when an assertion exits early.

## Takeaway

The initial vision emerges from composable data paths: IDL defines wire contracts, discovery and QoS
connect endpoints, adapters protect core from middleware, deterministic state machines own behavior,
and projections drive presentation. Because the three roles share no in-process implementation, the
same application boundaries can later sit on separate machines or behind hardware adapters.
