# 38 — Interceptor target reader

**Concept:** DDS distributes the target state; it does not call the observer or choose which samples
matter to an interceptor. The consuming application selects its assigned key and applies freshness
rules after mapping the wire sample into a domain value.

**In this project:** `TargetTrackSubscriber` reuses the catalog-backed `TargetTrackReader` and hands
valid samples to `TargetTrackInputPort` on the caller's thread. `InterceptorStateMachine` accepts
only its assigned target, retains the first sample and strictly newer measurements, and ignores
unrelated, duplicate, stale, and same-time conflicting samples. No target sample requests vehicle
motion in this step.

**Try it:** run the focused core and DDS adapter cases:

```bash
cmake --build --preset development --target interceptor_core_test interceptor_target_dds_adapter_test
ctest --preset development -R '^(InterceptorCore|InterceptorTargetDdsAdapter)\.' --output-on-failure
```

The DDS case publishes an unrelated track, a current assigned track, an older sample, and a newer
sample. It checks the retained core value after each bounded receive and confirms flight control was
never called.

**Takeaway:** the interceptor follows shared published state, not an in-process observer API. DDS
delivers candidate samples; assignment correlation and timestamp ordering remain core policy.
