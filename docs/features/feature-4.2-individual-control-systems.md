---
title: Feature 4.2 — Individual Control Systems
description: Independent start/stop and amplitude control per sonicator unit with <100ms response time and safety limits
owner: Technical Lead
status: Draft
created: 2025-09-12
updated: 2025-09-12
links:
  epic: docs/epics/epic-4-multi-sonicator-coordination.md
  story: docs/stories/4.2.individual-control-systems.md
  pin_matrix: docs/planning/pin-matrix.md
  register_map: include/register_map.h
---

# Feature 4.2 — Individual Control Systems

## Summary
Provide independent control for each sonicator unit: start/stop and amplitude setpoint (0–100%), with validation, safety limits, and <100ms end-to-end response per operation.

Note: Current program scope prioritizes S4-only simulation (UNO R4 wrapper mapping: D7, D8, A2, A3, A4, A1, D9 PWM). Implement Unit 4 fully and scaffold Units 1–3 with clear TODOs to enable later expansion.

## Functional Requirements
- FR-4.2-1: Independent start/stop control per unit (U1–U4) via MODBUS registers
- FR-4.2-2: Amplitude setpoint per unit, 0–100% (0.1% resolution internal)
- FR-4.2-3: Parameter validation and safety bounds per unit
- FR-4.2-4: Real-time status feedback (running, setpoint, faults) per unit
- FR-4.2-5: Response time <100ms from MODBUS write to control action

## Constraints
- ATmega32A resource limits (flash/SRAM); prefer lightweight classes and fixed-size buffers
- Single source of truth for pin mapping: `docs/planning/pin-matrix.md` and `config/hardware-config.yaml`
- Initial scope: S4-only wired mapping; others N/A until hardware available

## Register Map Sketch (per unit)
- CONTROL_START (bit) — write 1 to start
- CONTROL_STOP (bit) — write 1 to stop
- AMPLITUDE_SETPOINT (uint16; 0–1000 => 0.0–100.0%)
- STATUS_FLAGS (bitfield) — RUNNING, FAULT, LIMIT_ACTIVE
- TELEMETRY_AMPLITUDE (uint16; echo/filtered)

Actual addresses to be defined/confirmed in `include/register_map.h`.

## Class Design (OOP)
```cpp
// src/modules/control/SonicatorUnit.h
class SonicatorUnit {
public:
  explicit SonicatorUnit(uint8_t id);
  void setAmplitudePercent(uint16_t permille); // 0..1000
  void start();
  void stop();
  bool isRunning() const;
  uint16_t getAmplitudePermille() const;
  void tick(); // periodic tasks

private:
  uint8_t unitId_;
  uint16_t setpointPermille_;
  bool running_;
  void applySetpoint_();
  void enforceSafety_();
};
```

- Use SOLID: keep validation in `ParameterManager`, hardware ops in `SonicatorUnitHAL`, MODBUS glue in `UnitModbusInterface`.

## Safety & Validation
- Bounds: amplitude 0–100% (permille 0–1000)
- Rate limiting (optional): limit slew per tick
- Emergency stop integration hook

## Test Strategy (TDD)
- Unit tests: `test/unit/control/test_sonicator_unit.cpp`
  - set/get amplitude, start/stop, validation
- Integration tests: `test/integration/test_unit_modbus_interface.py`
  - write registers → verify HAL calls, status echo
- HIL tests (S4): `test/hil/test_unit4_individual_control.py`
  - start/stop, amplitude set, timing measurement <100ms

## Acceptance Criteria Traceability
- AC1↔start/stop registers; AC2↔setpoint write + echo; AC3↔validation tests; AC4↔status flags; AC5↔timing tests

## File Locations
- Control: `src/modules/control/SonicatorUnit.*`
- HAL: `src/modules/hal/SonicatorUnitHAL.*`
- MODBUS: `src/modules/communication/UnitModbusInterface.*`
- Config: `config/hardware-config.yaml`

## Open Questions
- Confirm final register addresses for per-unit map
- Decide amplitude slew behavior and default rate
- Confirm emergency stop handling per-unit vs global
