# 07 — Drone state

## Concept

Published state is a self-contained contract, not a view of an object living inside the interceptor
process. A receiving console cannot inspect that object's fields or call its methods; it can only
interpret the state snapshots that cross the process boundary. Each snapshot therefore carries the
facts needed to identify the drone, place it, order its reports, and explain its lifecycle.

The lifecycle vocabulary distinguishes an available drone, an assigned drone, an interceptor in
motion, and successful or failed outcomes. These are operational terms shared by simulated and
future physical vehicles. DDS transport, serialization, and state-machine transitions arrive in
later steps.

## In this project

`include/drone/domain/drone_state.h` defines `DroneState` from `DroneId`, `Position`, `Timestamp`, an
optional assigned `TargetId`, and `DroneStatus`. The five statuses are `available`, `assigned`,
`intercepting`, `interceptionSucceeded`, and `interceptionFailed`. Position changes make movement
visible while `reportedAt` lets the future console reject stale snapshots from the same drone.

An available drone cannot name a target; every later lifecycle status must name the target to which
it relates. `DroneState` enforces only that representational invariant. The interceptor core added
later will own transition rules such as whether an assigned drone may begin intercepting. This step
adds no IDL, generated type, Topic, DataWriter, or DataReader.

## Try it

Build and run only the drone-state tests from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target drone_state_test
ctest --preset development -R '^DroneState\.'
```

The tests construct every lifecycle status, reject inconsistent status/target combinations, and
change each published field independently to demonstrate snapshot value semantics.

## Takeaway

A consumer needs a complete, immutable-by-value snapshot rather than access to an interceptor's
in-process state. That contract keeps the console and future DDS schema independent of simulation
objects while still exposing availability, assignment, movement, and outcome.
