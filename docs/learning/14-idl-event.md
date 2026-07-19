# 14 — IDL explosion event

## Concept

An event identity lets consumers correlate and deduplicate one occurrence independently of its
payload. DDS uses the annotated key to divide a Topic into instances, but a matching key does not
make conflicting payloads valid or automatically stop an application from applying an occurrence
twice. The receiver must validate the facts and remember processed event identities where duplicate
effects matter.

History and durability determine what samples middleware can retain for readers. The catalog gives
explosion events `TRANSIENT_LOCAL` durability and `KEEP_LAST(1)` history per `event_id`, so a late
reader may receive the one retained sample for each event instance while its writer is still alive
and within the configured resource limits. This is not durable storage across writer restarts, and
the retained sample still goes through normal application duplicate handling.

## In this project

`src/drone_dds_types/idl/target_track.idl` defines `drone::dds::ExplosionEvent`. Its catalog-approved
`event_id` key identifies the occurrence; `drone_id`, `target_id`, `position`, and `occurred_at_ms`
carry the operational facts needed to correlate the outcome with the interceptor and target.

CMake regenerates the type and its `TopicDataType` support into `drone_dds_types`.
`tests/explosion_event_wire_type_test.cpp` compiles against every generated field and verifies that
only `event_id` affects the computed DDS instance key. No simulated effect, publisher, reader, or
domain mapping is added in this step. Those behaviors remain behind the later adapter and transport
boundaries.

Fast DDS documents how keys create
[topic instances](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/topic/instances.html)
and how
[durability and history QoS](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/core/policy/standardQosPolicies.html)
control retained data.

## Try it

Build and run the focused generated-type proof:

```bash
cmake --preset development
cmake --build --preset development --target explosion_event_wire_type_test
ctest --preset development -R '^ExplosionEventWireType\.'
```

In the key test, `firstOutcome` and `sameIdentityDifferentPayload` compute the same instance handle.
That makes a useful malformed/conflict case for the explicit mapping validation introduced in
roadmap step 17.

## Takeaway

The event key identifies an occurrence; the remaining fields correlate its outcome. DDS history can
make that occurrence visible to a late reader, but application code still owns validation,
idempotence, and any persistence beyond the writer's lifetime.
