# 11 — IDL target track

## Concept

Interface Definition Language (IDL) describes data independently of the programming language that
will send or receive it. Fast DDS-Gen turns that contract into a C++ value type and a
`TopicDataType` that Fast DDS can serialize, deserialize, allocate, and identify by key. Fast DDS
wraps a `TopicDataType` in `TypeSupport` when an application later registers it with a participant.
No participant or endpoint is needed merely to generate and compile the contract.

The `@key` annotation makes `target_id` the DDS instance identity. Measurements for target 42 can
change position and time while remaining updates to the same keyed instance. The generated
serialization code uses Fast CDR to encode those fields for the wire; applications do not hand-code
that byte layout.

## In this project

`src/drone_dds_types/idl/target_track.idl` is the source of truth for
`drone::dds::TargetTrack`. CMake runs the pinned Fast DDS-Gen version into
`build/development/generated/drone_dds_types` and compiles the generated `TopicDataType` and type
object support into `drone_dds_types`. `CartesianPosition` is marked `@nested`: it supplies the
structured position field but is not itself a top-level DDS topic type. Generated files stay out of
source control and are never edited directly.

`tests/target_track_wire_type_test.cpp` proves the generated fields are available and that samples
with the same `target_id` compute the same instance key even when their measurements differ. The
participant-neutral model under `include/drone/domain` remains independent; explicit conversion
between these two representations belongs to roadmap step 15.

The Fast DDS 3.3 documentation describes the
[Fast DDS-Gen output and generation workflow](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/topic/fastddsgen/fastddsgen.html).

## Try it

Build and run the focused compile-and-key proof:

```bash
cmake --preset development
cmake --build --preset development --target target_track_wire_type_test
ctest --preset development -R '^TargetTrackWireType\.'
```

To observe the IDL dependency, update the file timestamp and rebuild verbosely:

```bash
touch src/drone_dds_types/idl/target_track.idl
cmake --build --preset development --target drone_dds_types --verbose
```

The build prints `Generating drone DDS wire types from IDL` before recompiling the generated
sources.

## Takeaway

IDL owns the stable wire shape; generated C++ makes that shape usable by Fast DDS; `TopicDataType`
connects it to serialization and key computation; and `TypeSupport` will register it at runtime.
None of those generated concerns need to enter the domain model.
