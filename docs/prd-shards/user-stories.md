# Epic & User Story Details

This document breaks down the high-level epics into detailed user stories with acceptance criteria, providing a clear roadmap for development and testing.

---

## Epic 1: Foundational Control & Communication

**Goal:** Establish the core firmware structure, basic sonicator control, and the MODBUS communication layer, enabling control and monitoring of a single sonicator.

### Stories for Epic 1

#### US-001 — Start/Stop a Single Sonicator

- **As an** operator, **I want** to start or stop any individual sonicator channel (S1–S4), **so that** I have independent control for process steps and safety interventions.
- **Acceptance Criteria:**
  1. When I issue a start, the unit begins operating and indicates active state.
  2. When I issue a stop, the unit ceases operation and indicates inactive state.
  3. Commands take effect promptly and are reflected in status views.

#### US-003 — Set Amplitude (Common Control)

- **As an** operator, **I want** to set the output amplitude for the system, **so that** I can achieve consistent process intensity across runs.
- **Acceptance Criteria:**
  1. I can set amplitude within the allowed range (20-100%).
  2. The current amplitude setting is visible.
  3. Out-of-range inputs are clamped or rejected per the requirements.

#### US-004 — View Frequency and Lock Status per Unit

- **As an** operator, **I want** to view the operating frequency and whether the unit is frequency-locked, **so that** I can confirm unit stability and process quality.
- **Acceptance Criteria:**
  1. Frequency is displayed per unit and updates regularly when running.
  2. Lock status is visible and accurate.
  3. If a unit is stopped, values are stable and do not mislead.

#### US-006 — MODBUS Communication for Control & Monitoring

- **As an** integrator, **I want** to control and read the system via MODBUS RTU from a PLC/HMI, **so that** I can achieve seamless integration with industrial automation.
- **Acceptance Criteria:**
  1. A documented set of holding/input registers supports all key operations and status.
  2. Read/write operations behave predictably, with clear error feedback on invalid requests.
  3. Communication performance is adequate for line operation.

#### US-010 — System Information & Versioning

- **As an** integrator, **I want** to read the firmware version and device identification, **so that** I can support traceability and fleet management.
- **Acceptance Criteria:**
  1. Version and identity information are readable via a standard interface.
  2. Values persist across power cycles and match release artifacts.

### Enablers for Epic 1

- **EN-003 — CI: Run Simulation Acceptance Tests**
- **EN-004 — Register Map Drift Guardrail**
- **EN-005 — Coverage Gate**

---

## Epic 2: Multi-Sonicator Management & Safety Features

**Goal:** Scale the system to manage all four sonicators concurrently and implement critical safety and reliability features.

### Stories for Epic 2

#### US-009 — Operate Multiple Units Concurrently

- **As an** operator, **I want** to run any subset of units at the same time without interference, **so that** I can increase throughput and flexibility.
- **Acceptance Criteria:**
  1. Starting/stopping any unit does not destabilize others.
  2. Frequency/lock and status remain accurate per unit.
  3. System performance remains acceptable with four units active.

#### US-002 — Overload Detection and Reset per Unit

- **As an** operator, **I want** to see when a unit is in overload and clear the overload when safe, **so that** I can protect equipment and ensure predictable recovery.
- **Acceptance Criteria:**
  1. Overload is clearly indicated per unit when present.
  2. Clearing/resetting restores normal operation if the condition is gone.
  3. The system prevents restart if the overload condition persists.

#### US-007 — Fault Handling and Safe Defaults

- **As an** operator, **I want** the system to fail safe on errors and recover predictably, **so that** unsafe behavior is prevented and downtime is reduced.
- **Acceptance Criteria:**
  1. On startup or error, outputs assume safe defaults.
  2. A watchdog timer is implemented to recover from software failures.
  3. After recovery, the system state is clearly communicated and controllable.

#### US-005 — System Status Indicator (Single LED)

- **As an** operator/technician, **I want** to rely on a single system status LED for high-level health, **so that** I can get a quick at-a-glance status on the line.
- **Acceptance Criteria:**
  1. The LED reflects normal, fault, and activity states per a documented scheme.
  2. The behavior is consistent across power cycles.

---

## Epic 3: HIL-Validation and Finalization

**Goal:** Validate the complete system against physical hardware and finalize documentation and testing requirements for release.

### Stories for Epic 3

#### US-008 — Calibration and Scaling

- **As a** technician, **I want** to apply calibration so reported power and control levels match reality, **so that** I can ensure accuracy and repeatability across devices.
- **Acceptance Criteria:**
  1. Calibration values can be set and are used by the system.
  2. Reported values reflect the applied calibration.
  3. The calibration procedure is documented.

### Enablers for Epic 3

- **EN-001 — Simulation Harness via pysimulavr**
- **EN-002 — HIL Serial Protocol Definition**

---

## Notes

- After PO approval, Architect will add technical acceptance to each story: register addresses, timing guarantees, tolerance bands, and HIL references.
- Behave feature files will mirror these stories under `test/acceptance/features/`.
