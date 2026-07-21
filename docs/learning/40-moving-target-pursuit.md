# 40 — Moving-target pursuit across processes

**Concept:** DDS producers and consumers run at independent rates. Target samples arrive
asynchronously, while the interceptor's 100 ms control loop uses the latest accepted keyed sample
on each tick. A newer sample does not interrupt flight control; it changes the destination chosen by
the next deterministic tick.

**In this project:** the console process can enable a narrow `--auto-pursuit` orchestration mode for
tests. It waits for projected target and available-drone state plus matched command readers, then
uses the same console-core assignment and start use cases as an operator action. The interceptor
process receives assignments, target tracks, and commands through their DDS adapters, advances the
`SimulatedVehicle`, calls `InterceptorStateMachine::tick(100ms)`, and publishes each moved state.
Observer, console, and interceptor remain separate processes and share no core implementation.

**Try it:** run the bounded process scenario:

```bash
cmake --build --preset development --target moving_target_pursuit_process_test
ctest --preset development -R '^MovingTargetPursuit\.' --output-on-failure
```

The test waits on readiness and action logs rather than startup sleeps. It parses at least eight
pursuit ticks, finds movement vectors driven by different target timestamps and positions, and
asserts that their directions differ before cleaning up all child processes.

**Takeaway:** Fast DDS keeps the roles synchronized through samples, discovery, and QoS—not through
shared calls or a global clock. The application still owns input validation, latest-sample policy,
control cadence, movement bounds, and evidence that a new observation changed behavior.
