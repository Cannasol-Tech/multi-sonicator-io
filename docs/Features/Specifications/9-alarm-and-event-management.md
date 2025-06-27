# Feature Prompt: Alarm and Event Management

## Purpose

Implement the alarm detection, prioritization, logging, and acknowledgment system. This feature supports operational visibility, auditing, and integration with external HMI/PLC systems via EEPROM-backed alarm storage.

---

### Implementation Steps

1. **Create `AlarmManager` Class**
   - File: `src/diagnostics/alarm_manager.cpp`
   - Track active alarms (bitmask or enum array)
   - Assign each alarm a priority and reset behavior
   - Methods:
     - `raiseAlarm(AlarmCode)`
     - `acknowledgeAlarm(AlarmCode)`
     - `logAlarmToEEPROM(AlarmCode)`
     - `clearAlarm(AlarmCode)`
     - `getActiveAlarms()`

2. **Alarm Definitions**
   - Create `alarm_codes.h` enum:
     - `ALARM_OVERLOAD_UNIT_1` ... `UNIT_4`
     - `ALARM_FREQ_LOCK_LOSS`
     - `ALARM_EMERGENCY_STOP`
     - `ALARM_INTERNAL_FAULT`
   - Define associated priority and text labels

3. **EEPROM Logging**
   - Use circular log buffer (e.g., last 32 alarms)
   - Store timestamp (millisecond since boot)
   - Include unit, alarm code, status (RAISED/CLEARED)

4. **Alarm Acknowledgment Mechanism**
   - MODBUS register or button input triggers acknowledgment
   - Only resets alarms that are non-latching

5. **HMI Integration**
   - Expose alarm status bits via MODBUS
   - Include highest-priority active alarm in diagnostics register

6. **Testing**
   - Simulate all alarm types
   - Validate EEPROM entries via debug output
   - Confirm alarm bits respond correctly in MODBUS

---

### Acceptance Criteria

- [ ] All relevant alarm types are detected and managed
- [ ] Alarm bitmask and priority outputs are accurate
- [ ] EEPROM contains accurate time-stamped logs
- [ ] MODBUS interface shows correct alarm status
- [ ] Acknowledgment correctly resets non-latching alarms

---

### Dependencies

- Safety system from `8-safety-monitor-system.md`
- MODBUS registers from `5-modbus-register-mapping.md`

---

### Estimated Duration

6 hours

---

### Notes

This feature creates a full traceability system for debugging and compliance. Alarms may later be exported to Firebase or displayed on the HMI.

---

### Next Step

Once complete, mark `9-alarm-and-event-management.md` as ✅ and proceed to `10-diagnostics-implementation.md`
