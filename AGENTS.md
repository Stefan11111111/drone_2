# Repository Agent Guide

## Mission

- Read `VISION.md`, `architecture.md`, and `ROADMAP.md` before planning or implementation and keep
  changes aligned with all three.
- Ask before making hard-to-reverse product or architecture decisions that `VISION.md` does not
  resolve.

## Learning Project

- Treat teaching how Fast DDS works as a required outcome, not as optional documentation.
- Work on one numbered `ROADMAP.md` step at a time. Do not combine, skip, or reorder steps without
  explicit approval.
- Keep each change small enough that a reviewer can identify its single main concept, trace its data
  flow, and verify it independently.
- Every roadmap step shall add its named `docs/learning/NN-*.md` note. The note shall briefly cover
  the concept, where the step demonstrates it, a hands-on way to observe it, and the learner's main
  takeaway.
- Tie explanations to the code and behavior introduced in the step. Prefer plain language and small
  experiments over broad theory or large pasted listings.
- After verification, mark only the completed roadmap step as done. If acceptance criteria do not
  pass, leave it unchecked and report the blocker.

## Required Skills

- Use `$cpp-instructions` for every C++ task.
- Use `$clean-code-refactoring` for refactoring, cleanup, simplification, or structural review.
- Repository-specific guidance in this file takes precedence over skill defaults when they differ.

## Repository Contract

- Use C++26. Require it in the build configuration and disable non-standard compiler extensions.
- Do not downgrade the language standard without approval.
- Preserve the one-way library dependencies in `architecture.md`. Core and domain libraries shall
  not acquire Fast DDS, generated-type, UI, or simulation dependencies.
- Keep IDL as the source of truth for DDS wire types. Never manually edit generated Fast DDS-Gen
  output; follow the generated-source policy recorded by roadmap step 01.
- Keep topic names, keys, and non-default QoS policies centralized and documented in the DDS topic
  catalog introduced by roadmap step 10.
- Use the DDS PIM API for the initial project. Do not drop to RTPS internals or introduce advanced
  Fast DDS infrastructure unless a roadmap step requires it.
- Keep application behavior out of middleware callbacks when it could block a Fast DDS thread; hand
  work across an explicit adapter boundary.
- Processes communicate with other roles only through Fast DDS, including tests of cross-role
  behavior.

## Verification

- Run the canonical workflow from the repository root, in this order:

  ```bash
  ./scripts/format.sh --check
  cmake --preset development
  ./scripts/run-clang-tidy.sh
  cmake --build --preset development
  ctest --preset development -R '^VerificationWorkflow\.'
  ctest --preset development
  ```

- Use `./scripts/format.sh --write` to format C and C++ files before running the format check.
- For each step, run focused tests first and the complete available suite before marking the step
  complete. Process-level tests shall have bounded waits, actionable failure output, and reliable
  child-process cleanup; do not use fixed sleeps as a substitute for DDS match or readiness signals.
- Whenever a step changes tooling or adds a canonical workflow, update this section in the same
  review.
