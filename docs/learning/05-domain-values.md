# 05 — Domain values

## Concept

The application needs names for operational facts before it needs DDS representations of them.
Strong identifiers prevent a target ID from being passed where a drone ID is expected. A validated
position and timestamp prevent invalid state from entering later tracking and transport code.

Keeping these values independent of generated wire types matters because IDL-generated C++ serves
serialization and DDS. Application values instead express the rules the observer, console, and
interceptor share. A mapping boundary added in a later step will translate between the two.

## In this project

`include/drone/domain/` contains distinct `TargetId` and `DroneId` values, a three-dimensional local
Cartesian `Position` measured in metres, and a millisecond `Timestamp` measured from the Unix epoch.
Identifiers must be nonzero, coordinates must be finite, and timestamps cannot precede the epoch.
The `drone_domain` CMake target links no other project or middleware library; only its test target
links GoogleTest.

These representation choices are sufficient for the simulated scenario and remain on the
application side of the future DDS adapter boundary. Step 05 intentionally adds no generated type,
serialization, Topic, or DDS entity.

## Try it

Build and run only the value tests from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target domain_values_test
ctest --preset development -R '^DomainValues\.'
```

The tests demonstrate identifier type separation, value equality and ordering, and rejection of
zero IDs, non-finite coordinates, and pre-epoch timestamps. To inspect the dependency boundary, run:

```bash
cmake --build --preset development --target drone_domain
ninja -C build/development -t query drone_domain
```

## Takeaway

The application's shared vocabulary can enforce useful invariants without knowing anything about
DDS. That separation will let generated serialization code change without leaking middleware types
into the cores.
