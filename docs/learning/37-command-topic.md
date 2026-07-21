# 37 — Interception command Topic

**Concept:** Reliability retransmits missing transport data; it does not make an application action
exactly once. A stable command identity lets the receiving adapter recognize the same intent after
retransmission without invoking core twice. Volatile durability also avoids replaying an old start
command to an interceptor that joins later.

**In this project:** `InterceptionCommandTopic` centralizes
`drone.interception_commands` with the catalog's `RELIABLE / VOLATILE / KEEP_LAST(1)` policy and
limits for 256 command-keyed instances. The console publisher maps and writes domain commands. The
interceptor subscriber takes on its caller's thread, rejects malformed data, remembers delivered
`InterceptionCommandId` values, and forwards only the first occurrence through
`InterceptionCommandInputPort`.

**Try it:** run the focused adapter cases:

```bash
cmake --build --preset development --target interception_command_dds_adapter_test
ctest --preset development -R '^InterceptionCommandDdsAdapter\.' --output-on-failure
```

The cases verify one delivery, a deliberate retransmission reported as duplicate without a second
core call, and a bounded no-data wait that permits clean subscriber destruction.

**Takeaway:** QoS makes delivery dependable, while command identity and application policy make a
repeated delivery safe. Neither property proves that the interceptor actually began moving.
