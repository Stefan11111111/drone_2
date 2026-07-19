# 10 — Topics, keys, and QoS

## Concept

A DDS Topic is a named, typed data flow. A key divides that flow into instances: samples with the
same key update the same target, drone, command, or event, while different keys remain independent.
A DataWriter and DataReader still need compatible Quality of Service (QoS) requests and offers
before discovery can match them.

Reliability describes whether missing samples are retransmitted. Durability decides whether a new
reader can receive samples written before it matched. History and resource limits decide how many
samples and keyed instances the middleware may retain. These transport behaviors do not replace
application validation, stale-update rejection, or duplicate handling.

## In this project

`docs/dds-topic-catalog.md` centralizes the five initial flows: target tracks, drone state,
assignments, interception commands, and explosion events. Each row records one stable topic name,
future IDL type and key, producer and consumers, update pattern, and proposed QoS.

Current target and drone snapshots are retained for late joiners. Operator intent is volatile so it
is not replayed into a restarted interceptor. Outcomes are retained while their interceptor writer
lives. All histories and instance counts are bounded. These are explicit hypotheses that later
matching, late-joiner, process, and QoS experiments must confirm.

## Try it

Run the focused catalog check from the repository root:

```bash
cmake --preset development
ctest --preset development -R '^VerificationWorkflow.TopicCatalog$'
```

Then inspect the catalog and predict whether a `VOLATILE` reader created after a sample was written
will see it, and whether a reader requesting `RELIABLE` can match a writer offering `BEST_EFFORT`.
Steps 20, 32, and 46 turn those predictions into executable Fast DDS experiments. The Fast DDS
3.3.0 documentation describes [keyed Topic
instances](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/topic/instances.html) and
the [QoS compatibility
rules](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/core/policy/standardQosPolicies.html).

## Takeaway

Matching a name and type establishes what data can flow; keys establish which logical object a
sample updates; QoS establishes how Fast DDS attempts to deliver and retain it. The application
still owns the operational meaning and validation on either side of that boundary.
