# Story: Hardware Abstraction Layer (HAL) Implementation

## Status: Ready for Development

## Story

As a **firmware developer**,
I want **a hardware abstraction layer that provides consistent interfaces to ATmega32A peripherals**,
So that **higher-level modules can interact with hardware without direct register manipulation**.

## Epic Context

### Epic 1: Foundational Control & Communication

Goal: Establish core firmware structure, basic sonicator control, and MODBUS communication layer

## Story Context

### Technology Stack

- ATmega32A microcontroller peripherals (GPIO, ADC, PWM, UART, Timers)
- Arduino Framework abstraction layer
- C/C++ implementation with embedded best practices
- Pin assignments per docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)

### Integration Points

- Direct ATmega32A peripheral registers
- Arduino Framework core functions
- Future MODBUS communication layer
- Future sonicator control modules

## Acceptance Criteria

### Functional Requirements

1. GPIO HAL provides digital input/output control for all sonicator interface pins
2. ADC HAL provides analog reading capabilities for power/feedback monitoring
3. PWM HAL provides amplitude control signal generation
4. UART HAL provides hardware interface for MODBUS communication
5. Timer HAL provides precise timing for cooperative scheduler and watchdog

### Interface Requirements

6. All HAL modules provide consistent error handling and return codes
7. HAL interfaces are hardware-agnostic and can support future microcontroller changes
8. Pin definitions centralized and configurable via header files
9. Hardware initialization follows proper power-on sequence

### Quality Requirements

10. All HAL functions documented with Doxygen-style comments
11. Unit tests achieve >90% coverage for all HAL modules
12. Hardware abstraction enables easy mocking for unit tests
13. Code follows embedded coding standards from docs/standards/

## Technical Implementation Details

### Required HAL Modules

#### GPIO HAL (`hal/gpio.h`)

```c
// GPIO control for sonicator interfaces and status indicators
void gpio_init(void);
void gpio_set_pin_mode(uint8_t pin, gpio_mode_t mode);
void gpio_write_pin(uint8_t pin, bool state);
bool gpio_read_pin(uint8_t pin);
void gpio_toggle_pin(uint8_t pin);
```

#### ADC HAL (`hal/adc.h`)

```c
// Analog-to-digital conversion for power monitoring
void adc_init(void);
uint16_t adc_read_channel(uint8_t channel);
void adc_start_conversion(uint8_t channel);
bool adc_conversion_complete(void);
float adc_to_voltage(uint16_t raw_value);
```

#### PWM HAL (`hal/pwm.h`)

```c
// PWM generation for amplitude control
void pwm_init(void);
void pwm_set_frequency(uint8_t channel, uint32_t frequency);
void pwm_set_duty_cycle(uint8_t channel, uint8_t duty_percent);
void pwm_enable_channel(uint8_t channel);
void pwm_disable_channel(uint8_t channel);
```

#### UART HAL (`hal/uart.h`)

```c
// UART communication for MODBUS RTU
void uart_init(uint32_t baud_rate);
void uart_send_byte(uint8_t data);
uint8_t uart_receive_byte(void);
bool uart_data_available(void);
void uart_send_buffer(const uint8_t* buffer, uint16_t length);
```

#### Timer HAL (`hal/timer.h`)

```c
// Timer management for scheduler and watchdog
void timer_init(void);
void timer_start(uint8_t timer_id, uint32_t period_ms);
void timer_stop(uint8_t timer_id);
bool timer_expired(uint8_t timer_id);
uint32_t timer_get_millis(void);
```

### Pin Mapping Integration

- All pin definitions referenced from central pin matrix
- Hardware-specific constants isolated in config.h
- Pin assignments validated against ATmega32A datasheet
- Status LED on PD2 (physical pin 16) per architecture requirements

### Error Handling Strategy

- Consistent return codes across all HAL modules
- Hardware fault detection and reporting
- Graceful degradation for non-critical failures
- Watchdog integration for critical fault recovery

## Definition of Done

- [ ] All HAL modules implemented and tested
- [ ] GPIO control functional for all sonicator interface pins
- [ ] ADC readings accurate for power monitoring
- [ ] PWM generation verified for amplitude control
- [ ] UART communication operational at 115200 baud
- [ ] Timer functions provide accurate timing
- [ ] Unit tests achieve >90% coverage
- [ ] Doxygen documentation complete for all public interfaces
- [ ] **HIL verification: GPIO pin control validated on actual hardware**
- [ ] **HIL verification: ADC accuracy verified with known reference voltages**
- [ ] **HIL verification: PWM frequency and duty cycle measured with test equipment**
- [ ] **HIL verification: UART communication tested with HIL framework**
- [ ] **HIL verification: Timer accuracy validated with real-time measurements**
- [ ] Hardware validation on ATmega32A target successful
- [ ] Code review completed for architecture compliance

## Dependencies

### Prerequisites

- Epic 1 Story 0: HIL Testing Framework & Hardware Verification completed
- Epic 1 Story 1: Project Structure & Build System Setup completed
- Hardware setup with ATmega32A and test equipment
- Pin matrix documentation finalized

### Enables Next Stories

- MODBUS RTU communication implementation
- Basic sonicator control interface
- Cooperative scheduler implementation

## Risk Assessment

**Primary Risk:** Hardware register configuration complexity for ATmega32A
**Mitigation:** Thorough datasheet review and incremental testing
**Verification:** Hardware-in-loop validation of all peripheral functions

**Secondary Risk:** Pin assignment conflicts or errors
**Mitigation:** Strict adherence to pin matrix documentation
**Verification:** Physical pin testing and validation

## Notes

- Focus on simplicity and testability in HAL design
- Maintain clear separation between hardware-specific and generic code
- All timing-critical operations must consider ATmega32A limitations
- HAL must support future hardware abstraction for different microcontrollers
