# Feature Prompt: Sonicator Control Module

## Purpose

Create the core firmware module that interfaces with individual CT2000 sonicators, providing amplitude control, power/frequency monitoring, and overload reset capabilities. This logic will be instantiated four times for multi-unit support.

---

### Implementation Steps

1. **Create `SonicatorInterface` Class**
   - File: `src/sonicator/sonicator_interface.cpp`
   - Parameters: DB9 pin mapping, unit index
   - Methods:
     - `initialize()` — configures GPIO/ADC/DAC
     - `update()` — refreshes inputs/outputs
     - `start()`, `stop()`, `resetOverload()` — control functions
     - `getStatus()` — returns structured status

2. **Amplitude Control**
   - Accept command (0–1000 scale) and output via DAC
   - Clip to 0–10V max
   - Track last setpoint

3. **Input Monitoring**
   - Frequency Lock: digital input
   - Overload: digital input
   - Frequency Output: ADC input (scaled Hz)
   - Power Output: ADC input (scaled Watts)

4. **Status Struct Definition**
   - Define `SonicatorStatus` struct:

     ```cpp
     struct SonicatorStatus {
       bool running;
       bool overload;
       bool freq_lock;
       uint16_t frequency;
       uint16_t power;
     };
     ```

   - Used by diagnostics and MODBUS layers

5. **Overload Reset Logic**
   - Output pulse: set pin HIGH for 500ms, then LOW
   - Add optional cooldown timer (prevent spam resets)

6. **Safety Checks**
   - Prevent amplitude updates while in overload
   - Debounce digital inputs to avoid false triggers

7. **Unit Testing and Simulation**
   - Create unit tests for `SonicatorInterface`
   - Simulate input pin changes and validate outputs

---

### Acceptance Criteria

- [ ] Four independent instances can be created for each DB9 port
- [ ] Amplitude control properly drives DAC output
- [ ] Frequency and power readings are accurate and scaled
- [ ] Overload detection and reset work reliably
- [ ] Status struct provides accurate snapshot for diagnostics

---

### Dependencies

- Hardware interface layer (`2-hardware-interface-implementation.md`)
- DB9 signal configuration (`3-db9-interface-configuration.md`)
- MODBUS register definitions (`5-modbus-register-mapping.md`)

---

### Estimated Duration

10 hours

---

### Notes

This module is the primary abstraction layer for a single sonicator. It should be interrupt-safe, portable, and optimized for real-time polling or timer-based updates. Integration with `SystemController` comes next.

---

### Next Step

Once complete, mark `6-sonicator-control-module.md` as ✅ and proceed to `7-multi-unit-orchestration.md`
