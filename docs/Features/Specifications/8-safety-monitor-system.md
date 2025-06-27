# Feature Prompt: Safety Monitor System

## Purpose

Implement the `SafetyMonitor` system to ensure fast (faster than MODBUS protocol requirements) detection and response to critical events including overloads and emergency stops. This module acts as the first line of defense in preventing hardware damage and ensuring operator safety.

---

### Implementation Steps

1. **Create `SafetyMonitor` Class**
   - File: `src/safety/safety_monitor.cpp`
   - Members:
     - `volatile bool emergency_stop_triggered;`
     - `bool overload_flags[4];`
   - Methods:
     - `initialize()` — sets up interrupts and safety outputs
     - `checkOverloads()` — polls overload pins if not interrupt-driven
     - `triggerEmergencyStop()` — disables all outputs
     - `resetAllSafety()` — resets all states after cooldown

2. **Interrupt-Driven Emergency Stop**
   - Configure external interrupt pin (INT0 or PCINTx)
   - ISR should:
     - Immediately disable amplitude outputs and start signals
     - Set fail-safe flags for diagnostics
     - Optionally activate a hardware latch

3. **Overload Detection**
   - Either:
     - Poll overload lines rapidly in loop
     - Or use pin change interrupts for each DB9 channel
   - Track response time using debug GPIO toggle and oscilloscope

4. **Watchdog Timer Setup**
   - Enable internal WDT with 1s timeout
   - Reset within loop or interrupt handler
   - Use WDT reset to recover from lockups

5. **Fail-Safe Output State**
   - Default all outputs LOW or safe value on trigger
   - Visual indicator: blink red LED if any safety trigger is active

6. **Safety Debounce and Filtering**
   - Add software debounce for overload triggers
   - Use cooldown timers to avoid reset spamming

---

### Acceptance Criteria

- [ ] Emergency stop is triggered faster than MODBUS protocol requirements (measured)
- [ ] All outputs disabled upon safety event
- [ ] Each overload input is monitored and logged
- [ ] Watchdog is active and resets on lockup
- [ ] Fail-safe state persists until manually cleared

---

### Dependencies

- DB9 pin mapping from `3-db9-interface-configuration.md`
- `SystemController` for disabling units (`7-multi-unit-orchestration.md`)

---

### Estimated Duration

12 hours

---

### Notes

This module must be the most reliable and prioritized system in the firmware. Interrupts must be short, deterministic, and side-effect free. Future FDA compliance will depend on reliable traceability.

---

### Next Step

Once complete, mark `8-safety-monitor-system.md` as ✅ and proceed to `9-alarm-and-event-management.md`
