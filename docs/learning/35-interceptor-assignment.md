# 35 — Applying an assignment in interceptor core

**Concept:** Reliable delivery can produce retransmissions, and distributed actors may repeat an
intent when they do not know whether it was accepted. An interceptor therefore needs idempotent
handling: the same assignment has no second effect, while a different assignment that conflicts
with current state is rejected explicitly.

**In this project:** `InterceptorStateMachine` implements `AssignmentInputPort`. It ignores values
addressed to another drone, reports an exact repeat as a duplicate, and reports a different target
after assignment as conflicting. A valid assignment refreshes the positioning sample, remembers the
target, transitions from available to assigned, and publishes a correlated `DroneState`. The core
still has no DDS or simulation dependency.

**Try it:** run the core cases and DDS-level state observation:

```bash
cmake --build --preset development --target interceptor_core_test assignment_dds_adapter_test
ctest --preset development -R '^(InterceptorCore|AssignmentDdsAdapter)\.' --output-on-failure
```

The integration case sends an assignment over `drone.assignments`, lets the adapter call the core,
and takes the resulting assigned state from `drone.drone_states`.

**Takeaway:** DDS supplies a delivered value, while the state machine decides whether that value is
for this drone, new, repeated, or conflicting—and only the valid transition becomes new published
state.
