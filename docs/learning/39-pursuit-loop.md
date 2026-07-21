# 39 — Deterministic pursuit loop

**Concept:** DDS updates arrive asynchronously, while vehicle control is easier to reason about as a
deterministic loop. The reader replaces the latest target snapshot whenever a newer sample arrives;
each explicit core tick uses whichever snapshot is current at that instant.

**In this project:** a correlated start command moves `InterceptorStateMachine` from assigned to
intercepting and publishes that transition. `tick(duration)` does nothing before interception,
waits safely when no target sample has arrived, or asks `FlightControlPort` to move toward the latest
target position. It then reads `PositioningPort` and publishes the resulting intercepting
`DroneState`. A newer track between ticks changes the next destination without changing DDS code.

**Try it:** run the focused state-machine tests:

```bash
cmake --build --preset development --target interceptor_core_test
ctest --preset development -R '^InterceptorCore\.' --output-on-failure
```

The cases cover invalid start requests, starting before the first asynchronous target sample, one
bounded movement request, state reporting, and a newer target that changes the following tick's
destination.

**Takeaway:** DDS supplies the latest shared observation; the core owns when control work happens.
That split keeps middleware callbacks short and makes pursuit timing, movement, and retargeting
fully deterministic in tests.
