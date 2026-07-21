# 47 — RTPS networking between hosts

## Concept

The application, DDS, RTPS, and the network solve different parts of one data path. The application
creates domain values and uses DDS DataWriters and DataReaders. DDS supplies data-centric Topics,
keys, QoS, and endpoint matching. RTPS is the interoperable wire protocol Fast DDS uses to discover
participants and endpoints and to carry serialized samples. UDPv4 or shared memory is the physical
transport beneath RTPS.

With Fast DDS's default SIMPLE discovery, SPDP announces DomainParticipants and SEDP exchanges their
DataWriter and DataReader descriptions. Matching still requires the same DDS domain, Topic, type,
and compatible QoS. On one machine Fast DDS can prefer shared memory; between machines the current
default configuration uses UDPv4. Fast DDS enables default UDPv4 discovery and peer-to-peer locators
and derives their ports from the domain and participant IDs.

The official Fast DDS 3.3 documentation describes
[SIMPLE participant and endpoint discovery](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/discovery/simple.html),
[the default UDP transport](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/transport/udp/udp.html),
and the [RTPS well-known port formulas](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/transport/listening_locators.html).

## In this project

`DomainParticipantOwner` leaves discovery and transports at their Fast DDS defaults while giving
every participant an explicit domain ID and name. The observer's target writer, the interceptor's
state and event writers, and the console's readers therefore need no host addresses. SIMPLE
discovery exchanges their locators, endpoint descriptions, and QoS; the same adapter code works
whether all processes share a host or run on separate hosts.

The [demonstration runbook](../demonstration-runbook.md) fixes domain 47, documents its UDP port
block, gives exact commands for both topologies, identifies the supported `--auto-pursuit` operator
action, and maps observable log milestones to application, DDS, RTPS, and network failures. No
Discovery Server or custom transport was needed for the vision's same-LAN demonstration.

## Try it

First run the bounded local rehearsal:

```bash
cmake --build --preset development --target end_to_end_interception_process_test
ctest --preset development -R '^CompleteVisionScenario\.' --output-on-failure
```

For an observable manual run without opening three terminals, launch all roles together and press
Ctrl-C after the correlated outcome appears:

```bash
./scripts/run-demonstration.sh
```

Then follow the runbook on separate machines. Compare the console's `matched ... writer` lines with
a UDP capture on each host:

```bash
sudo tcpdump -ni any 'udp portrange 19150-19399'
```

A match proves that RTPS discovery delivered compatible DDS endpoint metadata. Visible RTPS packets
without a match direct investigation toward domain, Topic/type, or QoS configuration; no incoming
packets direct it toward interfaces, multicast, routing, or firewalls.

## Takeaway

DDS describes and matches the distributed data contract; RTPS discovers peers and encodes that
contract and its samples on the wire; UDP/IP and the LAN merely carry those packets. A local DDS
success can use shared memory, so a separate-host run is the experiment that proves the default
RTPS-over-UDP network path.
