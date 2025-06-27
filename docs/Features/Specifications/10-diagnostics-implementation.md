# Feature Prompt: Diagnostics Implementation

## Purpose

Create a centralized diagnostics system to report system runtime metrics, perform self-tests, and drive visual indicators for status feedback. This system improves transparency and eases troubleshooting.

---

### Implementation Steps

1. **Create `DiagnosticsSystem` Class**
   - File: `src/diagnostics/diagnostics_system.cpp`
   - Members:
     - `uint32_t uptime_seconds`
     - `uint16_t last_error_code`
     - `uint16_t loop_cycles_per_second`
     - Diagnostic LED state
   - Methods:
     - `initialize()`
     - `updateEverySecond()`
     - `recordError(uint16_t code)`
     - `performSelfTest()`
     - `getDiagnosticsStruct()`

2. **Runtime Statistics**
   - Track:
     - Uptime (increment every second)
     - Loop cycles per second (increment counter and sample once/sec)
     - Memory usage (if possible via linker macros or `__heap_start`)

3. **Error Logging**
   - Central method to record error codes (used by safety and communication)
   - Optionally store history in ring buffer (RAM or EEPROM)

4. **Self-Test Routine**
   - On startup or via MODBUS trigger, run:
     - GPIO test (loopback or expected state)
     - EEPROM write/read
     - Sonicator interface pings
     - Communication timeout validation

5. **LED Status Indicators**
   - Use RGB or multicolor LED for system status:
     - GREEN = healthy
     - YELLOW = degraded
     - RED = fault
   - Blink patterns indicate specific fault codes

6. **MODBUS Integration**
   - Expose diagnostics fields via read-only MODBUS registers
   - Include last error code, uptime, and status bitmask

---

### Acceptance Criteria

- [ ] System uptime and loop stats tracked and accurate
- [ ] Self-test routine executes and returns pass/fail
- [ ] Diagnostic LEDs reflect real-time status
- [ ] MODBUS exposes diagnostics data clearly
- [ ] All modules can report errors to diagnostics layer

---

### Dependencies

- Alarm Manager from `9-alarm-and-event-management.md`
- MODBUS from `5-modbus-register-mapping.md`

---

### Estimated Duration

8 hours

---

### Notes

This module should be lightweight and always-running. Avoid adding blocking behavior. Add debug logging for all failures to aid field diagnostics.

---

### Next Step

Once complete, mark `10-diagnostics-implementation.md` as ✅ and proceed to `11-performance-monitoring.md`
