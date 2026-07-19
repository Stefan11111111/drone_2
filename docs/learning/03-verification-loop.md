# 03 — Verification loop

## Concept

Fast feedback keeps later DDS experiments trustworthy. Formatting removes incidental style changes,
static analysis catches suspicious C++ before execution, and a focused unit test tells us whether a
small behavior works in isolation. The complete test suite then checks that the change did not break
an earlier lesson.

## In this project

`.clang-format` and `scripts/format.sh` define and check one C++ format. `.clang-tidy` and
`scripts/run-clang-tidy.sh` analyze the repository's handwritten C++ using the compilation database
exported by CMake. `CMakeLists.txt` finds the system GoogleTest 1.17.x package, and
`tests/verification_workflow_test.cpp` supplies one discovered sample test. `AGENTS.md` records the
order every later roadmap step follows. Because there are no C++ module units, CMake's automatic
module scan is disabled so its GCC compilation commands remain usable by clang-tidy 21. None of
these pieces introduces DDS or application behavior; they make failures in those later layers easier
to localize.

## Try it

From the repository root, run the same sequence used to verify this step:

```bash
./scripts/format.sh --write
./scripts/format.sh --check
cmake --preset development
./scripts/run-clang-tidy.sh
cmake --build --preset development
ctest --preset development -R '^VerificationWorkflow\.'
ctest --preset development
```

The focused CTest command runs only the GoogleTest workflow probe. The final command also runs the
earlier C++26 build probe. To observe a useful failure, temporarily replace `SUCCEED()` in the sample
with `FAIL() << "intentional failure";`, rebuild, and rerun the focused test; restore the line when
finished.

## Takeaway

A DDS test is only informative when the ordinary C++ feedback loop is stable. This workflow makes
style, configuration, static-analysis, compilation, focused behavior, and regression checks separate
and repeatable observations.
