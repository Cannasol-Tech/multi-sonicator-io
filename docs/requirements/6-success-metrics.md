# 6. Success Metrics

The success of this project will be measured against the following key performance indicators (KPIs), which are directly tied to the project goals.

## 6.1. Primary Metrics

* **Throughput Increase**: The system must demonstrate control of at least three sonicator units simultaneously in a production-like environment, validating the 3x throughput goal.
* **Command Latency**: End-to-end command latency (from PLC command issuance to sonicator action) must be verified to be under 200ms for all control actions (start, stop, reset).
* **Telemetry Accuracy & Cadence**:
  * Power and frequency telemetry must update in MODBUS registers at a rate of at least 10Hz.
  * Amplitude control must be maintained within ±2% of the setpoint.
* **System Reliability**:
  * The system must pass all HIL acceptance tests without failure for a continuous 24-hour soak test.
  * The watchdog timer must correctly reset the system into a safe state upon a forced software hang.

## 6.2. Secondary Metrics

* **Test Coverage**: Unit test coverage must meet or exceed the 85% target.
* **Documentation Quality**: All public APIs in the firmware must be fully documented with Doxygen comments, and the generated documentation must be complete and free of errors.
* **Integrator Satisfaction**: The integration team must confirm that the provided MODBUS register map and documentation are clear and sufficient for PLC/HMI integration.
