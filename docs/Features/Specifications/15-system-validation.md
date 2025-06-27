# Feature Prompt: System Validation

## Purpose

Perform full validation of the system under operational conditions including EMI, thermal stress, timing accuracy, and functional testing with live CT2000 sonicators. This confirms system readiness for production deployment.

---

### Implementation Steps

1. **EMI Testing**
   - Use EMI chamber or third-party testing service
   - Operate all sonicators at max amplitude during scan
   - Document interference susceptibility or emissions
   - Apply shielding or filtering mitigation if needed

2. **Thermal Stress Testing**
   - Run all four units continuously in warm environment (e.g., 45°C chamber)
   - Monitor temperature drift, failures, or resets
   - Validate component ratings on board vs observed temps

3. **Timing Accuracy**
   - Measure MODBUS frame turnaround latency (µs scale)
   - Validate sonicator start/stop delays against spec
   - Confirm watchdog and safety ISR timing under load

4. **Full System Test with Real Sonicators**
   - Connect live CT2000 sonicators via DB9
   - Run amplitude ramp-up/down profiles
   - Trigger and recover from overload events
   - Simulate frequency lock loss

5. **Acceptance Testing Checklist**
   - Review and execute full checklist:
     - MODBUS compliance
     - HMI compatibility
     - Alarm and diagnostics reporting
     - Emergency stop performance
     - Multi-unit orchestration

6. **Documentation of Results**
   - Record video of validation procedures
   - Log results in `docs/validation-report.md`
   - Include oscilloscope and thermal imagery if available

---

### Acceptance Criteria

- [ ] EMI testing passed or mitigations applied
- [ ] Thermal stress test shows no abnormal behavior
- [ ] Timing meets real-time spec under all loads
- [ ] System operates flawlessly with real sonicators
- [ ] All tests recorded and archived for traceability

---

### Dependencies

- All firmware and HIL test phases completed (phases 1–8)

---

### Estimated Duration

16 hours

---

### Notes

This is the final gate before deployment and customer installation. Any failing item must block shipment. Add summary in final deployment package.

---

### Nxt Step

Once complete, mark `15-system-validation.md` as ✅ and proceed to `16-deployment-preparation.md`
