# 15 — Target-track mapping

## Concept

DDS-generated classes describe the serialized wire contract, while domain values describe facts the
application is allowed to use. An explicit mapping at the transport boundary keeps generated APIs,
schema naming, and serialization concerns from spreading into core logic. It is also a trust
boundary: received bytes can produce generated values that violate domain invariants, so conversion
must validate them before the application accepts them.

## In this project

`drone_dds_transport` depends directly on both `drone_domain` and `drone_dds_types`, preserving the
one-way dependency graph in `architecture.md`. `target_track_mapping.cpp` converts every target ID,
position coordinate, and millisecond timestamp in both directions. A valid domain track maps to
`drone::dds::TargetTrack` without loss.

The wire-to-domain direction returns `std::expected`. It rejects a zero target ID, any non-finite
coordinate, and a timestamp before the Unix epoch with a specific `TargetTrackMappingError`; it does
not invent replacement values or let malformed samples enter core code. No participant, Topic,
DataWriter, or DataReader exists yet—the generated value is only crossing an in-process software
boundary. Fast DDS describes generated data-type APIs in its
[Fast DDS-Gen documentation](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/topic/fastddsgen/fastddsgen.html).

## Try it

Build and run the focused round-trip and malformed-input checks:

```bash
cmake --preset development
cmake --build --preset development --target target_track_mapping_test
ctest --preset development -R '^TargetTrackMapping\.'
```

To observe the boundary, temporarily change `makeWireTrack()` in
`tests/target_track_mapping_test.cpp` to use target ID `0`. The valid-path test then reports the
mapping error instead of constructing a domain `TargetTrack`.

## Takeaway

Generated wire objects stop at the transport boundary. Explicit conversion preserves valid data and
makes malformed distributed input visible before it can affect application behavior.
