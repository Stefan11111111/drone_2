# 20 — Discovery and endpoint matching

## Concept

Fast DDS first discovers participants in a domain, then exchanges endpoint information for their
DataWriters and DataReaders. Two discovered endpoints match only when their Topic name and wire type
agree and the writer's offered QoS satisfies the reader's requested QoS. Discovery therefore does
not by itself mean that samples can flow.

### What QoS policies exist, and when are they compatible?

QoS is not one setting. It is a collection of policies describing the service an endpoint offers
or requires. The most useful groups for this project are:

| Question | Main policies | What they control |
| --- | --- | --- |
| Will data arrive? | Reliability | Whether delivery is best effort or retransmitted after loss |
| Can a late reader see old data? | Durability | Whether samples exist only for current matches or are retained |
| How much data is kept? | History, Resource Limits | Which samples remain in memory and the maximum bounded capacity |
| Is data timely and alive? | Deadline, Lifespan, Liveliness, Latency Budget | Update expectations, expiry, writer health, and delivery delay |
| Which value wins? | Destination Order, Ownership, Ownership Strength | Ordering and arbitration when several writers publish an instance |
| Which endpoints belong together? | Partition, Presentation | Logical grouping and coherent access to related samples |

DDS applies a **requested/offered** rule to matching policies: the DataWriter offers a service level
and the DataReader requests its minimum acceptable level. Every matching policy must be compatible.
“Stronger” is defined separately for each policy; it does not mean that all QoS values use one
universal ordering.

The four policies configured for target tracks behave as follows:

| Policy | Values or rule | Does it prevent a match? |
| --- | --- | --- |
| Reliability | `BEST_EFFORT < RELIABLE`; writer must be at least the reader's request | Yes |
| Durability | `VOLATILE < TRANSIENT_LOCAL < TRANSIENT < PERSISTENT`; writer must be at least the reader's request | Yes |
| History | `KEEP_LAST(depth)` or `KEEP_ALL`; controls each endpoint's stored samples | No requested/offered comparison |
| Resource Limits | Bounds samples, instances, and samples per instance; must be consistent with History | No requested/offered comparison |

For Reliability, all combinations are small enough to show explicitly:

| Writer offers | Reader requests | Compatible? |
| --- | --- | --- |
| `BEST_EFFORT` | `BEST_EFFORT` | Yes |
| `BEST_EFFORT` | `RELIABLE` | **No** |
| `RELIABLE` | `BEST_EFFORT` | Yes |
| `RELIABLE` | `RELIABLE` | Yes |

Durability uses the same “writer at least reader” shape. For example, a `TRANSIENT_LOCAL` writer
matches `VOLATILE` and `TRANSIENT_LOCAL` readers, but not a reader requesting `TRANSIENT` or
`PERSISTENT`. This project uses `TRANSIENT_LOCAL` on both target endpoints, so late readers can
receive retained target state while the writer remains alive.

Other policies can also participate in matching. For example, Deadline and Latency Budget require
the writer's offered duration to be no greater than the reader's requested duration; Ownership
must agree; and Liveliness, Destination Order, and Presentation have their own ordered rules. This
project leaves those policies at compatible Fast DDS defaults. The complete list and policy-specific
rules are in the official
[Fast DDS standard QoS policy documentation](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/core/policy/standardQosPolicies.html).

Fast DDS reports an incompatible matching policy through the offered/requested incompatible-QoS
statuses rather than a publication or subscription match. The relevant listener callbacks are
described in the official
[DataWriterListener](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/publisher/dataWriterListener/dataWriterListener.html)
and
[DataReaderListener](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/subscriber/dataReaderListener/dataReaderListener.html)
documentation.

## In this project

`TargetTrackWriter` and `TargetTrackReader` now listen for match and incompatible-QoS callbacks.
Each exposes a small `TargetTrackDiscoveryStatus` snapshot containing current and cumulative match
counts plus the count and last policy from incompatible-QoS reports. Their bounded wait methods use
a condition variable notified by those callbacks; application code and tests do not guess how long
discovery should take.

The normal constructors still use the catalog's
`RELIABLE / TRANSIENT_LOCAL / KEEP_LAST(1)` policies. The focused experiment can explicitly give a
writer `targetTrackBestEffortWriterQosForDiscoveryExperiment()`, which changes only Reliability and
is named so it cannot be mistaken for the operational policy.

`target_track_discovery_test.cpp` runs two experiments in separate domains:

1. Matching Topic names, generated type names, and catalog QoS produce one publication match and one
   subscription match.
2. The same Topic and type with a `BEST_EFFORT` writer and `RELIABLE` reader produce no match; both
   endpoints receive an incompatibility callback naming the Reliability policy.

## Try it

Build and run the two experiments verbosely so their status lines are visible:

```bash
cmake --preset development
cmake --build --preset development --target target_track_discovery_test
ctest --preset development -V -R '^TargetTrackDiscovery\.'
```

The passing experiment logs `MATCH` with one peer at each endpoint. The controlled non-match logs
`NON-MATCH` and `RELIABILITY`. Both tests have a five-second failure bound, but success is triggered
by a DDS callback rather than elapsed time.

## Takeaway

Participant discovery only makes remote entities known. Endpoint matching additionally requires
the same Topic and compatible type and QoS contracts. Fast DDS exposes positive matches and QoS
rejections as concrete status events, so software can observe both outcomes deterministically.
