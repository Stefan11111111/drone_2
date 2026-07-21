# 41 — Arrival and the interception-effect port

## Concept

Reaching a target and producing a physical effect are different responsibilities. The interceptor
core can decide that its reported position is close enough to the latest target position, but it
must not know whether the effect is simulated or produced by future hardware. A replaceable output
port keeps that device-specific action outside the state machine.

## In this project

`InterceptorStateMachine::tick()` requests bounded movement, reads the resulting position, and
measures its three-dimensional distance from the latest assigned target. Its explicit
`arrivalToleranceMeters` configuration defines arrival; the interceptor executable currently uses
0.25 metres.

On arrival, core calls `InterceptionEffectPort::trigger()` exactly once. `SimulatedVehicle`
implements that port with a successful simulated effect, while core tests substitute both success
and failure results. The state machine publishes `interceptionSucceeded` or `interceptionFailed`
and leaves the intercepting state, so later ticks cannot repeat the effect. The core includes no
simulation or DDS types.

## Try it

Run the deterministic core and simulated-adapter experiments:

```bash
cmake --build --preset development --target interceptor_core_test simulated_vehicle_adapter_test
ctest --preset development -R '^(InterceptorCore|SimulatedVehicle)\.' --output-on-failure
```

The core cases keep the vehicle outside tolerance, place it just inside tolerance, tick again after
success, and substitute a failed effect. The adapter case calls the same port through
`SimulatedVehicle` to show which replaceable boundary supplies the demonstration behavior.

## Takeaway

Core decides *when* an interception has arrived; an adapter decides *how* to perform the effect.
The explicit port and terminal outcome state make the one-shot behavior testable without coupling
the application to simulation or physical equipment.
