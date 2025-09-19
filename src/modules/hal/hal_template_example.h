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
 * 
 * @tparam HalImpl The HAL implementation class that provides static methods
 */
template<typename HalImpl>
class GpioHal {
public:
    /**
     * @brief Initialize GPIO subsystem
     * @return true if initialization successful, false otherwise
     */
    bool init() { return HalImpl::gpioInit(); }
    
    /**
     * @brief Set pin mode (input/output/pullup)
     * @param pin Pin number to configure
     * @param mode Pin mode (0=input, 1=output, 2=input_pullup)
     * @return true if successful, false otherwise
     */
    bool setPinMode(uint8_t pin, uint8_t mode) { return HalImpl::gpioSetPinMode(pin, mode); }
    
    /**
     * @brief Write digital value to pin
     * @param pin Pin number to write to
     * @param value Digital value (0=LOW, 1=HIGH)
     * @return true if successful, false otherwise
     */
    bool digitalWrite(uint8_t pin, uint8_t value) { return HalImpl::gpioDigitalWrite(pin, value); }
    
    /**
     * @brief Read digital value from pin
     * @param pin Pin number to read from
     * @return Digital value (0=LOW, 1=HIGH)
     */
    uint8_t digitalRead(uint8_t pin) { return HalImpl::gpioDigitalRead(pin); }
    
    /**
     * @brief Toggle pin state
     * @param pin Pin number to toggle
     * @return true if successful, false otherwise
     */
    bool togglePin(uint8_t pin) { return HalImpl::gpioTogglePin(pin); }

    /**
     * @brief Start sonicator operation
     * @param id Sonicator ID (0-based)
     * @return true if successful, false otherwise
     */
    bool sonicatorStart(uint8_t id) { return HalImpl::gpioSonicatorStart(id); }
    
    /**
     * @brief Stop sonicator operation
     * @param id Sonicator ID (0-based)
     * @return true if successful, false otherwise
     */
    bool sonicatorStop(uint8_t id) { return HalImpl::gpioSonicatorStop(id); }
    
    /**
     * @brief Reset sonicator
     * @param id Sonicator ID (0-based)
     * @return true if successful, false otherwise
     */
    bool sonicatorReset(uint8_t id) { return HalImpl::gpioSonicatorReset(id); }
    
    /**
     * @brief Read sonicator overload status
     * @param id Sonicator ID (0-based)
     * @param state Pointer to store overload state
     * @return true if successful, false otherwise
     */
    bool sonicatorReadOverload(uint8_t id, bool* state) { return HalImpl::gpioSonicatorReadOverload(id, state); }
    
    /**
     * @brief Read sonicator frequency lock status
     * @param id Sonicator ID (0-based)
     * @param state Pointer to store frequency lock state
     * @return true if successful, false otherwise
     */
    bool sonicatorReadFreqLock(uint8_t id, bool* state) { return HalImpl::gpioSonicatorReadFreqLock(id, state); }
    
    /**
     * @brief Control status LED
     * @param state LED state (true=ON, false=OFF)
     * @return true if successful, false otherwise
     */
    bool statusLed(bool state) { return HalImpl::gpioStatusLed(state); }
};

/**
 * @brief ADC HAL Interface (Template-based)
 * 
 * Provides analog-to-digital conversion functionality with compile-time
 * implementation selection.
 * 
 * @tparam HalImpl The HAL implementation class that provides static methods
 */
template<typename HalImpl>
class AdcHal {
public:
    /**
     * @brief Initialize ADC subsystem
     * @return true if initialization successful, false otherwise
     */
    bool init() { return HalImpl::adcInit(); }
    
    /**
     * @brief Read raw ADC value from channel
     * @param channel ADC channel number
     * @param value Pointer to store ADC reading
     * @return true if successful, false otherwise
     */
    bool readChannel(uint8_t channel, uint16_t* value) { return HalImpl::adcReadChannel(channel, value); }
    
    /**
     * @brief Convert raw ADC value to voltage
     * @param raw Raw ADC value (0-1023 for 10-bit ADC)
     * @return Voltage in volts
     */
    float rawToVoltage(uint16_t raw) { return HalImpl::adcRawToVoltage(raw); }
    
    /**
     * @brief Read sonicator power consumption
     * @param id Sonicator ID (0-based)
     * @param power Pointer to store power reading in watts
     * @return true if successful, false otherwise
     */
    bool readSonicatorPower(uint8_t id, float* power) { return HalImpl::adcReadSonicatorPower(id, power); }
};

/**
 * @brief PWM HAL Interface (Template-based)
 * 
 * Provides pulse-width modulation functionality for amplitude control
 * and other PWM-based operations.
 * 
 * @tparam HalImpl The HAL implementation class that provides static methods
 */
template<typename HalImpl>
class PwmHal {
public:
    /**
     * @brief Initialize PWM subsystem
     * @return true if initialization successful, false otherwise
     */
    bool init() { return HalImpl::pwmInit(); }
    
    /**
     * @brief Set PWM duty cycle for channel
     * @param channel PWM channel number
     * @param duty Duty cycle percentage (0-100)
     * @return true if successful, false otherwise
     */
    bool setDutyCycle(uint8_t channel, uint8_t duty) { return HalImpl::pwmSetDutyCycle(channel, duty); }
    
    /**
     * @brief Set output amplitude
     * @param amplitude Amplitude percentage (0-100)
     * @return true if successful, false otherwise
     */
    bool setAmplitude(uint8_t amplitude) { return HalImpl::pwmSetAmplitude(amplitude); }
    
    /**
     * @brief Emergency stop - immediately disable all PWM outputs
     * @return true if successful, false otherwise
     */
    bool emergencyStop() { return HalImpl::pwmEmergencyStop(); }
};

/**
 * @brief Complete HAL Template (combines all peripherals)
 * 
 * Unified interface that combines GPIO, ADC, and PWM functionality
 * into a single template class. Provides a convenient way to initialize
 * all peripherals at once.
 * 
 * @tparam HalImpl The HAL implementation class that provides static methods
 */
template<typename HalImpl>
class HalTemplate : public GpioHal<HalImpl>,
                   public AdcHal<HalImpl>,
                   public PwmHal<HalImpl> {
public:
    /**
     * @brief Initialize all HAL subsystems
     * 
     * Calls init() on GPIO, ADC, and PWM subsystems.
     * All must succeed for this method to return true.
     * 
     * @return true if all subsystems initialized successfully, false otherwise
     */
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
 * 
 * This implementation assumes the existence of lower-level GPIO, ADC,
 * and PWM driver functions that return status codes.
 */
class AtmegaHal {
public:
    // ========================================================================
    // GPIO Implementation
    // ========================================================================
    
    /**
     * @brief Initialize GPIO hardware
     * @return true if GPIO initialization successful
     */
    static bool gpioInit() { return gpio_init() == GPIO_OK; }
    
    /**
     * @brief Configure pin mode
     * @param pin Pin number
     * @param mode Pin mode (0=input, 1=output, 2=input_pullup)
     * @return true if successful
     */
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
    
    /**
     * @brief Write digital value to pin
     * @param pin Pin number
     * @param value Digital value (0 or 1)
     * @return true if successful
     */
    static bool gpioDigitalWrite(uint8_t pin, uint8_t value) {
        gpio_state_t state = (value == 0) ? GPIO_LOW : GPIO_HIGH;
        return gpio_write_pin(pin, state) == GPIO_OK;
    }
    
    /**
     * @brief Read digital value from pin
     * @param pin Pin number
     * @return Digital value (0 or 1)
     */
    static uint8_t gpioDigitalRead(uint8_t pin) {
        gpio_state_t state;
        gpio_read_pin(pin, &state);
        return (state == GPIO_HIGH) ? 1 : 0;
    }
    
    /**
     * @brief Toggle pin state
     * @param pin Pin number
     * @return true if successful
     */
    static bool gpioTogglePin(uint8_t pin) {
        return gpio_toggle_pin(pin) == GPIO_OK;
    }
    
    /**
     * @brief Start sonicator via GPIO control
     * @param id Sonicator ID
     * @return true if successful
     */
    static bool gpioSonicatorStart(uint8_t id) {
        return gpio_sonicator_start(id) == GPIO_OK;
    }
    
    /**
     * @brief Stop sonicator via GPIO control
     * @param id Sonicator ID
     * @return true if successful
     */
    static bool gpioSonicatorStop(uint8_t id) {
        return gpio_sonicator_stop(id) == GPIO_OK;
    }
    
    /**
     * @brief Reset sonicator via GPIO control
     * @param id Sonicator ID
     * @return true if successful
     */
    static bool gpioSonicatorReset(uint8_t id) {
        return gpio_sonicator_reset(id) == GPIO_OK;
    }
    
    /**
     * @brief Read sonicator overload status
     * @param id Sonicator ID
     * @param state Pointer to store overload status
     * @return true if successful
     */
    static bool gpioSonicatorReadOverload(uint8_t id, bool* state) {
        return gpio_sonicator_read_overload(id, state) == GPIO_OK;
    }
    
    /**
     * @brief Read sonicator frequency lock status
     * @param id Sonicator ID
     * @param state Pointer to store frequency lock status
     * @return true if successful
     */
    static bool gpioSonicatorReadFreqLock(uint8_t id, bool* state) {
        return gpio_sonicator_read_freq_lock(id, state) == GPIO_OK;
    }
    
    /**
     * @brief Control status LED
     * @param state LED state (true=ON, false=OFF)
     * @return true if successful
     */
    static bool gpioStatusLed(bool state) {
        gpio_state_t gpio_state = state ? GPIO_HIGH : GPIO_LOW;
        return gpio_status_led(gpio_state) == GPIO_OK;
    }

    // ========================================================================
    // ADC Implementation
    // ========================================================================
    
    /**
     * @brief Initialize ADC hardware
     * @return true if ADC initialization successful
     */
    static bool adcInit() { return adc_init() == ADC_OK; }
    
    /**
     * @brief Read ADC channel
     * @param channel ADC channel number
     * @param value Pointer to store ADC reading
     * @return true if successful
     */
    static bool adcReadChannel(uint8_t channel, uint16_t* value) {
        return adc_read_channel((adc_channel_t)channel, value) == ADC_OK;
    }
    
    /**
     * @brief Convert raw ADC value to voltage
     * @param raw Raw ADC value
     * @return Voltage in volts
     */
    static float adcRawToVoltage(uint16_t raw) {
        float voltage;
        adc_to_voltage(raw, &voltage);
        return voltage;
    }
    
    /**
     * @brief Read sonicator power consumption
     * @param id Sonicator ID
     * @param power Pointer to store power reading
     * @return true if successful
     */
    static bool adcReadSonicatorPower(uint8_t id, float* power) {
        return adc_read_sonicator_power(id, power) == ADC_OK;
    }

    // ========================================================================
    // PWM Implementation
    // ========================================================================
    
    /**
     * @brief Initialize PWM hardware
     * @return true if PWM initialization successful
     */
    static bool pwmInit() { return pwm_init() == PWM_OK; }
    
    /**
     * @brief Set PWM duty cycle
     * @param channel PWM channel
     * @param duty Duty cycle percentage
     * @return true if successful
     */
    static bool pwmSetDutyCycle(uint8_t channel, uint8_t duty) {
        return pwm_set_duty_cycle((pwm_channel_t)channel, duty) == PWM_OK;
    }
    
    /**
     * @brief Set output amplitude
     * @param amplitude Amplitude percentage
     * @return true if successful
     */
    static bool pwmSetAmplitude(uint8_t amplitude) {
        return pwm_set_amplitude(amplitude) == PWM_OK;
    }
    
    /**
     * @brief Emergency stop all PWM outputs
     * @return true if successful
     */
    static bool pwmEmergencyStop() {
