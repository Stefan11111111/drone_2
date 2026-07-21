# 46 — QoS experiments

## Concept

Quality of Service is an executable agreement between a DataWriter and DataReader, not merely a
configuration label. Reliability and Durability participate in requested/offered matching. History
and resource limits govern the samples an endpoint can retain after it matches. A keyed Topic
applies `KEEP_LAST(1)` independently to every key, so one target update does not evict another
target's latest state.

## In this project

`qos_experiments_test.cpp` audits the QoS factories used by all five Topic implementations and then
varies one condition at a time. The experiments confirm that transient-local state reaches a late
reader, only the newest target sample per key remains, volatile operator intent is not replayed, and
a durability mismatch prevents discovery from matching endpoints. The existing
`target_track_discovery_test.cpp` performs the corresponding Reliability mismatch experiment.

The audit found no difference between the catalog and the implemented endpoints, so step 46 keeps
the policies unchanged. The capacity experiment also confirms the documented 64-target bound:
Fast DDS accepts the first 64 retained keys and the transport reports the 65th write failure rather
than silently hiding it.

## Try it

Build and run the focused experiments:

```bash
cmake --build --preset development --target qos_experiments_test target_track_discovery_test
ctest --preset development -R '^(QosCatalogAudit|QosExperiments|TargetTrackDiscovery)\.' --output-on-failure
```

Predict these results before running them:

| Experiment | Prediction |
| --- | --- |
| Catalog audit | Every writer and reader reports the policy and limits in `docs/dds-topic-catalog.md` |
| Reliable reader versus best-effort writer | No match; both incompatibility callbacks identify Reliability |
| Transient-local target history | A late reader receives two keys, the newer value for the updated key, and no superseded value |
| Volatile assignment history | A pre-match assignment is absent; a new post-match assignment arrives |
| Transient-local reader versus volatile writer | No match; both incompatibility callbacks identify Durability |
| Target capacity | Writes for 64 keys succeed; the 65th reports a resource error |

The Reliability experiment deliberately tests the deterministic compatibility rule. A successful
localhost delivery cannot prove that retransmission happened because it may lose no packets;
injecting transport loss would require infrastructure outside this roadmap step.

## Takeaway

QoS choices become trustworthy when configuration assertions, matching callbacks, retained-data
behavior, and capacity failures all agree with scenario semantics. DDS handles delivery and
retention within those bounds; application identities and validation still decide whether a
received command or event should be acted upon.
