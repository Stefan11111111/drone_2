# 01 — Fast DDS toolchain

## Concept

[Fast DDS](https://fast-dds.docs.eprosima.com/en/v3.3.0/fastdds/getting_started/definitions.html)
is the runtime middleware: it discovers participants and moves typed data according to DDS QoS.
Fast CDR is the serialization library that turns typed samples into bytes and back. Fast DDS-Gen is
a build-time Java tool that reads IDL and produces C++ data and type-support files. The application
owns the operational behavior and uses those middleware pieces at its DDS adapter boundary.

## In this project

The exact tool versions and generated-source rules are recorded in
[`docs/decisions/0001-toolchain-and-dependencies.md`](../decisions/0001-toolchain-and-dependencies.md).
The future `drone_dds_types` library will generate wire types from committed IDL, while
`drone_dds_transport` will use Fast DDS and Fast CDR. The domain and core libraries remain unaware of
all three tools, preserving the dependency boundaries in `architecture.md`.

## Try it

Compare the runtime middleware version with the build-time generator version:

```bash
fastdds --version
fastddsgen -version
dpkg-query -W -f='${binary:Package}\t${Version}\n' libfastdds-dev libfastcdr-dev fastddsgen
```

For this step the output was Fast DDS 3.3.0, Fast CDR 2.3.5, and Fast DDS-Gen 4.2.0. The complete
availability and C++26 probe commands are in the decision record. There was no repository configure,
build, or test command yet; those workflows belong to roadmap steps 02 and 03. The temporary IDL
compatibility probe generated into `/tmp`; its `TopicDataType` compiled, linked, and constructed with
`g++ -std=c++26 -pedantic-errors`, and no probe source was retained in the repository.

## Takeaway

DDS is not the application's data model. IDL defines the wire contract, Fast DDS-Gen creates the
serialization-facing C++, Fast CDR encodes it, Fast DDS transports it, and application adapters keep
those concerns out of core behavior.
