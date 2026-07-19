# 12 — IDL drone state

## Concept

An IDL enum becomes a generated C++ enum, and each IDL structure member becomes generated accessors
and serialized data. That gives every DDS participant the same lifecycle vocabulary and field layout
without importing an application's domain classes. An `@optional` member distinguishes an absent
assigned target from a target identifier value; Fast DDS-Gen represents it with
`eprosima::fastcdr::optional`.

Wire contracts require deliberate evolution because deployed participants may run different
generated versions at the same time. Enum numeric values must retain their meaning, existing fields
must not be silently reordered or retyped, and the type's extensibility rules determine which
changes can remain assignable. The lifecycle values therefore have explicit numeric values, and any
future IDL edit must regenerate and verify the contract rather than editing generated C++.

## In this project

`src/drone_dds_types/idl/target_track.idl` now defines `drone::dds::DroneState` beside the shared
`CartesianPosition`. The state carries the catalog's `drone_id` key, position, report time,
`DroneStatus`, and an optional assigned target. Its vocabulary mirrors the participant-neutral
domain model but remains a separate generated type; conversion between the two belongs to roadmap
step 16.

CMake regenerates the existing `drone_dds_types` target from IDL. The
`drone_state_wire_type_test` target links that generated library directly, without `drone_domain`,
a participant core, or an adapter. Its tests exercise every generated field and prove that changing
position or lifecycle does not change a sample's key, while changing `drone_id` does.

The Fast DDS 3.3 documentation shows the generated mappings for
[IDL enums, optional members, and extensibility](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastddsgen/dataTypes/dataTypes.html).

## Try it

Build and run the focused generated-type proof:

```bash
cmake --preset development
cmake --build --preset development --target drone_state_wire_type_test
ctest --preset development -R '^DroneStateWireType\.'
```

To see that IDL remains the source of truth, update its timestamp and rebuild verbosely:

```bash
touch src/drone_dds_types/idl/target_track.idl
cmake --build --preset development --target drone_dds_types --verbose
```

The build reports `Generating drone DDS wire types from IDL` and recompiles the generated support.
No generated file under `build/development/generated/drone_dds_types` should be edited.

## Takeaway

IDL fixes the serialized field and enum vocabulary shared by independently deployed participants.
Generated C++ is replaceable build output, while explicit enum values, keys, optionality, and
reviewed evolution preserve the meaning of data already in flight.
