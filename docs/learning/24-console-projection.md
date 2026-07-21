# 24 — Console projection

## Concept

A DDS reader receives samples in transport terms, while the console needs an application-shaped
view: the latest known state of every target. A projection builds that view by applying each sample
to keyed state. The target identifier selects the entry, and the measurement timestamp decides
whether an update is newer than the state already shown.

Repeated or reordered delivery must not move the view backward. An exact repeat of the current
sample is a duplicate and has no effect. A sample with an older timestamp is stale and is also
ignored. Different data with the same identifier and timestamp is rejected as conflicting because
one logical measurement version must not have two meanings.

## In this project

`TargetTrackInputPort` is the DDS-free boundary through which a future console subscriber will pass
domain `TargetTrack` values. `TargetProjection` implements that port and keeps one latest track per
`TargetId`. It reports whether an input was added, updated, duplicated, stale, or conflicting, so
the receive adapter can make those outcomes observable without owning the policy.

The projection exposes lookup by identifier and a snapshot ordered by target identifier. That
stable snapshot is suitable for the terminal UI added in a later step. The `console_core` target
links only `drone_domain`; it has no Fast DDS, generated-type, UI, or simulation dependency.

## Try it

Run the focused projection tests from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target console_core_test
ctest --preset development -R '^ConsoleCore\.' -V
```

The cases apply a new target, a newer measurement, an exact duplicate, an older measurement, and a
same-time conflict. Each test checks both the reported outcome and the state retained for the
target. To inspect the library boundary, run:

```bash
ninja -C build/development -t query console_core
```

## Takeaway

DDS distributes samples, but the console core decides how those samples become operator-facing
state. Keeping version and conflict policy in this deterministic projection makes later subscriber
and UI adapters small and replaceable.
