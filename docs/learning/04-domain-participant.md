# 04 — Domain participant

## Concept

A DDS domain is a logical communication space identified by a numeric domain ID. Participants in
different domains do not discover one another, while participants in the same domain can discover
compatible endpoints. A `DomainParticipant` gives one running application its identity and entry
point into that space; publishers, subscribers, topics, readers, and writers are created beneath it.

## In this project

`tests/domain_participant_smoke_test.cpp` asks the Fast DDS factory to create one participant in
domain `0` with the name `drone_step_04_smoke`. The test deliberately creates no Topic, Publisher,
or Subscriber yet, so it isolates middleware linkage and participant lifetime from later data-flow
lessons. `CMakeLists.txt` requires the approved Fast DDS 3.3.0 and Fast CDR 2.3.5 packages exactly
and links the smoke target directly to Fast DDS.

The participant is deleted through the same factory that created it. The successful return code is
the observable proof that this test left no participant-owned DDS entities behind. See the
[Fast DDS 3.3 DomainParticipant documentation](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/dds_layer/domain/domainParticipant/domainParticipant.html)
for the API and entity-hierarchy context.

## Try it

Configure, build, and run only this experiment from the repository root:

```bash
cmake --preset development
cmake --build --preset development --target domain_participant_smoke_test
ctest --preset development -R '^DomainParticipantSmoke\.'
```

CTest reports one passing test. To observe the dependency guard, configure in a disposable build
directory with either the Fast DDS or Fast CDR package disabled; CMake stops before compilation:

```bash
cmake -S . -B /tmp/drone-missing-fastdds -G Ninja \
  -DCMAKE_CXX_COMPILER=g++-15 -DCMAKE_DISABLE_FIND_PACKAGE_fastdds=TRUE
```

## Takeaway

Before an application can publish or receive data, it must join a DDS domain as a participant. This
small test proves that the approved middleware is present, linkable, able to create that root DDS
entity, and able to release it cleanly.
