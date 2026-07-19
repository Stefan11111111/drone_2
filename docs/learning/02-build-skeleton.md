# 02 — C++26 build skeleton

## Concept

A build contract makes every later Fast DDS experiment start from the same language rules. CMake
selects strict C++26, requires the selected compiler to support it, and disables compiler-specific
language extensions. CTest then runs a tiny executable that was compiled under that contract.

## In this project

[`CMakeLists.txt`](../../CMakeLists.txt) accepts the toolchain chosen in step 01 and applies C++26 to
the repository. [`CMakePresets.json`](../../CMakePresets.json) exposes the same GCC 15 and Ninja
configuration to VS Code's CMake Tools extension and the command line.
[`tests/cpp26_build_probe.cpp`](../../tests/cpp26_build_probe.cpp) verifies the language-mode macro
and exits successfully as the repository's first test. This repository compiles that probe; Fast
DDS, Fast CDR, generated wire types, and application behavior are not part of this step and will be
supplied or added by later roadmap steps.

## Try it

The workspace setting in [`.vscode/settings.json`](../../.vscode/settings.json) makes the CMake Tools
extension use repository presets instead of compiler kits. After opening or reloading the workspace,
run **CMake: Select Configure Preset**, choose **Development (GCC 15, Ninja)**, and then run
**CMake: Build**. **CMake: Configure All Projects** configures the preset already selected for each
project; it is not the command that selects a preset. The equivalent terminal workflow is:

```bash
cmake --list-presets
cmake --preset development
cmake --build --preset development
ctest --preset development -R '^cpp26_build_probe$'
ctest --preset development
```

The first CTest command is the focused check; the second is the complete suite available at this
step. To see the expanded compiler invocation and its strict `-std=c++26` option, run:

```bash
ninja -C build/development -t commands cpp26_build_probe
```

The equivalent commands without the shared preset are:

```bash
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=g++-15
cmake --build build
ctest --test-dir build --output-on-failure -R '^cpp26_build_probe$'
ctest --test-dir build --output-on-failure
```

During implementation, the recorded tool versions and compiler mode were also checked with:

```bash
cmake --version
ninja --version
g++-15 --version
printf 'int main() {}\n' | g++-15 -x c++ -std=c++26 -pedantic-errors -fsyntax-only -
```

## Takeaway

The application now has a reproducible, strict language baseline. Middleware integration remains a
separate concern, so a later Fast DDS failure can be distinguished from a basic compiler or build
configuration failure.
