# Simulated vehicle adapter

**Concept:** An application core should request positioning and motion capabilities, not depend on a
particular simulator or flight controller. Ports express those capabilities while an adapter
supplies their environment-specific behavior.

**In this project:** `SimulatedVehicle` implements the `PositioningPort` and `FlightControlPort`
owned by `interceptor_core`. A movement call advances an explicit simulation time step and moves in
three dimensions by at most `maximumSpeedMetersPerSecond * timeStep`. It chooses neither a target
nor when to move; those decisions remain future state-machine behavior.

**Try it:** run the deterministic adapter tests:

```bash
cmake --build --preset development --target simulated_vehicle_adapter_test
ctest --preset development -R '^SimulatedVehicle\.' --output-on-failure
```

The cases make no-movement, bounded movement, exact arrival, and invalid configuration observable
without wall-clock sleeps.

**Takeaway:** a physical deployment can replace this adapter with positioning and flight-control
implementations while preserving the interceptor core and every DDS contract around it.
