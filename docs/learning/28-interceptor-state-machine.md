# Interceptor state machine ports

**Concept:** DDS callbacks report that distributed data arrived; they are not a safe place to run a
vehicle state machine. A deterministic core can validate each transition and hand slow work to
explicit ports without blocking a middleware thread.

**In this project:** `InterceptorStateMachine` is in `interceptor_core`, which depends only on
`drone_domain`. On startup it reads one `PositionSample` through `PositioningPort`, constructs the
drone's available state, and sends it through `DroneStateOutputPort`. `FlightControlPort` marks the
future movement boundary without adding movement behavior yet. DDS and simulated-vehicle types do
not appear in these interfaces.

**Try it:** build the focused test and run it directly:

```bash
cmake --build --preset development --target interceptor_core_test
ctest --preset development -R '^InterceptorCore\.' --output-on-failure
```

The tests show the initial available state, its single output-port report, and the absence of a
flight-control request.

**Takeaway:** middleware adapters can deliver inputs and publish outputs while the interceptor's
state and transition rules remain deterministic, testable, and reusable with physical vehicle
adapters.
