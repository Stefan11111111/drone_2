# 13 — IDL control messages

## Concept

A received DDS command sample says that middleware data reached a reader; it does not prove that the
addressed application accepted the intent, changed state, or completed the requested action.
Reliable delivery can detect and retransmit missing transport samples while matching endpoints are
alive, but application validation, duplicate handling, and outcome reporting remain explicit parts
of the distributed workflow.

Assignment and interception are therefore separate wire contracts. An assignment describes the
latest target selected for one drone. An interception command is a distinct occurrence with its own
identity, allowing application code to recognize repeated or stale intent later. Both contracts use
fixed-width identifiers, so the approved model contains no variable-length string or sequence that
needs an IDL bound.

## In this project

`src/drone_dds_types/idl/target_track.idl` defines `drone::dds::Assignment` with the catalog's
`drone_id` key and `drone::dds::InterceptionCommand` with the catalog's `command_id` key. The
assignment carries only a drone-to-target selection; the command additionally carries the command
identity that distinguishes one operator action from another.

CMake regenerates both C++ types and their `TopicDataType` support into the existing
`drone_dds_types` library. `tests/control_wire_types_test.cpp` compiles against the generated API,
checks every field, and proves the two key definitions. It deliberately does not add a participant,
DataWriter, or DataReader. Domain-to-wire validation and conversion belong to roadmap step 17.

The Fast DDS 3.3 documentation explains the transport-side scope of
[reliable reliability](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/core/policy/standardQosPolicies.html#reliabilityqospolicy).

## Try it

Build and run the focused generated-type proof:

```bash
cmake --preset development
cmake --build --preset development --target control_wire_types_test
ctest --preset development -R '^ControlWireTypes\.'
```

Compare samples with the same assignment `drone_id` or command `command_id` in the key tests. Their
payload may differ while Fast DDS still computes the same keyed instance. Changing the key produces
a different instance.

## Takeaway

DDS can transport control data reliably, but it does not turn delivery into application acceptance
or successful execution. Separate contracts and deliberate identities preserve the difference
between selecting a target and commanding the assigned drone to begin interception.
