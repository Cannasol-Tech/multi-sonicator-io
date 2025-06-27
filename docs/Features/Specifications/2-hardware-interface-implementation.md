# Feature Prompt: Hardware Interface Implementation

## Purpose

Implement core low-level hardware interfaces for the Multi Sonicator I/O Controller, including GPIO control, analog I/O (ADC/DAC), and signal isolation to safely interact with CT2000 sonicators.

---

### Implementation Steps

1. **GPIO Abstraction Layer**
   - Create `GpioManager` class in `src/io/gpio_manager.cpp`
   - Support pinMode, digitalRead, digitalWrite with semantic aliasing (e.g., `OVERLOAD_INPUT1`, `AMPLITUDE_OUT2`)

2. **Digital Input/Output Configuration**
   - Define constants for digital input/output mapping
   - Enable pull-up resistors for open-drain inputs
   - Use atomic writes for output control to avoid glitches

3. **ADC Configuration**
   - Use AVCC as reference, configure prescaler for 16MHz
   - Sample analog values from frequency/power sense pins
   - Create polling and interrupt sampling modes

4. **PWM Amplitude Control**
   - Configure Timer1 and Timer2 for PWM generation on pins PB3, PD5-7
   - Create RC filters for each PWM output (1kΩ + 10µF)
   - Use op-amp circuits for 0–10V amplitude scaling

5. **Hardware Abstraction Layer (HAL)**
   - Create a `HardwareInterface` module that abstracts all above layers
   - Implement `initialize()`, `readInputs()`, `writeOutputs()` methods
   - Use `struct` or `class` to pass all hardware state cleanly

6. **Isolation Testing**
   - Simulate digital inputs and measure isolation effectiveness
   - Validate output transitions do not couple into analog inputs

---

### Acceptance Criteria

- [ ] GPIOManager supports all digital I/O signals per schematic
- [ ] ADC values are stable and properly scaled
- [ ] DAC interface allows 0–10V output to external circuit
- [ ] Isolation between channels validated with oscilloscope
- [ ] HAL provides clear and modular interface to control logic

---

### Dependencies

- Completion of physical hardware and schematic (1.2)
- Project setup from `1-initial-setup.md`

---

### Estimated Duration

8 hours

---

### Notes

Design must be interrupt-safe and deterministic. Avoid long blocking delays or polling in real-time interfaces. Prepare for future use in safety-related interrupt handlers.

---

### Next Step

Once complete, mark `2-hardware-interface-implementation.md` as ✅ and proceed to `3-db9-interface-configuration.md`
