# 2. Requirements

## 2.1. Functional Requirements

1. **FR1**: The system shall control up to four CT2000 sonicator units concurrently via a single interface.
2. **FR2**: The system shall provide individual start and stop control for each connected sonicator unit. The `START` command being active is the strict definition of a unit being "active".
3. **FR3**: The system shall provide individual amplitude control for each sonicator unit. If a value outside the 20-100% range is commanded, the system shall clamp it to the nearest valid limit (20% or 100%).
4. **FR4**: The system shall provide an individual overload reset command for each sonicator unit.
5. **FR5**: The system shall monitor and report the real-time power consumption (in Watts) for each sonicator unit.
6. **FR6**: The system shall monitor and report the real-time operating frequency (in Hz) for each sonicator unit.
7. **FR7**: The system shall monitor and report the status for each sonicator unit, including `Overload`, `Frequency Lock`, and a `COMM_FAULT` flag if a unit becomes physically disconnected.
8. **FR8**: The system shall expose all control and monitoring capabilities through a MODBUS RTU slave interface.
9. **FR9**: The system shall report the total number of currently active sonicators and a bitmask representing their individual running states.
10. **FR10**: On power-up, all sonicator units shall default to an 'off' state and require an explicit 'start' command to begin operation.
11. **FR11**: If communication with the master PLC/HMI is lost for more than one second, all sonicator units shall automatically enter a safe, non-operational state.

## 2.2. Non-Functional Requirements

1. **NFR1**: The end-to-end latency for any command (e.g., start, stop, reset) shall not exceed 200ms.
2. **NFR2**: Any change in a sonicator's state must be reflected in its corresponding MODBUS register within 100ms.
3. **NFR3**: The firmware must be implemented in C with a modular architecture that separates hardware abstraction, communication protocols, and control logic.
4. **NFR4**: All public functions and data structures in header files must be documented using Doxygen-style comments.
5. **NFR5**: The system must implement a watchdog timer to ensure it resets to a safe, non-operational state in the event of a software failure.
6. **NFR6**: The project must achieve a minimum of 85% unit test coverage.
7. **NFR7**: All functional requirements must be verifiable through automated acceptance tests (simulation and/or Hardware-in-the-Loop).
8. **NFR8**: The MODBUS RTU communication must operate at 115200 baud with an 8N1 frame format.
9. **NFR9**: Telemetry data (power, frequency) for each unit shall be updated in the MODBUS registers at a minimum frequency of 10Hz.
10. **NFR10**: The system shall provide a non-volatile flag or dedicated register to indicate if the last system reset was caused by the watchdog timer.
11. **NFR11**: Firmware updates will be performed via a physical ISP header; no over-the-air update mechanism is required for the MVP.
