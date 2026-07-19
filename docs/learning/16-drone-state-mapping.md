# 16 — Drone-state mapping

## Concept

A generated schema API changes when its IDL changes, while core application code should depend on
stable domain meaning. A mapping layer absorbs differences in field names, optional-value APIs, and
enum spelling. Explicit enum conversion is especially important: matching numeric values today does
not make `static_cast` a safe contract for future schema evolution.

## In this project

`drone_state_mapping.cpp` converts between the generated `drone::dds::DroneState` and the
participant-neutral `drone::domain::DroneState`. Both mapping directions name all five lifecycle
statuses individually. The transport also translates Fast CDR's generated optional target into
`std::optional<TargetId>` rather than exposing the middleware representation to core code.

Received wire state is accepted only when its drone ID is nonzero, its coordinates are finite, its
timestamp is not before the Unix epoch, its status is known, and its optional target agrees with the
lifecycle status. Failures return a specific `DroneStateMappingError`. No DDS entity is created in
this step; the mapping only protects the boundary between generated and domain values.

## Try it

Build and run the focused mapping tests:

```bash
cmake --preset development
cmake --build --preset development --target drone_state_mapping_test
ctest --preset development -R '^DroneStateMapping\.'
```

The first test iterates over every domain status and checks both its exact wire enumerator and a
complete round trip. The malformed cases show the boundary rejecting an unknown generated enum and
status/assignment combinations that the domain model cannot represent.

## Takeaway

The mapping layer makes every lifecycle meaning deliberate and contains generated schema mechanics.
Core code receives only validated domain state, even when a wire sample carries an unknown enum or
an inconsistent optional field.
