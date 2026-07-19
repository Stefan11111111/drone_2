# 08 — Control messages

## Concept

State data reports what is true now; a control message expresses what an operator wants to happen.
An assignment asks one drone to take responsibility for one target. A later interception command
asks that assigned drone to start acting on that responsibility. Delivering either value does not
mean that the receiver accepted it or performed the action.

DDS Reliability may retransmit lost wire data, but delivery still does not prove that the requested
action completed. A sender may write the same intent again after an uncertain outcome, so a command
identity lets the future interceptor recognize the same start request. Naming the target also lets
it reject a delayed command that no longer agrees with the drone's current assignment.

## In this project

`include/drone/domain/assignment.h` defines `Assignment` from a `DroneId` and `TargetId`.
`include/drone/domain/interception_command.h` defines the distinct `InterceptionCommand` from a
nonzero `InterceptionCommandId`, the addressed drone, and the target whose assignment should start.
The assignment pair is enough to distinguish a duplicate from a conflicting assignment; the start
command needs its own identity because two intentional start requests can otherwise have identical
drone and target fields.

The values belong to `drone_domain`, which still links only the C++ standard library. This step adds
no IDL, generated type, Topic, QoS, DataWriter, or DataReader. Those later transport pieces will
carry these values without owning their operational meaning.

## Try it

Build and run only the control-message tests from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target control_messages_test
ctest --preset development -R '^ControlMessages\.'
```

The tests change each field independently to show value semantics, prove that target and drone IDs
remain different types, and reject every zero identifier. Compare these messages with the snapshot
tests by also running:

```bash
ctest --preset development -R '^(TargetTrack|DroneState|ControlMessages)\.'
```

## Takeaway

DDS distributes both state and intent as typed data, but the application gives them different
semantics. State describes the latest known world; an assignment and a start command request two
separate transitions, and explicit identity and correlation fields prepare receivers to handle
duplicates and delayed intent safely.
