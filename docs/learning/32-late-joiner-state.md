# Late-joining drone-state reader

**Concept:** Durability controls whether a newly matched reader can receive data written before it
matched. `TRANSIENT_LOCAL` keeps samples in the living writer; `KEEP_LAST(1)` keeps the newest sample
for each drone key. It does not persist anything after that writer process exits.

**In this project:** `drone.drone_states` offers and requests
`RELIABLE / TRANSIENT_LOCAL / KEEP_LAST(1)`. `LateJoinerDroneState` runs the console-first order to
observe live delivery, then runs the interceptor-first order after waiting for the interceptor's
post-write log. The late console still projects and renders drone 1 because the same interceptor
writer remains alive with its retained keyed sample.

**Try it:** run both start-order experiments:

```bash
cmake --build --preset development --target late_joiner_drone_state_process_test
ctest --preset development -R '^LateJoinerDroneState\.' --output-on-failure
```

Both tests use readiness logs, DDS matching, bounded output waits, and child cleanup rather than
fixed startup sleeps. As a manual contrast, terminate the interceptor before launching the console:
there is then no living transient-local writer from which to retrieve the sample.

**Takeaway:** transient-local durability handles a late reader while the writer lives; history depth
selects how much that writer retains, and neither setting is durable storage across process restarts.
