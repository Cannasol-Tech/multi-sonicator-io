# 1. Goals and Background Context

## 1.1. Goals

* Increase throughput per PLC cell by at least 3x compared to the single-channel baseline.
* Reduce operator intervention time per batch by 50% or more.
* Decrease unplanned downtime from overload events by at least 30% through a standardized reset flow.
* Ensure the PLC receives per-unit telemetry at the required cadence with less than 1% packet loss.
* Enable the HMI to start, stop, or reset any connected sonicator unit within a 200ms command latency.
* Maintain amplitude control accuracy within ±2% of the setpoint during steady-state operation.

## 1.2. Background Context

Currently, operations depend on manual or single-channel control of CT2000 sonicator units. This approach creates significant production bottlenecks, leads to inconsistent processing outcomes, and relies on fragmented, ad-hoc telemetry. The lack of a standardized, automated overload reset procedure further contributes to preventable downtime. These inefficiencies cap throughput, increase operator workload, and introduce quality assurance variability.

The proposed solution is a microcontroller-based multiplexer designed to manage up to four CT2000 units concurrently. It will expose a stable, well-defined MODBUS RTU interface for seamless integration with the primary PLC/HMI system. This system will provide granular, per-unit control (amplitude, start/stop, reset) and real-time monitoring (power, frequency, status), all validated through a comprehensive hardware-in-the-loop (HIL) testing framework to guarantee reliability before deployment.

## 1.3. Change Log

| Date       | Version | Description                             | Author    |
| :--------- | :------ | :-------------------------------------- | :-------- |
| 2025-09-01 | 1.0     | Initial draft based on Project Brief.   | John (PM) |
| 2025-09-01 | 1.1     | Updated requirements after brainstorming. | John (PM) |
