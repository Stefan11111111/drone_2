# 36 — Operator start action

**Concept:** Human input, application intent, and DDS transport are separate boundaries. A terminal
string is untrusted presentation input; console core turns a valid request into a domain command;
only a later adapter decides how that command is distributed.

**In this project:** `TerminalAction` recognizes the replaceable `start <drone-id>` grammar and
passes a typed `DroneId` to `InterceptionCommandUseCase`. Core requires the drone to be known and in
the assigned state, derives the correlated target, assigns a process-local command identity, and
emits one `InterceptionCommand` through `InterceptionCommandOutputPort`. A repeated start is
reported as a duplicate. This step adds no command DDS endpoint.

**Try it:** run the focused core and terminal-adapter cases:

```bash
cmake --build --preset development --target console_core_test console_ui_adapter_test
ctest --preset development -R '^(ConsoleCore|TerminalAction)\.' --output-on-failure
```

The tests compare malformed input, unknown and unassigned drones, one valid assigned drone, and a
repeated action. Their capturing output ports show that only the valid action emits a command.

**Takeaway:** the UI parses syntax, core validates operational meaning, and the output port marks
the future DDS boundary. Each layer can change without moving operator policy into middleware code.
