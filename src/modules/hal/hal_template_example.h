/**
 * @file hal_template_example.h
 * @brief Template-Based HAL Example for Zero-Overhead Polymorphism
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.0.0
 *
 * This example demonstrates template-based HAL design that provides:
 * - Zero runtime overhead (no virtual methods)
 * - Compile-time polymorphism
 * - Type safety
 * - Testability through template parameters
 * - Perfect for embedded systems with limited resources
 */

#ifndef HAL_TEMPLATE_EXAMPLE_H
#define HAL_TEMPLATE_EXAMPLE_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// TEMPLATE-BASED HAL DESIGN
// ============================================================================

/**
 * @brief GPIO HAL Interface (Template-based)
 *
 * Template parameter allows different implementations at compile time
 * with zero runtime overhead.
 */
template<typename HalImpl>
class GpioHal {
public:
    // Core GPIO operations - direct calls to implementation
    bool init() { return HalImpl::gpioInit(); }
    bool setPinMode(uint8_t pin, uint8_t mode) { return HalImpl::gpioSetPinMode(pin, mode); }
    bool digitalWrite(uint8_t pin, uint8_t value) { return HalImpl::gpioDigitalWrite(pin, value); }
    uint8_t digitalRead(uint8_t pin) { return HalImpl::gpioDigitalRead(pin); }
    bool togglePin(uint8_t pin) { return HalImpl::gpioTogglePin(pin); }

    // Sonicator-specific operations
    bool sonicatorStart(uint8_t id) { return HalImpl::gpioSonicatorStart(id); }
    bool sonicatorStop(uint8_t id) { return HalImpl::gpioSonicatorStop(id); }
    bool sonicatorReset(uint8_t id) { return HalImpl::gpioSonicatorReset(id); }
    bool sonicatorReadOverload(uint8_t id, bool* state) { return HalImpl::gpioSonicatorReadOverload(id, state); }
    bool sonicatorReadFreqLock(uint8_t id, bool* state) { return HalImpl::gpioSonicatorReadFreqLock(id, state); }
    bool statusLed(bool state) { return HalImpl::gpioStatusLed(state); }
};

/**
 * @brief ADC HAL Interface (Template-based)
 */
template<typename HalImpl>
class AdcHal {
public:
    bool init() { return HalImpl::adcInit(); }
    bool readChannel(uint8_t channel, uint16_t* value) { return HalImpl::adcReadChannel(channel, value); }
    float rawToVoltage(uint16_t raw) { return HalImpl::adcRawToVoltage(raw); }
    bool readSonicatorPower(uint8_t id, float* power) { return HalImpl::adcReadSonicatorPower(id, power); }
};

/**
 * @brief PWM HAL Interface (Template-based)
 */
template<typename HalImpl>
class PwmHal {
public:
    bool init() { return HalImpl::pwmInit(); }
    bool setDutyCycle(uint8_t channel, uint8_t duty) { return HalImpl::pwmSetDutyCycle(channel, duty); }
    bool setAmplitude(uint8_t amplitude) { return HalImpl::pwmSetAmplitude(amplitude); }
    bool emergencyStop() { return HalImpl::pwmEmergencyStop(); }
};

/**
 * @brief Complete HAL Template (combines all peripherals)
 */
template<typename HalImpl>
class HalTemplate : public GpioHal<HalImpl>,
                   public AdcHal<HalImpl>,
                   public PwmHal<HalImpl> {
public:
    // Unified interface
    bool begin() {
        bool gpio_ok = GpioHal<HalImpl>::init();
        bool adc_ok = AdcHal<HalImpl>::init();
        bool pwm_ok = PwmHal<HalImpl>::init();
        return gpio_ok && adc_ok && pwm_ok;
    }
};

// ============================================================================
// CONCRETE IMPLEMENTATION EXAMPLE (ATmega32A)
// ============================================================================

/**
 * @brief ATmega32A HAL Implementation
 *
 * Contains static methods that implement the HAL interface.
 * No virtual methods, no objects - just direct function calls.
 */
class AtmegaHal {
public:
    // GPIO Implementation
    static bool gpioInit() { return gpio_init() == GPIO_OK; }
    static bool gpioSetPinMode(uint8_t pin, uint8_t mode) {
        gpio_mode_t gpio_mode;
        switch (mode) {
            case 0: gpio_mode = GPIO_MODE_INPUT; break;
            case 1: gpio_mode = GPIO_MODE_OUTPUT; break;
            case 2: gpio_mode = GPIO_MODE_INPUT_PULLUP; break;
            default: return false;
        }
        return gpio_set_pin_mode(pin, gpio_mode) == GPIO_OK;
    }
    static bool gpioDigitalWrite(uint8_t pin, uint8_t value) {
        gpio_state_t state = (value == 0) ? GPIO_LOW : GPIO_HIGH;
        return gpio_write_pin(pin, state) == GPIO_OK;
    }
    static uint8_t gpioDigitalRead(uint8_t pin) {
        gpio_state_t state;
        gpio_read_pin(pin, &state);
        return (state == GPIO_HIGH) ? 1 : 0;
    }
    static bool gpioTogglePin(uint8_t pin) {
        return gpio_toggle_pin(pin) == GPIO_OK;
    }
    static bool gpioSonicatorStart(uint8_t id) {
        return gpio_sonicator_start(id) == GPIO_OK;
    }
    static bool gpioSonicatorStop(uint8_t id) {
        return gpio_sonicator_stop(id) == GPIO_OK;
    }
    static bool gpioSonicatorReset(uint8_t id) {
        return gpio_sonicator_reset(id) == GPIO_OK;
    }
    static bool gpioSonicatorReadOverload(uint8_t id, bool* state) {
        return gpio_sonicator_read_overload(id, state) == GPIO_OK;
    }
    static bool gpioSonicatorReadFreqLock(uint8_t id, bool* state) {
        return gpio_sonicator_read_freq_lock(id, state) == GPIO_OK;
    }
    static bool gpioStatusLed(bool state) {
        gpio_state_t gpio_state = state ? GPIO_HIGH : GPIO_LOW;
        return gpio_status_led(gpio_state) == GPIO_OK;
    }

    // ADC Implementation
    static bool adcInit() { return adc_init() == ADC_OK; }
    static bool adcReadChannel(uint8_t channel, uint16_t* value) {
        return adc_read_channel((adc_channel_t)channel, value) == ADC_OK;
    }
    static float adcRawToVoltage(uint16_t raw) {
        float voltage;
        adc_to_voltage(raw, &voltage);
        return voltage;
    }
    static bool adcReadSonicatorPower(uint8_t id, float* power) {
        return adc_read_sonicator_power(id, power) == ADC_OK;
    }

    // PWM Implementation
    static bool pwmInit() { return pwm_init() == PWM_OK; }
    static bool pwmSetDutyCycle(uint8_t channel, uint8_t duty) {
        return pwm_set_duty_cycle((pwm_channel_t)channel, duty) == PWM_OK;
    }
    static bool pwmSetAmplitude(uint8_t amplitude) {
        return pwm_set_amplitude(amplitude) == PWM_OK;
    }
    static bool pwmEmergencyStop() {
        return pwm_emergency_stop() == PWM_OK;
    }
};

// ============================================================================
// MOCK IMPLEMENTATION EXAMPLE (for testing)
// ============================================================================

/**
 * @brief Mock HAL Implementation for Unit Testing
 */
class MockHal {
private:
    static bool mock_gpio_initialized;
    static bool mock_adc_initialized;
    static bool mock_pwm_initialized;
    static uint8_t mock_pin_states[32]; // Mock pin states

public:
    // GPIO Mock Implementation
    static bool gpioInit() { mock_gpio_initialized = true; return true; }
    static bool gpioSetPinMode(uint8_t pin, uint8_t mode) {
        if (!mock_gpio_initialized) return false;
        // Mock implementation - just record the mode
        return true;
    }
    static bool gpioDigitalWrite(uint8_t pin, uint8_t value) {
        if (!mock_gpio_initialized) return false;
        mock_pin_states[pin] = value;
        return true;
    }
    static uint8_t gpioDigitalRead(uint8_t pin) {
        if (!mock_gpio_initialized) return 0;
        return mock_pin_states[pin];
    }
    static bool gpioTogglePin(uint8_t pin) {
        if (!mock_gpio_initialized) return false;
        mock_pin_states[pin] = !mock_pin_states[pin];
        return true;
    }

    // Mock implementations for other functions (simplified)
    static bool gpioSonicatorStart(uint8_t id) { return mock_gpio_initialized; }
    static bool gpioSonicatorStop(uint8_t id) { return mock_gpio_initialized; }
    static bool gpioSonicatorReset(uint8_t id) { return mock_gpio_initialized; }
    static bool gpioSonicatorReadOverload(uint8_t id, bool* state) {
        if (!mock_gpio_initialized) return false;
        *state = false; // Mock: no overload
        return true;
    }
    static bool gpioSonicatorReadFreqLock(uint8_t id, bool* state) {
        if (!mock_gpio_initialized) return false;
        *state = true; // Mock: frequency locked
        return true;
    }
    static bool gpioStatusLed(bool state) { return mock_gpio_initialized; }

    // ADC Mock Implementation
    static bool adcInit() { mock_adc_initialized = true; return true; }
    static bool adcReadChannel(uint8_t channel, uint16_t* value) {
        if (!mock_adc_initialized) return false;
        *value = 512; // Mock: mid-scale reading
        return true;
    }
    static float adcRawToVoltage(uint16_t raw) { return (raw * 5.0f) / 1023.0f; }
    static bool adcReadSonicatorPower(uint8_t id, float* power) {
        if (!mock_adc_initialized) return false;
        *power = 500.0f; // Mock: 500W
        return true;
    }

    // PWM Mock Implementation
    static bool pwmInit() { mock_pwm_initialized = true; return true; }
    static bool pwmSetDutyCycle(uint8_t channel, uint8_t duty) { return mock_pwm_initialized; }
    static bool pwmSetAmplitude(uint8_t amplitude) { return mock_pwm_initialized; }
    static bool pwmEmergencyStop() { return mock_pwm_initialized; }
};

// Static member initialization
bool MockHal::mock_gpio_initialized = false;
bool MockHal::mock_adc_initialized = false;
bool MockHal::mock_pwm_initialized = false;
uint8_t MockHal::mock_pin_states[32] = {0};

// ============================================================================
// USAGE EXAMPLES
// ============================================================================

/**
 * @brief Example: Production Hardware Usage
 */
void productionExample() {
    // Use ATmega32A HAL implementation
    using MyHal = HalTemplate<AtmegaHal>;
    MyHal hal;

    // Initialize all peripherals
    hal.begin();

    // Use GPIO
    hal.digitalWrite(13, 1);  // Direct call, zero overhead
    uint8_t state = hal.digitalRead(12);

    // Use ADC
    uint16_t adc_value;
    hal.readChannel(0, &adc_value);
    float voltage = hal.rawToVoltage(adc_value);

    // Use PWM
    hal.setAmplitude(75);
}

/**
 * @brief Example: Unit Testing with Mock
 */
void testingExample() {
    // Use Mock HAL implementation for testing
    using TestHal = HalTemplate<MockHal>;
    TestHal hal;

    // Initialize mock peripherals
    hal.begin();

    // Test GPIO operations
    hal.digitalWrite(13, 1);
    uint8_t state = hal.digitalRead(13);
    assert(state == 1);  // Mock returns what we wrote

    // Test ADC operations
    uint16_t adc_value;
    hal.readChannel(0, &adc_value);
    assert(adc_value == 512);  // Mock returns known value
}

/**
 * @brief Example: Individual HAL Components
 */
void componentExample() {
    // Use just GPIO
    GpioHal<AtmegaHal> gpio;
    gpio.init();
    gpio.digitalWrite(13, 1);

    // Use just ADC
    AdcHal<MockHal> adc;
    adc.init();
    uint16_t value;
    adc.readChannel(0, &value);
}

#endif // HAL_TEMPLATE_EXAMPLE_H
