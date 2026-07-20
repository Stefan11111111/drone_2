# 22 — Simulated radar

## Concept

A simulator is an adapter when it translates a replaceable source of observations into the input
port understood by the application core. Here, one explicit call advances a scenario by one fixed
time step. No thread, timer, or wall clock is hidden inside the adapter, so the same scenario always
produces the same measurements.

This deterministic boundary is useful in a DDS application even though this step does not publish
anything yet. The observer core and the eventual DDS output see only participant-neutral
`Detection` and `TargetTrack` values; neither contract reveals whether a simulated radar or physical
sensor supplied the measurement.

## In this project

`SimulatedRadar` lives in `simulated_radar_adapter` and depends on the observer's
`DetectionInputPort`. Its `Scenario` makes the target identifier, initial position, start time,
velocity, and tick interval explicit. Each `tick()` computes position from scenario time and sends
one timestamped `Detection` through the port.

The caller owns scheduling. Tests call `tick()` directly, while a later observer executable can use
the same operation from its runtime loop. The adapter links to `observer_core`; the core has no
dependency back on simulation code.

## Try it

Run the focused adapter tests from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target simulated_radar_adapter_test
ctest --preset development -R '^SimulatedRadar\.'
```

The moving-target case starts at `(10, 20, 30)` metres, advances in 500 ms steps at
`(2, -4, 1)` metres per second, and checks all three emitted positions and measurement times. Change
the velocity or interval in `movingTargetScenario()` and the expected sequence will change without
waiting for real time.

## Takeaway

Deterministic simulation comes from explicit scenario data and explicit time advancement. The
observer input contract stays independent of radar hardware, so a real sensor adapter can replace
this simulator without changing the observer core or the DDS-facing output path.
