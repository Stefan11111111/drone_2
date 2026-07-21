# Interceptor state publisher

**Concept:** A keyed DDS Topic treats each key value as an independent instance. Rewriting the same
drone key updates that drone's state history; it does not create a new drone. With `KEEP_LAST(1)`,
the writer retains only the newest sample for each instance.

**In this project:** `DroneStateTopic` centralizes the catalog name `drone.drone_states` and its
`RELIABLE / TRANSIENT_LOCAL / KEEP_LAST(1)` QoS with limits for 16 keyed drones.
`DroneStateWriter` maps the domain value to the generated IDL type, and `DroneStatePublisher`
implements the interceptor core's output port. The `interceptor` composition executable wires that
publisher to `InterceptorStateMachine` and `SimulatedVehicle`, then reports drone 1 as available.

**Try it:** run the separate-process probe:

```bash
cmake --build --preset development --target interceptor_publisher_process_test
ctest --preset development -R '^InterceptorPublisher\.' --output-on-failure
```

The probe creates a reader, launches the interceptor process, waits with a bound for DDS data, and
checks the mapped available state before cleaning up the child process.

**Takeaway:** the state-machine output call becomes a keyed DDS sample through an adapter; neither
the core nor the simulated vehicle knows about generated types, Topics, or DataWriters.
