# Drone interception demonstration runbook

This runbook reproduces the complete simulated interception with the `observer`, `console`, and
`interceptor` processes either on one Linux machine or on three machines on one trusted LAN. It
uses Fast DDS 3.3.0's built-in SIMPLE discovery and default UDPv4/SHM transports. It does not use a
Discovery Server, custom locators, or transport profiles.

The successful result is a console row under `Interception outcomes` that correlates event 1,
drone 1, and target 1. With the default geometry and speed, it normally appears about 25 seconds
after pursuit begins.

## Prerequisites

On every machine:

- use the same repository revision and the Ubuntu 26.04 dependency versions in
  [ADR 0001](decisions/0001-toolchain-and-dependencies.md);
- use IPv4 interfaces that can reach one another on the same multicast-capable LAN or VLAN;
- disable Wi-Fi client isolation and avoid NAT, containers, VPN boundaries, or routed subnets for
  this demonstration;
- allow bidirectional UDP discovery and user traffic for the chosen DDS domain through host and
  network firewalls; and
- use a trusted demonstration network: this project does not configure DDS authentication,
  encryption, or access control.

Check the revision, interfaces, routes, and Fast DDS environment before starting:

```bash
git rev-parse HEAD
ip -brief -4 address
ip route
env | rg '^(FASTDDS|FASTRTPS)'
```

All machines should report the same Git revision. No Fast DDS environment override is needed. In
particular, `FASTDDS_BUILTIN_TRANSPORTS=SHM` prevents communication between machines; unset such an
override before the demonstration.

This runbook uses DDS domain 47. Fast DDS's default RTPS port base is
`7400 + 250 * domain-id`, so domain 47 starts at UDP port 19150. The current executables create
several participants and let Fast DDS allocate participant IDs. The simplest firewall rule is
therefore to permit the entire domain block, UDP 19150-19399, bidirectionally between the three host
addresses. A narrower rule must account for every allocated participant ID:

| Traffic | Default port for domain 47 |
| --- | --- |
| Discovery multicast | 19150 |
| Discovery unicast | `19160 + 2 * participant-id` |
| User multicast, if configured | 19151 |
| User unicast | `19161 + 2 * participant-id` |

Do not blindly change firewall policy on a shared machine. Inspect the active policy with the
platform's normal tool, such as `sudo nft list ruleset` or `sudo ufw status`, and have the network
owner permit the domain block only between the demonstration hosts. ICMP `ping` is a useful route
check but does not prove that multicast or UDP is permitted.

## Build

Run this from the repository root on every machine that will host a role:

```bash
cmake --preset development
cmake --build --preset development
ctest --preset development -R '^CompleteVisionScenario\.' --output-on-failure
```

The test is the bounded one-machine rehearsal. It proves the executable, DDS contract, assignment,
start, pursuit, and outcome paths before network differences are introduced. A passing local test
does not prove that cross-machine UDP multicast works because Fast DDS can use shared memory between
local participants.

## Run on one machine

From the repository root, start all three roles in one terminal with:

```bash
./scripts/run-demonstration.sh
```

The script uses the commands below with DDS domain 47, combines their output in the current
terminal, and stops every role when you press Ctrl-C. Use `--domain-id ID` to select another domain
from 0 through 232. The development executables must already be built.

To keep each role's output in a separate terminal instead, run the following commands manually.
Startup order is not significant.

Terminal 1 — observer:

```bash
./build/development/observer \
  --domain-id 47 \
  --participant-name demo_observer \
  --tick-ms 250
```

Terminal 2 — interceptor:

```bash
./build/development/interceptor \
  --domain-id 47 \
  --participant-name demo_interceptor \
  --tick-ms 100
```

Terminal 3 — console and operator action:

```bash
./build/development/console \
  --domain-id 47 \
  --participant-name demo_console \
  --tick-ms 50 \
  --auto-pursuit
```

`--auto-pursuit` is the exact supported operator interaction in the current composition executable.
The operator selects this mode when starting the console. The console then waits until target 1 and
available drone 1 are visible and the interceptor's command readers are matched. It invokes the
same validated console-core use cases as the UI boundary to send exactly one assignment of drone 1
to target 1 and exactly one start command. The current executable does not read interactive
`assign` or `start` commands from standard input.

Observe these milestones rather than relying on a fixed startup delay:

1. `observer: publishing target 1 in DDS domain 47` and repeated `published target update` lines;
2. `interceptor: published available state for drone 1`;
3. the console's `matched observer TargetTrack writer`, `matched interceptor DroneState writer`,
   and `matched interceptor ExplosionEvent writer` lines;
4. console table rows for target 1 and available drone 1;
5. `console: automated assignment sent for drone 1 to target 1` followed by
   `console: automated interception start sent for drone 1`;
6. interceptor acceptance and `pursuit position` lines; and
7. a console outcome row beginning `1 | 1 | 1 |` under `Interception outcomes (1)`.

Press Ctrl-C once in each terminal after the outcome. Each process should end with
`shutdown complete`.

## Run on separate machines

Use one host per role. Build the same revision on all three hosts, choose interfaces on the same
multicast-capable LAN, and verify bidirectional host reachability before starting. From the
repository root on each host, run exactly one of these commands:

Observer host:

```bash
./build/development/observer \
  --domain-id 47 \
  --participant-name lan_observer \
  --tick-ms 250
```

Interceptor host:

```bash
./build/development/interceptor \
  --domain-id 47 \
  --participant-name lan_interceptor \
  --tick-ms 100
```

Console host:

```bash
./build/development/console \
  --domain-id 47 \
  --participant-name lan_console \
  --tick-ms 50 \
  --auto-pursuit
```

Watch the same seven milestones from the one-machine run. SIMPLE discovery periodically announces
participants, and the state Topics retain their latest samples while their writers remain alive,
so the roles may start in any order. The volatile assignment and start command are not sent until
the console has matched the corresponding interceptor readers.

This default deployment is intentionally limited to one multicast-capable network segment. If the
hosts must communicate across a router, NAT, multicast-blocking Wi-Fi, or a WAN, stop: initial peers,
a Discovery Server, or transport customization would be a new architecture decision outside this
roadmap step.

## Troubleshooting from the logs

Work from the top of this table downward. A later milestone depends on every earlier layer.

| Layer | Evidence and likely failure |
| --- | --- |
| Application process | Each role's startup line proves that its arguments were accepted and its DDS adapters were constructed. A line followed by `usage:` is a configuration error. A message such as `Fast DDS could not create participant` or `could not create Topic` is local DDS startup/resource failure, not a remote network failure. |
| DDS endpoint contract | Any console `matched ... writer` line proves participant discovery plus a compatible Topic name, wire type, and QoS for that endpoint. If some writers match and another never does, the network and RTPS discovery path work to the matched writer's host; inspect the missing role's host and startup log, then compare Git revisions. Run the step 46 QoS experiments if an incompatible build or policy is suspected. |
| RTPS discovery | If all processes print healthy startup lines but the console prints none of its three `matched` lines, first confirm all commands use domain 47. If the domain agrees, SIMPLE participant announcements or endpoint-discovery metatraffic is not reaching the other hosts. |
| Network | While the processes run, `sudo tcpdump -ni any 'udp portrange 19150-19399'` should show RTPS UDP traffic on every host. No outbound traffic suggests a local interface, environment, or firewall problem. Outbound but no inbound traffic suggests LAN multicast filtering, client isolation, routing, or the receiving firewall. Packet capture proves network delivery; it does not prove DDS endpoint compatibility. |
| Application data | Match lines with no target/drone rows mean discovery succeeded but user data did not complete the writer-to-reader path. Confirm observer update logs, then inspect the UDP capture for unicast user traffic. `discarded a malformed sample` is an adapter/schema problem; `discarded an instance-state notification` is DDS instance metadata rather than a usable state sample. |
| Operator control | No automated assignment means the console still lacks a valid target, an available drone, or the assignment-reader match. An assignment line without interceptor acceptance points to the assignment data path. A start line without `accepted interception start` points to the command data path. |
| Pursuit/outcome | `accepted interception start` without `pursuit position` is interceptor application/state-machine failure. Pursuit without an outcome after roughly 30 seconds calls for observer, interceptor, and console logs together; an outcome row proves the complete event writer, middleware delivery, DDS reader, correlation, and UI path. On separate hosts, that delivery uses RTPS over UDP. |

Useful bounded checks from the repository root are:

```bash
ctest --preset development -R '^TargetTrackDiscovery\.' -V
ctest --preset development -R '^QosExperiments\.' --output-on-failure
ctest --preset development -R '^CompleteVisionScenario\.' --output-on-failure
```

The discovery test distinguishes a successful endpoint match from a deliberate Reliability-QoS
non-match. The QoS experiments check the final contracts. The complete scenario captures all three
process logs and reports the first missing milestone.
