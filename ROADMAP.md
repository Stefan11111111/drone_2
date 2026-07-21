# Roadmap

## Goal

This roadmap turns [VISION.md](VISION.md) into a sequence of small, reviewable learning tasks.
The tasks build one simulated drone-interception system while introducing Fast DDS a concept at a
time. Complete them in order: later tasks may rely on contracts, tests, and terminology introduced
by earlier tasks.

The roadmap ends at the initial success criteria in `VISION.md`. Discovery Server, security,
record/replay, automated target assignment, and physical hardware adapters are useful follow-up
subjects, but they are intentionally outside this first learning path.

## How to use a step

Each numbered block below is an AI prompt. Give the AI one block at a time together with the
repository. The repository guidance in `AGENTS.md` remains part of every prompt.

For every step, the AI shall:

1. implement only that step and preserve the dependency rules in `architecture.md`;
2. add focused automated verification for the behavior introduced by the step;
3. add the named note under `docs/learning/` using the format below;
4. run the relevant repository verification commands and record useful hands-on commands in the
   note; and
5. change that step's checkbox to `[x]` only after its acceptance criteria pass.

Keep each learning note brief and grounded in the code from the step:

- **Concept:** explain the Fast DDS or distributed-systems idea in plain language.
- **In this project:** point to the files and runtime roles that demonstrate it.
- **Try it:** give a command, test, log message, or small experiment that makes the behavior
  observable.
- **Takeaway:** state what the learner should now understand.

Pure domain or adapter steps must still explain why their boundary matters to a DDS application.
Link to the matching version of the official Fast DDS documentation when it adds value. Do not turn
the note into generic documentation or paste large source listings.

## Phase 1: Reproducible foundation

### 01 — Record the toolchain and dependency decisions

- [x] **AI prompt:** Complete roadmap step 01 only. Inspect the available development environment
  and create a small decision record for the build system, test framework, supported compiler,
  formatter, static analysis, Fast DDS acquisition/version, compatible Fast DDS-Gen version, and
  generated-source policy. Resolve only tooling decisions; do not add production code. If a
  hard-to-reverse choice is not already resolved by the repository, ask before committing to it.
  Add `docs/learning/01-fast-dds-toolchain.md`, explaining the distinct roles of Fast DDS, Fast CDR,
  Fast DDS-Gen, and the application. Acceptance: the decision record contains the exact versions or
  version rules and the commands needed to verify that the required tools are available.

### 02 — Bootstrap the C++26 build

- [x] **AI prompt:** Complete roadmap step 02 only. Add the smallest build skeleton that configures
  the repository as strict C++26 with compiler extensions disabled, plus one trivial executable or
  test that proves the selected compiler accepts the configuration. Do not add Fast DDS or domain
  behavior yet. Add `docs/learning/02-build-skeleton.md`, explaining what will be compiled by this
  repository versus supplied by the middleware. Acceptance: a clean configure, build, and test pass.

### 03 — Establish the verification workflow

- [x] **AI prompt:** Complete roadmap step 03 only. Add the selected unit-test integration and
  repository helpers or configuration for formatting and static analysis, then replace the
  verification placeholder in `AGENTS.md` with the canonical commands. Keep the sample test focused
  on proving the workflow. Add `docs/learning/03-verification-loop.md`, explaining how rapid tests
  make later DDS experiments trustworthy. Acceptance: format/check, configure, analysis, build, and
  test commands are documented and pass in their prescribed order.

### 04 — Prove that Fast DDS links

- [x] **AI prompt:** Complete roadmap step 04 only. Integrate the approved Fast DDS dependency into
  the build and add a minimal smoke target that creates and cleanly deletes one named
  `DomainParticipant`; do not create a Topic, Publisher, or Subscriber. Add
  `docs/learning/04-domain-participant.md`, explaining DDS domains, participant identity, and why a
  participant is the entry point into the DDS data space. Acceptance: the smoke test links against
  Fast DDS, runs without leaking a participant, and fails clearly when the dependency is missing.

## Phase 2: Domain values and wire contracts

### 05 — Add identifiers, position, and time values

- [x] **AI prompt:** Complete roadmap step 05 only. Create the `drone_domain` library with strongly
  typed target and drone identifiers, a position value, and the minimal time representation needed
  by the vision. Add focused value-semantic tests; add no DDS includes. Add
  `docs/learning/05-domain-values.md`, explaining why application values are kept independent of
  generated wire types. Acceptance: the library depends only on the C++ standard library and its
  public invariants are tested.

### 06 — Model a target track

- [x] **AI prompt:** Complete roadmap step 06 only. Add the participant-neutral `TargetTrack` model
  needed to identify a target and carry its latest measured position and time. Test construction,
  equality, and any validation that protects a real invariant; do not add serialization. Add
  `docs/learning/06-target-track.md`, explaining how a data-centric system distributes state rather
  than invoking another process. Acceptance: observer, console, and interceptor needs from
  `VISION.md` can be represented without DDS or simulation terminology.

### 07 — Model drone state

- [x] **AI prompt:** Complete roadmap step 07 only. Add the participant-neutral `DroneState` and the
  smallest lifecycle/status vocabulary required for availability, assignment, movement, and
  outcome reporting. Test its invariants and transitions only where they belong to the value model.
  Add `docs/learning/07-drone-state.md`, explaining why published state is a contract rather than a
  view of an in-process object. Acceptance: the console can eventually render all initially required
  drone status from this value alone.

### 08 — Model assignment and interception commands

- [x] **AI prompt:** Complete roadmap step 08 only. Add separate participant-neutral values for
  assigning one drone to one target and commanding that assigned drone to start interception.
  Include identifiers needed to detect duplicates or stale intent if the approved contract calls
  for them. Add `docs/learning/08-control-messages.md`, explaining the difference between state data
  and operator intent in DDS. Acceptance: the two distinct operator actions in `VISION.md` are
  representable and invalid values are covered by tests.

### 09 — Model the explosion event

- [x] **AI prompt:** Complete roadmap step 09 only. Add the participant-neutral event emitted when
  an interceptor reaches its assigned target, carrying only the operational facts consumers need.
  Test its value semantics and invariants. Add `docs/learning/09-explosion-event.md`, explaining the
  difference between an event and continuously updated state. Acceptance: the console can correlate
  the future event with the drone and target without simulation-specific types.

### 10 — Define the DDS topic catalog and QoS hypotheses

- [x] **AI prompt:** Complete roadmap step 10 only. Create a concise DDS topic catalog listing each
  initial topic's purpose, wire type, key, producer, consumers, update pattern, and proposed
  Reliability, Durability, History, and resource-limit behavior. Explain every non-default QoS
  choice from scenario semantics and call out choices that need experimental confirmation; add no
  endpoints yet. Add `docs/learning/10-topics-keys-qos.md`, explaining Topics, keyed instances, QoS
  compatibility, and why topic names and types must match. Acceptance: every cross-process flow in
  `VISION.md` appears exactly once in the catalog and simulation/hardware details do not.

### 11 — Generate the target-track wire type

- [x] **AI prompt:** Complete roadmap step 11 only. Create the `drone_dds_types` target and the
  smallest IDL contract for `TargetTrack`, including the approved key, and integrate reproducible
  Fast DDS-Gen generation according to step 01's policy. Do not add a DataWriter or DataReader. Add
  `docs/learning/11-idl-target-track.md`, explaining IDL, generated C++ types, `TopicDataType`,
  `TypeSupport`, serialization, and keys. Acceptance: changing the IDL causes regeneration and the
  generated type compiles without manual edits.

### 12 — Generate the drone-state wire type

- [x] **AI prompt:** Complete roadmap step 12 only. Extend the IDL with the `DroneState` wire
  contract and approved key while preserving the participant-neutral vocabulary. Add generation and
  compile coverage but no DDS endpoint. Add `docs/learning/12-idl-drone-state.md`, explaining how IDL
  enums and fields become serialized C++ data and why wire-contract evolution must be deliberate.
  Acceptance: the generated state type compiles and remains independent of core and adapter types.

### 13 — Generate the control wire types

- [x] **AI prompt:** Complete roadmap step 13 only. Extend the IDL with the assignment and
  interception-command contracts, using the topic catalog's keys and bounded fields where the
  approved model requires them. Add generation and compile coverage only. Add
  `docs/learning/13-idl-control.md`, explaining what DDS does and does not guarantee when a command
  sample is delivered. Acceptance: both generated types compile and preserve the semantic
  distinction introduced in step 08.

### 14 — Generate the explosion-event wire type

- [x] **AI prompt:** Complete roadmap step 14 only. Extend the IDL with the explosion-event contract
  and its approved key, then add generation and compile coverage. Do not implement the simulated
  effect or a publisher. Add `docs/learning/14-idl-event.md`, explaining how event identity and DDS
  history affect duplicate handling and late joiners. Acceptance: the generated type compiles and
  can correlate the outcome with the domain identifiers from step 09.

### 15 — Map target tracks across the DDS boundary

- [x] **AI prompt:** Complete roadmap step 15 only. Start `drone_dds_transport` with explicit,
  bidirectional conversion between domain and generated target-track values. Test round trips and
  rejection of malformed wire data; add no participant or endpoint. Add
  `docs/learning/15-target-mapping.md`, explaining why serialization types stop at the transport
  boundary. Acceptance: valid domain data survives a domain-to-wire-to-domain round trip without
  loss.

### 16 — Map drone state across the DDS boundary

- [x] **AI prompt:** Complete roadmap step 16 only. Add explicit, bidirectional conversion for drone
  state, including exhaustive status mapping and malformed-input tests. Add no DDS entities. Add
  `docs/learning/16-drone-state-mapping.md`, explaining how a mapping layer protects core code from
  generated API and schema changes. Acceptance: every domain status maps deliberately and round-trip
  tests pass.

### 17 — Map commands and events across the DDS boundary

- [x] **AI prompt:** Complete roadmap step 17 only. Add and test explicit conversions for assignment,
  interception command, and explosion event. Keep mapping errors visible to callers rather than
  silently manufacturing valid values. Add `docs/learning/17-control-event-mapping.md`, explaining
  validation at a distributed trust boundary. Acceptance: round-trip and malformed-wire tests cover
  every remaining initial wire type.

## Phase 3: A minimal DDS data path

### 18 — Own participant lifetime and configuration

- [x] **AI prompt:** Complete roadmap step 18 only. Add the smallest RAII owner in
  `drone_dds_transport` for a named `DomainParticipant`, configured by an explicit domain ID and with
  deterministic reverse-order cleanup. Keep it independent of participant-specific cores. Add
  `docs/learning/18-participant-lifetime.md`, explaining the DDS entity hierarchy and why child
  entities must be deleted before their participant. Acceptance: success and construction-failure
  tests leave no live DDS entities.

### 19 — Round-trip one target track in process

- [x] **AI prompt:** Complete roadmap step 19 only. Using two participants, create the minimum
  Publisher, Subscriber, Topic, DataWriter, and DataReader support needed to send one target track
  through Fast DDS and receive it through the domain mapping. Apply only the target QoS from the
  catalog and avoid a speculative generic transport framework. Add
  `docs/learning/19-first-publish-subscribe.md`, explaining the DCPS entity chain and the difference
  between writing a sample and taking it. Acceptance: a deterministic integration test waits for a
  match, round-trips one sample, and times out with a useful failure instead of sleeping blindly.

### 20 — Make discovery and matching observable

- [x] **AI prompt:** Complete roadmap step 20 only. Expose narrowly scoped match/discovery status
  from the target endpoint support and test both matching and an intentionally incompatible setup
  without depending on timing guesses. Add `docs/learning/20-discovery-and-matching.md`, explaining
  participant discovery, endpoint discovery, and the topic/type/QoS conditions for a match.
  Acceptance: the learner can run one passing match experiment and one controlled non-match
  experiment and understand their logs.

## Phase 4: Observer-to-console target slice

### 21 — Implement observer core tracking ports

- [x] **AI prompt:** Complete roadmap step 21 only. Create `observer_core` with an input port for
  detections and an output port for target-track updates, plus the smallest use case that converts a
  detection into current track state. Use test doubles and add no DDS or simulation dependencies.
  Add `docs/learning/21-observer-ports.md`, explaining how ports let the same core feed DDS from a
  simulated or physical radar. Acceptance: unit tests prove the use case emits the expected track
  update through its output port.

### 22 — Produce a deterministic moving target

- [x] **AI prompt:** Complete roadmap step 22 only. Implement `simulated_radar_adapter` against the
  observer input port with a deterministic, time-stepped moving target. Keep scheduling and scenario
  parameters explicit so tests do not depend on wall-clock timing. Add
  `docs/learning/22-simulated-radar.md`, explaining why the simulator is an adapter and the target
  contract says nothing about radar hardware. Acceptance: focused tests prove repeatable positions
  over several ticks.

### 23 — Publish observer tracks through DDS

- [x] **AI prompt:** Complete roadmap step 23 only. Implement `observer_dds_adapter` as the
  observer-core output port using the target-track DataWriter support, then add a minimal observer
  composition executable that wires it to the simulated radar. Keep the executable limited to
  configuration, startup, and wiring. Add `docs/learning/23-observer-publisher.md`, explaining how an
  application port call becomes a keyed DDS sample. Acceptance: an integration test or probe can
  observe multiple track updates from the observer process.

### 24 — Project target state in console core

- [x] **AI prompt:** Complete roadmap step 24 only. Create `console_core` support for accepting
  target updates and maintaining the latest target state by identifier. Define and test stale-update
  and duplicate behavior without adding DDS or UI code. Add `docs/learning/24-console-projection.md`,
  explaining how a subscriber builds an application-specific view from received samples.
  Acceptance: deterministic unit tests cover new, newer, duplicate, and stale updates.

### 25 — Receive target tracks through DDS

- [x] **AI prompt:** Complete roadmap step 25 only. Implement the target-track receive side of
  `console_dds_adapter`, mapping valid taken samples into the console-core input port and handling
  invalid samples and shutdown safely. Keep core work out of the Fast DDS callback if it could block
  middleware threads. Add `docs/learning/25-console-subscriber.md`, explaining listeners or
  conditions, sample information, valid data, and the chosen callback handoff. Acceptance: an
  integration test delivers a track through DDS into the console projection.

### 26 — Display targets in a terminal console

- [x] **AI prompt:** Complete roadmap step 26 only. Implement the first `console_ui_adapter` as a
  replaceable terminal view and add the console composition executable. Render stable, readable
  snapshots of live target state without introducing operator commands yet. Add
  `docs/learning/26-terminal-view.md`, explaining why presentation remains outside DDS callbacks and
  console core. Acceptance: a UI-adapter test covers its rendered output and the executable contains
  wiring rather than business logic.

### 27 — Verify the first separate-process flow

- [x] **AI prompt:** Complete roadmap step 27 only. Add a bounded automated integration scenario
  that starts observer and console as separate processes, waits for DDS discovery, and proves that
  at least two changing target positions reach the console. Capture actionable logs on failure and
  clean up child processes. Add `docs/learning/27-cross-process-targets.md`, explaining what changed
  when the same DDS data path crossed a process boundary. Acceptance: the scenario passes without
  fixed startup sleeps and demonstrates the first two roles from `VISION.md`.

## Phase 5: Interceptor availability and state

### 28 — Add the interceptor core state machine shell

- [x] **AI prompt:** Complete roadmap step 28 only. Create `interceptor_core` with explicit state,
  positioning/flight-control ports, and a drone-state output port, but implement only startup in an
  available/idle state. Use test doubles and add no DDS or simulation code. Add
  `docs/learning/28-interceptor-state-machine.md`, explaining why a deterministic core state machine
  is safer than encoding behavior in DDS callbacks. Acceptance: unit tests prove initial state and
  initial state reporting.

### 29 — Simulate vehicle position and motion

- [x] **AI prompt:** Complete roadmap step 29 only. Implement `simulated_vehicle_adapter` for
  deterministic position and bounded time-step movement through interceptor-core ports. Do not add
  target-following decisions or explosion behavior yet. Add
  `docs/learning/29-simulated-vehicle.md`, explaining how the same core could later drive physical
  positioning and flight-control adapters. Acceptance: tests cover no movement, one movement step,
  speed bounds, and arrival at a requested point.

### 30 — Publish interceptor state

- [x] **AI prompt:** Complete roadmap step 30 only. Add the drone-state DataWriter path to
  `interceptor_dds_adapter` and a minimal interceptor composition executable wired to the simulated
  vehicle. Publish its available state using the catalog's key and QoS. Add
  `docs/learning/30-interceptor-state-publisher.md`, explaining keyed state updates and DDS instance
  identity. Acceptance: a probe or integration test receives the interceptor's available state and
  the executable remains a composition root.

### 31 — Show live drone state on the console

- [x] **AI prompt:** Complete roadmap step 31 only. Extend console core, its DDS adapter, and the
  terminal UI just enough to receive, project, and display latest drone state by identifier. Keep
  the change limited to the state path and test projection, DDS delivery, and rendering at their
  respective boundaries. Add `docs/learning/31-console-drone-state.md`, explaining how one
  participant can subscribe to multiple independent Topics. Acceptance: observer, console, and
  interceptor can run separately and the console shows both target and available drone state.

### 32 — Verify late discovery of an available interceptor

- [x] **AI prompt:** Complete roadmap step 32 only. Extend the process-level integration scenario to
  start the interceptor and console in both orders and verify the documented drone-state QoS
  behavior. Change QoS only if the experiment disproves step 10's hypothesis, and update the catalog
  with evidence. Add `docs/learning/32-late-joiner-state.md`, explaining durability, history, and
  late joiners using the observed result. Acceptance: both start orders have deterministic expected
  outcomes and no startup sleeps.

## Phase 6: Operator control and pursuit

### 33 — Validate assignment in console core

- [x] **AI prompt:** Complete roadmap step 33 only. Add a console-core output port and validation use
  case for assigning an available drone to a known target. Cover unknown, unavailable, duplicate,
  and valid selections with unit tests; add no DDS or terminal-input parsing. Add
  `docs/learning/33-assignment-use-case.md`, explaining why command validation precedes distributed
  publication. Acceptance: only a valid selection emits one assignment value through the port.

### 34 — Send assignments through DDS

- [x] **AI prompt:** Complete roadmap step 34 only. Add the assignment DataWriter to the console DDS
  adapter and DataReader to the interceptor DDS adapter with the catalog's QoS and safe callback
  handoff. Test the adapter path with a captured interceptor input; do not change interceptor state
  yet. Add `docs/learning/34-assignment-topic.md`, explaining reliable delivery, volatility or
  durability, and the limits of middleware delivery guarantees. Acceptance: one valid console
  assignment crosses DDS and reaches the interceptor input exactly as designed.

### 35 — Apply assignment in interceptor core

- [x] **AI prompt:** Complete roadmap step 35 only. Teach interceptor core to validate an assignment
  addressed to itself, remember the target identifier, transition to assigned state, and publish the
  new drone state. Cover wrong-drone, duplicate, conflicting, and valid assignments with unit tests.
  Add `docs/learning/35-interceptor-assignment.md`, explaining idempotence and why distributed
  messages may need duplicate handling. Acceptance: a DDS-level integration test observes the
  assigned state after a valid assignment.

### 36 — Accept a start command from the operator

- [x] **AI prompt:** Complete roadmap step 36 only. Add console-core validation and a terminal UI
  action for commanding an assigned drone to begin interception. Keep parsing replaceable and do
  not add its DDS endpoint yet. Add `docs/learning/36-operator-command.md`, explaining the boundary
  between human interaction, application intent, and DDS transport. Acceptance: tests show that
  only a valid command reaches the console-core output port.

### 37 — Send interception commands through DDS

- [x] **AI prompt:** Complete roadmap step 37 only. Add the interception-command DataWriter and
  DataReader adapter path with the catalog's QoS, mapping, duplicate policy, and safe shutdown.
  Deliver commands to a captured interceptor input without starting motion yet. Add
  `docs/learning/37-command-topic.md`, explaining how command identity and QoS combine to handle
  retransmission without repeating intent. Acceptance: an integration test sends one command from
  console core through DDS to the interceptor boundary.

### 38 — Subscribe the interceptor to target updates

- [x] **AI prompt:** Complete roadmap step 38 only. Add the target-track DataReader to the interceptor
  DDS adapter and an interceptor-core input that stores only the newest update for its assigned
  target. Ignore unrelated and stale tracks with tests; do not move the vehicle yet. Add
  `docs/learning/38-interceptor-target-reader.md`, explaining content selection in application code
  and why the interceptor follows published state rather than calling the observer. Acceptance: a
  DDS integration test proves that relevant newer tracks reach core and irrelevant tracks do not
  alter it.

### 39 — Follow the latest target position

- [x] **AI prompt:** Complete roadmap step 39 only. Extend the interceptor state machine so a valid
  start command transitions to intercepting and each deterministic tick requests bounded movement
  toward the latest assigned target position, then reports updated drone state. Use test doubles;
  change no DDS code. Add `docs/learning/39-pursuit-loop.md`, explaining how asynchronous DDS updates
  feed a deterministic control loop. Acceptance: unit tests prove start preconditions, movement,
  mid-pursuit retargeting, and state reporting.

### 40 — Verify pursuit of a moving target

- [x] **AI prompt:** Complete roadmap step 40 only. Wire the interceptor process loop to the
  simulated vehicle and add a bounded separate-process scenario that assigns, starts, and observes
  the drone change course after newer target samples. Add only the orchestration needed for
  deterministic automation of console actions. Add `docs/learning/40-moving-target-pursuit.md`,
  explaining update rates, asynchronous arrival, and why the latest keyed sample drives pursuit.
  Acceptance: logs and assertions prove the drone responds to at least one changed target position.

## Phase 7: Interception outcome

### 41 — Detect arrival and request the simulated effect

- [x] **AI prompt:** Complete roadmap step 41 only. Add an interception-effect output port and core
  arrival logic that triggers exactly once when the drone reaches the current assigned target
  according to an explicit tolerance. Extend `simulated_vehicle_adapter` or add its cohesive effect
  implementation without putting simulation details in core. Add
  `docs/learning/41-arrival-and-effect-port.md`, explaining why the explosion is requested through a
  replaceable port. Acceptance: deterministic tests cover not-yet-arrived, arrival, repeated ticks,
  and effect failure behavior.

### 42 — Publish the explosion event

- [x] **AI prompt:** Complete roadmap step 42 only. Add the explosion-event DataWriter to the
  interceptor DDS adapter and connect the successful simulated effect outcome to it through the
  interceptor core's output boundary. Apply and test the catalog's event QoS. Add
  `docs/learning/42-explosion-publisher.md`, explaining the path from a local physical-effect result
  to a distributed operational event. Acceptance: exactly one correlated explosion event is
  observable through DDS for one successful interception.

### 43 — Display the interception outcome

- [x] **AI prompt:** Complete roadmap step 43 only. Add the explosion-event DataReader to the console
  DDS adapter, correlate outcomes in console core, and render the result in the terminal UI. Cover
  duplicate, unrelated, and valid events at the appropriate boundaries. Add
  `docs/learning/43-console-outcome.md`, explaining event consumption, correlation, and the effect of
  history on a restarting console. Acceptance: an integration test delivers a DDS event and verifies
  the rendered correlated outcome.

### 44 — Prove the complete vision scenario

- [ ] **AI prompt:** Complete roadmap step 44 only. Add one bounded end-to-end test that launches
  observer, console, and interceptor as separate processes; observes target and drone state;
  performs assignment and start actions; verifies pursuit; and ends when the correlated explosion
  appears on the console. Reuse existing test support rather than duplicating orchestration. Add
  `docs/learning/44-end-to-end-dds.md`, tracing one scenario datum through IDL, discovery, writer,
  reader, adapter, core, and UI. Acceptance: the automated scenario proves every initial success
  criterion except the separate-machine run documented later.

## Phase 8: Operability and final audit

### 45 — Make process lifecycle and configuration explicit

- [ ] **AI prompt:** Complete roadmap step 45 only. Give all three executables consistent validated
  configuration for domain ID, participant identity, simulation tick/rate inputs, and graceful
  shutdown. Ensure DDS entities are destroyed safely even after partial startup failure. Add
  `docs/learning/45-configuration-and-shutdown.md`, explaining DDS domain isolation, participant
  names, and entity cleanup. Acceptance: configuration-error and signal-driven shutdown tests are
  bounded, leave no child processes, and report actionable errors.

### 46 — Turn QoS hypotheses into executable evidence

- [ ] **AI prompt:** Complete roadmap step 46 only. Audit the topic catalog against the implemented
  behavior and add focused integration experiments for the most important Reliability, Durability,
  History, and incompatible-QoS cases. Change policies only when evidence and scenario semantics
  require it; keep the tests deterministic. Add `docs/learning/46-qos-experiments.md`, showing how to
  run each experiment and predict its result. Acceptance: the catalog states final policies and
  points to a passing test that demonstrates each critical choice.

### 47 — Document a separate-machine demonstration

- [ ] **AI prompt:** Complete roadmap step 47 only. Add a concise runbook for building and running
  observer, console, and interceptor on one machine and on separate machines, including network
  prerequisites, domain configuration, discovery expectations, troubleshooting, and the exact
  operator interaction. Do not add a Discovery Server or transport customization unless the
  existing implementation cannot satisfy `VISION.md` and approval is obtained. Add
  `docs/learning/47-rtps-networking.md`, explaining at a high level how Fast DDS discovery and user
  data travel between hosts. Acceptance: a reader can reproduce the demonstration and distinguish
  application, DDS, RTPS, and network failures from the logs.

### 48 — Audit the result against the vision and architecture

- [ ] **AI prompt:** Complete roadmap step 48 only. Perform a final evidence-based audit of every
  `VISION.md` success criterion and every dependency rule in `architecture.md`. Add automated
  architecture checks where practical, remove no useful learning scaffolding, and fix only small
  gaps; report larger gaps as new roadmap items instead of hiding them in this review. Add
  `docs/learning/48-what-fast-dds-provides.md`, summarizing what Fast DDS supplied, what the
  application still had to implement, and how adapters permit future hardware replacement.
  Acceptance: the audit links each criterion to a passing test or reproducible demonstration and
  the complete verification workflow passes.

## Primary learning references

- [Fast DDS: What is DDS?](https://fast-dds.docs.eprosima.com/en/stable/fastdds/getting_started/definitions.html)
- [Fast DDS: DDS layer](https://fast-dds.docs.eprosima.com/en/stable/fastdds/dds_layer/dds_layer.html)
- [Fast DDS: Discovery](https://fast-dds.docs.eprosima.com/en/stable/fastdds/discovery/discovery.html)
- [Fast DDS-Gen for data type generation](https://fast-dds.docs.eprosima.com/en/stable/fastdds/dds_layer/topic/fastddsgen/fastddsgen.html)
- [Fast DDS versions and compatible tools](https://fast-dds.docs.eprosima.com/en/stable/notes/versions.html)
