# Feature Prompt: Testing Framework

## Purpose

Establish a full testing strategy for the Multi-Sonicator I/O Controller firmware, including unit tests, hardware-in-loop (HIL) tests, integration validation, and regression testing.

---

### Implementation Steps

1. **Unit Test Suite**
   - Use Unity (or custom) test harness
   - Create `test/unit/*` directory for logic tests
   - Write test cases for:
     - `SonicatorInterface`
     - `SafetyMonitor`
     - `AlarmManager`
     - `MODBUS CRC & frame parsing`

2. **HIL Test Setup**
   - Use signal generators and input fixtures to simulate DB9 input states
   - Use relay/motor testers to simulate output signal paths
   - Connect logic analyzer and oscilloscope for timing validation
   - Automate with scripts and USB-controlled relays where possible

3. **Integration Testing**
   - Validate sequences from HMI → PLC → MODBUS → controller
   - Simulate overloads, frequency drops, and control toggles
   - Capture expected MODBUS responses and validate correctness

4. **Regression Suite**
   - Archive successful test results from each release
   - Automate comparison of new test runs to historical baseline
   - Flag timing or functional regressions automatically

5. **Test Documentation**
   - Create `docs/test-cases.md`
   - Include all test scenarios, expected outcomes, and coverage checklist
   - Record edge case handling: loss of power, invalid MODBUS frames, rapid overloads

6. **Automation Goals**
   - Use Git pre-push hook or CI system (optional) to run local unit tests
   - Future: automate full test bench cycle using test runner on ESP32 or host PC

---

### Acceptance Criteria

- [ ] All core modules have unit tests with >80% logic coverage
- [ ] HIL tests validate input/output timing and signal mapping
- [ ] Integration tests confirm functional control via MODBUS
- [ ] Regression test logs available and comparable
- [ ] Test case documentation is complete and current

---

### Dependencies

- Full firmware implementation (phases 1–7)
- Diagnostic and alarm systems (`10`, `9`)

---

### Estimated Duration

12 hours

---

### Notes

Testing is critical for safety compliance and long-term support. Use version-tagged test logs for traceability and debugging.

---

### Next Step

Once complete, mark `14-testing-framework.md` as ✅ and proceed to `15-system-validation.md`
