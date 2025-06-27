# Feature Prompt: Performance Monitoring

## Purpose

Implement a performance monitoring subsystem to track CPU load, memory utilization, communication throughput, and system uptime. This helps ensure real-time constraints are met and aids optimization/debugging.

---

### Implementation Steps

1. **CPU Usage Monitoring**
   - Measure time spent in `loop()`
   - Use a hardware timer to track elapsed time vs active time
   - Calculate CPU utilization % and store as 8-bit value

2. **Memory Usage Tracking**
   - Use `__heap_start`, `__brkval` to estimate heap usage
   - Optionally check free stack by placing sentinel values
   - Create struct:

     ```cpp
     struct MemoryStats {
       uint16_t heap_used;
       uint16_t heap_free;
       uint16_t stack_free;
     };
     ```

3. **Communication Statistics**
   - Track MODBUS frames received, errors, CRC failures, and response time
   - Store stats in struct for diagnostics export

4. **Uptime and Cycle Metrics**
   - Extend DiagnosticsSystem to include:
     - `uint32_t loop_cycles`
     - `uint32_t modbus_requests`
     - `uint16_t last_response_time_ms`

5. **Display and Logging**
   - Output metrics via MODBUS diagnostics block
   - Log critical stats (e.g. 95%+ CPU) to EEPROM or Firebase (future)
   - Optionally flash status LED if any metric enters danger threshold

6. **Visualization Prep**
   - Prepare exported metrics format for future use in mobile/Firebase dashboard
   - Define JSON or key-value layout

---

### Acceptance Criteria

- [ ] CPU usage is accurately calculated and updated regularly
- [ ] Heap and stack estimates are available and accurate
- [ ] MODBUS statistics show traffic and errors
- [ ] Performance metrics are readable from MODBUS
- [ ] Visual indicator or log triggers on abnormal resource usage

---

### Dependencies

- Diagnostics system from `10-diagnostics-implementation.md`
- MODBUS map from `5-modbus-register-mapping.md`

---

### Estimated Duration

4 hours

---

### Notes

This module must not introduce overhead that skews measurements. Schedule sampling in a low-frequency timer or background task.

---

### Next Step

Once complete, mark `11-performance-monitoring.md` as ✅ and proceed to `12-firebase-integration.md`
