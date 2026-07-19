# Vision

## Purpose

This project demonstrates a distributed drone-interception system whose software architecture can move from simulation to real hardware without redesigning the core applications.

The demonstration is entirely simulated. An interception succeeds when a drone reaches the current position of its assigned target, at which point the simulation emits an explosion event.

## System Roles

The system consists of independently deployable participants:

- **Observers** detect and track targets. A simulated radar is the initial observer.
- **Consoles** present tracks and system status to a human operator. The operator can assign a drone to a target and command an interception. Automated decision-making may be added later without changing the other participants.
- **Interceptor drones** receive assignments, navigate toward the latest reported target position, report their status, and trigger the simulated explosion when they reach the target.

## Demonstration Scenario

The initial demonstration covers the complete operational loop:

1. An observer detects and tracks a moving target.
2. The observer distributes the target track to the rest of the system.
3. A console displays the target to a human operator.
4. The operator assigns an available drone and commands an interception.
5. The drone follows the updated target position while reporting its own state.
6. When the drone reaches the target position, the simulation records and distributes an explosion event.
7. The console displays the outcome of the interception.

## Architecture

All participants communicate through **eProsima Fast DDS**. Their data contracts must not depend on whether a participant is connected to simulated or physical equipment.

The same core observer, console, and drone applications should run in both environments. Hardware-dependent behavior—such as radar input, vehicle positioning, and flight control—belongs behind replaceable adapters. The simulation supplies simulated adapters; a hardware deployment would replace them with device-specific implementations.

Participants should be runnable as separate processes and, eventually, on separate machines. No participant may depend on another participant's in-process implementation details.

## Initial Success Criteria

The project fulfills its initial vision when it can demonstrate that:

- the observer, console, and drone run as separate participants and exchange information through Fast DDS;
- the console shows live target and drone state;
- a human operator can assign a drone to a tracked target;
- the drone responds to updated target positions;
- reaching the target produces a simulated explosion event that is visible on the console; and
- simulation-specific behavior is isolated behind adapters so the core applications remain reusable for future hardware integration.

## Scope

The project demonstrates the software architecture and end-to-end interaction. It does not include physical drone, radar, or explosive hardware. Automated interception decisions are a possible future extension; the initial demonstration keeps the human operator in control of target assignment and the interception command.
