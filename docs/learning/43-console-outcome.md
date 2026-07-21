# 43 — Consuming and correlating the outcome

## Concept

Receiving an event is not the same as accepting it into an application view. A console must map the
wire sample, recognize repeated event identities, and correlate the event's subject identifiers
with state it already understands. That keeps an unrelated or conflicting occurrence from being
presented as the result of the operator's interception.

## In this project

`ExplosionEventReader` takes valid samples with the catalog's event QoS and maps them back to the
domain. `ExplosionEventSubscriber` hands each value to `OutcomeProjection` outside the middleware
callback. The projection first handles an existing event identity as a duplicate or conflict, then
requires the target and drone to be known and the drone to be assigned to that target before it
records a new outcome.

`TerminalView` renders recorded outcomes in stable event-identity order with the correlated drone,
target, position, and occurrence time. No generated DDS type enters console core or the UI adapter.

## Try it

Run the focused core, adapter, and rendering experiments:

```bash
cmake --build --preset development --target console_core_test console_outcome_dds_adapter_test console_ui_adapter_test
ctest --preset development -R '^(ConsoleCore|ConsoleOutcomeDdsAdapter|TerminalView)\.' --output-on-failure
```

The DDS test delivers valid, duplicate, and unrelated samples and verifies the rendered correlated
row. It also destroys one console reader and creates another while the interceptor writer remains
alive. The second reader receives the retained `TRANSIENT_LOCAL` event. This demonstrates
late-reader history, not persistence after writer restart.

## Takeaway

DDS supplies a sample and its configured history; console core decides whether that event belongs
to its operational view. Stable event identity prevents duplicate display, while explicit drone
and target correlation prevents unrelated outcomes from being mistaken for success.
