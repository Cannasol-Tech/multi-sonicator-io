# PRD Functional Requirements Coverage (BDD Tags)

This report summarizes which PRD requirements (FR1–FR11) are tagged in Behave feature files.
Tags used: `@requirement_FRx`.

| FR | Description | Tagged In | Pending? |
|----|-------------|-----------|----------|
| FR1 | Control up to four CT2000 sonicator units concurrently via a single interface. | <none> | no |
| FR2 | Provide individual start and stop control for each connected sonicator unit. | test/acceptance/features/start_stop.feature | no |
| FR3 | Provide individual amplitude control per unit with clamping to 20–100%. | test/acceptance/features/amplitude_control.feature | no |
| FR4 | Provide individual overload reset per unit. | test/acceptance/features/overload_reset.feature | no |
| FR5 | Monitor and report real-time power (Watts) per unit. | test/acceptance/features/monitoring.feature | no |
| FR6 | Monitor and report real-time operating frequency (Hz) per unit. | test/acceptance/features/frequency_and_lock.feature | no |
| FR7 | Monitor and report status per unit (Overload, Frequency Lock, COMM_FAULT). | test/acceptance/features/frequency_and_lock.feature | no |
| FR8 | Expose all control/monitoring over a MODBUS RTU slave interface. | test/acceptance/features/modbus.feature | no |
| FR9 | Report total number of active sonicators and a bitmask of running states. | test/acceptance/features/start_stop.feature<br/>test/acceptance/features/system.feature | no |
| FR10 | On power-up, all sonicator units default to 'off' and require explicit start. | test/acceptance/features/system.feature | no |
| FR11 | On PLC/HMI comms loss >1s, all sonicators enter a safe, non-operational state. | test/acceptance/features/safety.feature | no |
