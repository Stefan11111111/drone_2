# 33 — Assignment validation in console core

**Concept:** DDS can deliver an assignment, but it cannot decide whether an operator's selection is
valid for the application's current view. Validating before publication prevents known-invalid
intent from becoming distributed work and keeps that policy deterministic and testable.

**In this project:** `AssignmentUseCase` reads the DDS-free `TargetProjection` and
`DroneProjection`. It accepts only a known target and an available, known drone, then emits the
domain `Assignment` through `AssignmentOutputPort`. It remembers a pending selection so repeating
the same action is reported as a duplicate and does not emit a second value. No console-core file
includes Fast DDS or generated wire types.

**Try it:** run the focused console-core cases:

```bash
cmake --build --preset development --target console_core_test
ctest --preset development -R '^ConsoleCore\.' --output-on-failure
```

The assignment cases use a capturing output port to show that unknown, unavailable, and duplicate
selections produce no output, while one valid selection produces exactly one `Assignment`.

**Takeaway:** middleware delivery guarantees begin after the application publishes. Core validation
is the earlier boundary that decides whether operator intent is meaningful enough to publish at
all.
