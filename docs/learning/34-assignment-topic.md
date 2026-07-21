# 34 — Assignment Topic

**Concept:** Reliable DDS delivery detects and retransmits missing transport samples, but it does
not prove that the interceptor accepted or acted on an assignment. The assignment Topic is volatile,
so only readers matched while the writer is alive receive the operator's current intent; a late or
restarted interceptor does not replay an old assignment automatically.

**In this project:** `AssignmentTopic` centralizes the catalog name `drone.assignments` and its
`RELIABLE / VOLATILE / KEEP_LAST(1)` QoS with capacity for 16 drone-keyed instances.
`AssignmentPublisher` implements the console-core output port and writes mapped domain values.
`AssignmentSubscriber` waits and takes on its caller's thread before invoking
`AssignmentInputPort`; its Fast DDS listener only records discovery matches, so core work never runs
inside a middleware callback.

**Try it:** run the focused adapter test:

```bash
cmake --build --preset development --target assignment_dds_adapter_test
ctest --preset development -R '^AssignmentDdsAdapter\.' --output-on-failure
```

The test waits for endpoint matching, publishes one assignment from the console adapter, and checks
that a capturing interceptor input receives the same domain value within a bounded wait.

**Takeaway:** matching QoS and reliable delivery move an assignment across the process boundary;
application validation, acknowledgement, duplicate handling, and state changes remain separate
responsibilities.
