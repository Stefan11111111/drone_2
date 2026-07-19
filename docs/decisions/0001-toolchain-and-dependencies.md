# ADR 0001: Toolchain and dependency baseline

- **Status:** Accepted
- **Date:** 2026-07-19
- **Scope:** Roadmap step 01; tooling decisions only

## Context

The project needs a reproducible C++26 toolchain before it adds build targets or application code.
The available reference environment is Ubuntu 26.04 LTS on `amd64`. Its packaged Fast DDS stack was
also checked with a small, temporary IDL generation, compilation, linking, and construction probe.

These choices are deliberately replaceable. A version change requires updating this record and
rerunning the complete verification workflow, but it does not change the dependency directions in
`architecture.md`.

## Decision

| Concern | Decision and supported version rule | Reference environment |
| --- | --- | --- |
| Build system | CMake `>= 3.30, < 5`; generate Ninja build files | CMake 4.2.3, Ninja 1.13.2 |
| C++ compiler | GCC 15.2.x using `g++-15`; require `-std=c++26` and reject GNU extensions | GCC 15.2.0 |
| Test framework | GoogleTest 1.17.x, found as the system CMake package | GoogleTest 1.17.0 |
| Formatter | `clang-format-21`; the major version is part of the format contract | clang-format 21.1.8 |
| Static analysis | `clang-tidy-21` using CMake compile commands | clang-tidy 21.1.8 |
| DDS middleware | System Fast DDS 3.3.0 and Fast CDR 2.3.5, both required exactly by CMake | Fast DDS 3.3.0, Fast CDR 2.3.5 |
| IDL generator | System `fastddsgen` 4.2.0 required exactly | Fast DDS-Gen 4.2.0+dfsg |
| Generator runtime | The Java runtime supplied with the reference OS | OpenJDK 25.0.3 |

[CMake 3.30 added the `cxx_std_26` compile
feature](https://cmake.org/cmake/help/v4.0/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html), so it is the
minimum. GCC is the sole supported compiler for the initial learning path; Clang may be added later
as a second compiler after it has its own build-and-test job. Versioned LLVM commands prevent
formatter or analysis behavior from changing when an unversioned system alternative changes.

Use CMake with `CMAKE_CXX_STANDARD=26`, `CMAKE_CXX_STANDARD_REQUIRED=ON`, and
`CMAKE_CXX_EXTENSIONS=OFF`. Step 02 will encode that build contract. Step 03 will add GoogleTest,
format configuration, static-analysis configuration, and the canonical commands; this step does not
preempt either task.

### Dependency acquisition

The reference setup uses Ubuntu packages rather than vendoring, `FetchContent`, or a source build.
Install the exact reference packages with:

```bash
sudo apt-get update
sudo apt-get install \
  cmake=4.2.3-2ubuntu2 \
  ninja-build=1.13.2-1 \
  g++-15=15.2.0-16ubuntu1 \
  googletest=1.17.0-1build1 \
  libgtest-dev=1.17.0-1build1 \
  clang-format-21=1:21.1.8-6ubuntu1 \
  clang-tidy-21=1:21.1.8-6ubuntu1 \
  libfastdds-dev=3.3.0+ds-3ubuntu1 \
  fastdds-tools=3.3.0+ds-3ubuntu1 \
  libfastcdr-dev=2.3.5-1 \
  fastddsgen=4.2.0+dfsg-1
```

The [Fast DDS 3.3 compatibility
table](https://fast-dds.docs.eprosima.com/en/v3.3.0/notes/versions.html) names Fast DDS-Gen 4.1.x as
its related release, while Ubuntu 26.04 packages generator 4.2.0 with Fast DDS 3.3.0. This repository
accepts that exact distro combination because a generated `TopicDataType` compiled, linked, and was
constructed against the installed Fast DDS 3.3.0 and Fast CDR 2.3.5 libraries. The generated header's
`FASTDDS_GEN_API_VER` check also accepted the installed API. Step 11 must repeat that proof with the
project's real IDL. Other Fast DDS/Fast DDS-Gen combinations are unsupported until this record and
the generation compile test are updated together.

## Generated-source policy

- IDL files are the source of truth and are committed under the `drone_dds_types` source tree.
- Fast DDS-Gen output is not committed. CMake generates it below the build directory and compiles it
  into `drone_dds_types`.
- Generation is a build dependency of the IDL input, so editing IDL or starting a clean build reruns
  `fastddsgen`. The project owns its CMake targets and does not use `fastddsgen -example` output.
- Generated files are never edited, formatted, or analyzed manually. Fix the IDL, generation command,
  or generator version instead.
- Configuration fails clearly if the exact generator or middleware versions are unavailable. CI and
  developer builds use the same generation path.

This policy keeps reviews focused on the wire contract, makes regeneration observable, and avoids
committed output that can drift from its IDL or generator.

## Availability checks

Run these commands before configuring the future build:

```bash
cmake --version
ninja --version
g++-15 --version
printf 'int main() {}\n' | g++-15 -x c++ -std=c++26 -pedantic-errors -fsyntax-only -
pkg-config --modversion gtest
clang-format-21 --version
clang-tidy-21 --version
fastdds --version
fastddsgen -version
java -version
dpkg-query -W -f='${binary:Package}\t${Version}\n' \
  cmake ninja-build g++-15 googletest libgtest-dev clang-format-21 clang-tidy-21 \
  libfastdds-dev fastdds-tools libfastcdr-dev fastddsgen
```

The version output must satisfy the table above; the C++ probe must exit successfully. Starting with
step 11, the generated-type compile test is the authoritative compatibility check for Fast DDS,
Fast CDR, and Fast DDS-Gen.

## Consequences

Developers need the generator even though it is not a runtime dependency. Clean builds can reproduce
wire types from IDL, and generated churn does not obscure reviews. Upgrading the distro or DDS stack
is an explicit decision with a generation-and-compilation check rather than an accidental local
change.
