# 17 — Control and event mapping

## Concept

A received DDS sample has passed wire-format deserialization, but that does not make its values valid
for the application. Remote publishers may be outdated, buggy, or built from a different schema
assumption. The transport boundary must therefore treat generated objects as untrusted input and
report validation failures instead of silently replacing bad fields with plausible values.

## In this project

`drone_dds_transport` now provides explicit, bidirectional mappings for `Assignment`,
`InterceptionCommand`, and `ExplosionEvent`. Each domain concept has its own mapping header and error
enum. Zero identifiers are reported according to their semantic role; explosion events additionally
reject non-finite coordinates and timestamps before the Unix epoch.

The domain-to-wire direction unwraps strongly typed identifiers and application values into the IDL
fields. The wire-to-domain direction returns `std::expected`, so a future DataReader adapter must
handle malformed input before calling core code. DDS delivery and generated deserialization do not
validate these domain invariants, deduplicate intent, or prove that a publisher is trustworthy.

## Try it

Build and run the focused mappings:

```bash
cmake --preset development
cmake --build --preset development --target control_event_mapping_test
ctest --preset development -R '^ControlEventMapping\.'
```

The tests round-trip all three values, then independently set each identifier to zero and corrupt
the explosion position and time. Each malformed sample returns its named error without constructing
a domain value.

## Takeaway

DDS moves typed samples between participants; the transport mapping decides whether those samples
are meaningful enough to enter the application. Visible conversion errors keep distributed faults
observable and prevent invalid intent or events from becoming trusted domain data.
