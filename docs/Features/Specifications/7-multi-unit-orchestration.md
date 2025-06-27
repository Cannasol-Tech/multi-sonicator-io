# Feature Prompt: Multi-Unit Orchestration

## Purpose

Implement the `SystemController` class to coordinate simultaneous control of up to four `SonicatorInterface` instances. This module manages synchronization, broadcasting commands, and managing global control states.

---

### Implementation Steps

1. **Create `SystemController` Class**
   - File: `src/system/system_controller.cpp`
   - Members:
     - `SonicatorInterface* units[4];`
     - `uint8_t active_units;`
   - Methods:
     - `initializeAll()` — initializes all sonicators
     - `startAll()` / `stopAll()` — bulk commands
     - `setAmplitude(uint8_t unit, uint16_t value)` — per-unit control
     - `updateAll()` — runs per-loop updates
     - `getStatus(uint8_t unit)` — retrieve snapshot

2. **Synchronization Logic**
   - Implement staggered start: delay 200ms between start commands
   - Support both selective and broadcast control
   - Track global active/running state

3. **Unit Selection and Routing**
   - Use a 4-bit mask to enable or disable units
   - Create method `enableUnit(uint8_t index, bool enable)`
   - Allow dynamic runtime toggling

4. **Concurrency and Safety**
   - Ensure all actions are lock-free and cooperative
   - Use timers or watchdog to monitor update time per unit

5. **Diagnostics Support**
   - Add system-wide fault summary method
   - Log last action timestamp per unit

6. **Integration with MODBUS and Main Loop**
   - Ensure MODBUS register writes can trigger orchestration actions
   - Hook into `loop()` with `SystemController::updateAll()`

---

### Acceptance Criteria

- [ ] SystemController can manage all 4 sonicators simultaneously
- [ ] Supports synchronized or individual start/stop
- [ ] Unit enable/disable works in runtime
- [ ] Proper delays prevent power surge on bulk start
- [ ] No blocking or concurrency issues during update cycle

---

### Dependencies

- `SonicatorInterface` from `6-sonicator-control-module.md`
- MODBUS register map from `5-modbus-register-mapping.md`

---

### Estimated Duration

8 hours

---

### Notes

This module provides centralized coordination and is the primary integration point for MODBUS, safety, and diagnostics modules. Use debug logging throughout development.

---

### Next Step

Once complete, mark `7-multi-unit-orchestration.md` as ✅ and proceed to `8-safety-monitor-system.md`
