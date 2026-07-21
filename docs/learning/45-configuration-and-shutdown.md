# Step 45: Explicit configuration and shutdown

## Concept

A DDS domain ID isolates discovery and data exchange: endpoints in different domains do not match.
A participant name makes a process and its endpoints recognizable in discovery diagnostics, but it
is not a network address or a uniqueness guarantee. DDS entities form an ownership hierarchy, so
shutdown must stop application work and then destroy readers, writers, topics, publishers, and
subscribers before deleting each participant.

## In this project

`application_support` gives `observer`, `console`, and `interceptor` the same validated common
options:

- `--domain-id ID` selects domain 0 through 232;
- `--participant-name NAME` supplies the process identity used as the base for its DDS participant
  names; and
- `--tick-ms MILLISECONDS` sets the observer simulation tick, console polling tick, or interceptor
  control/simulation tick.

The observer additionally accepts `--tick-count COUNT` for a finite run, and the console retains
the test-oriented `--auto-pursuit` action mode. Invalid or duplicate options fail before DDS startup
with the relevant option name and complete usage text.

`ShutdownMonitor` converts SIGINT and SIGTERM into a cooperative stop request. Each composition
root leaves its loop, then stack unwinding destroys its adapter objects in reverse construction
order. `DomainParticipantOwner` remains the final safety net: its destructor asks Fast DDS to delete
any contained entities left by partial startup before it deletes the participant.

## Try it

Run an observer with explicit identity and timing, then request graceful shutdown:

```bash
./build/development/observer --domain-id 45 --participant-name learning_observer --tick-ms 500 &
observer_pid=$!
kill -TERM "$observer_pid"
wait "$observer_pid"
```

The last line is `observer: shutdown complete`, and `wait` returns exit code 0. To see validation
without creating DDS entities, run:

```bash
./build/development/interceptor --domain-id 233
```

The bounded automated experiments cover invalid domain/name/tick values, signal all three roles
while they are waiting on a 60-second tick, require a normal exit, and verify partial-startup DDS
cleanup:

```bash
ctest --preset development -R '^(ProcessConfiguration|ProcessLifecycle|DomainParticipantOwner)\.' --output-on-failure
```

## Takeaway

Operable DDS processes need explicit domain, identity, and rate configuration, while graceful
shutdown is application behavior: signals stop the loops and deterministic C++ ownership performs
the middleware cleanup.
