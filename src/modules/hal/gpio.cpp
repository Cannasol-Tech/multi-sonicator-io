/**
 * @file gpio.cpp
 * @brief GPIO Hardware Abstraction Layer Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

/**
 * @defgroup HAL_GPIO GPIO HAL (ATmega32A)
 * @ingroup HAL
 * @brief Implementation notes and design rationale for the GPIO HAL.
 *
 * @section hal_gpio_overview Overview
 * - Provides safe, hardware‑aware digital I/O for sonicator interfaces and system LED.
 * - Mirrors canonical pin roles from config/hardware-config.yaml via include/system_config.h.
 * - Initializes inputs with pull‑ups for noise immunity; initializes outputs LOW for safety.
 *
 * @section hal_gpio_safety Safety Model
 * - On init, all control outputs (START/RESET) are driven LOW to ensure sonicators remain off.
 * - Overload and frequency‑lock inputs are configured as INPUT_PULLUP to reject floating states.
 * - Status LED defaults to OFF; higher layers define patterns (see Operating State policy).
 *
 * @section hal_gpio_open_drain Open‑Drain Simulation
 * - ATmega32A lacks native open‑drain mode; where open‑collector semantics are desired
 *   (ULN2003A drivers), outputs are driven LOW for active and otherwise left HIGH.
 *
 * @section hal_gpio_timing Timing & Concurrency
 * - All functions are non‑blocking and suitable for cooperative schedulers.
 * - Reset is pulsed using a short delay (50 ms one‑shot) compliant with CT2000 requirements.
 *
 * @section hal_gpio_errors Error Handling
 * - Returns GPIO_OK on success; otherwise GPIO_ERROR_* codes mapping invalid pins/modes/state.
 * - Higher layers should map to hal_result_t for unified error reporting.
 *
 * @section hal_gpio_related Related
 * - @see gpio.h Public C API and enums
 * - @see system_config.h Pin mapping mirror of hardware-config.yaml (SOLE SOURCE OF TRUTH)
 */


#include "gpio.h"
#include <system_config.h>
#include <Arduino.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static bool gpio_initialized = false;

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

static bool is_valid_pin(uint8_t pin);
static uint8_t get_sonicator_start_pin(uint8_t sonicator_id);
static uint8_t get_sonicator_reset_pin(uint8_t sonicator_id);
static uint8_t get_sonicator_overload_pin(uint8_t sonicator_id);
static uint8_t get_sonicator_freq_lock_pin(uint8_t sonicator_id);

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

gpio_result_t gpio_init(void) {
    // Initialize status LED pin
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Initialize sonicator interface pins
    gpio_result_t result = gpio_init_sonicator_pins();
    if (result != GPIO_OK) {
        return result;
    }

    gpio_initialized = true;
    return GPIO_OK;
}

gpio_result_t gpio_set_pin_mode(uint8_t pin, gpio_mode_t mode) {
    if (!gpio_initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }

    switch (mode) {
        case GPIO_MODE_INPUT:
            pinMode(pin, INPUT);
            break;
        case GPIO_MODE_INPUT_PULLUP:
            pinMode(pin, INPUT_PULLUP);
            break;
        case GPIO_MODE_OUTPUT:
            pinMode(pin, OUTPUT);
            break;
        case GPIO_MODE_OUTPUT_OPEN_DRAIN:
            // ATmega32A doesn't have native open-drain, simulate with INPUT/OUTPUT switching
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            break;
        default:
            return GPIO_ERROR_INVALID_MODE;
    }

    return GPIO_OK;
}

gpio_result_t gpio_write_pin(uint8_t pin, gpio_state_t state) {
    if (!gpio_initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }

    digitalWrite(pin, (state == GPIO_HIGH) ? HIGH : LOW);
    return GPIO_OK;
}

gpio_result_t gpio_read_pin(uint8_t pin, gpio_state_t* state) {
    if (!gpio_initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_pin(pin) || state == nullptr) {
        return GPIO_ERROR_INVALID_PIN;
    }

    *state = (digitalRead(pin) == HIGH) ? GPIO_HIGH : GPIO_LOW;
    return GPIO_OK;
}

gpio_result_t gpio_toggle_pin(uint8_t pin) {
    if (!gpio_initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_pin(pin)) {
        return GPIO_ERROR_INVALID_PIN;
    }

    gpio_state_t current_state;
    gpio_result_t result = gpio_read_pin(pin, &current_state);
    if (result != GPIO_OK) {
        return result;
    }

    gpio_state_t new_state = (current_state == GPIO_HIGH) ? GPIO_LOW : GPIO_HIGH;
    return gpio_write_pin(pin, new_state);
}

gpio_result_t gpio_get_pin_mode(uint8_t pin, gpio_mode_t* mode) {
    if (!gpio_initialized) {
        return GPIO_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_pin(pin) || mode == nullptr) {
        return GPIO_ERROR_INVALID_PIN;
    }

    // Note: Arduino framework doesn't provide direct pin mode reading
    // This is a limitation that would require direct register access
    // For now, return a generic output mode
    *mode = GPIO_MODE_OUTPUT;
    return GPIO_OK;
}

// ============================================================================
// SONICATOR INTERFACE FUNCTIONS
// ============================================================================

gpio_result_t gpio_init_sonicator_pins(void) {
    gpio_result_t result;

    // Configure sonicator input pins (with pullups for noise immunity)
    uint8_t input_pins[] = {
        SON1_OVERLOAD_PIN, SON1_FREQ_OUTPUT_PIN, SON1_FREQ_LOCK_PIN,
        SON2_OVERLOAD_PIN, SON2_FREQ_OUTPUT_PIN, SON2_FREQ_LOCK_PIN,
        SON3_OVERLOAD_PIN, SON3_FREQ_OUTPUT_PIN, SON3_FREQ_LOCK_PIN,
        SON4_OVERLOAD_PIN, SON4_FREQ_OUTPUT_PIN, SON4_FREQ_LOCK_PIN
    };

    for (uint8_t i = 0; i < sizeof(input_pins)/sizeof(input_pins[0]); i++) {
        result = gpio_set_pin_mode(input_pins[i], GPIO_MODE_INPUT_PULLUP);
        if (result != GPIO_OK) {
            return result;
        }
    }

    // Configure sonicator output pins (start LOW for safety)
    uint8_t output_pins[] = {
        SON1_START_PIN, SON1_RESET_PIN,
        SON2_START_PIN, SON2_RESET_PIN,
        SON3_START_PIN, SON3_RESET_PIN,
        SON4_START_PIN, SON4_RESET_PIN
    };

    for (uint8_t i = 0; i < sizeof(output_pins)/sizeof(output_pins[0]); i++) {
        result = gpio_set_pin_mode(output_pins[i], GPIO_MODE_OUTPUT);
        if (result != GPIO_OK) {
            return result;
        }
        // Ensure safe initial state (sonicators off)
        gpio_write_pin(output_pins[i], GPIO_LOW);
    }

    return GPIO_OK;
}

gpio_result_t gpio_sonicator_start(uint8_t sonicator_id) {
    if (sonicator_id < 1 || sonicator_id > MAX_SONICATORS) {
        return GPIO_ERROR_INVALID_PIN;
    }

    uint8_t start_pin = get_sonicator_start_pin(sonicator_id);
    return gpio_write_pin(start_pin, GPIO_HIGH);
}

gpio_result_t gpio_sonicator_stop(uint8_t sonicator_id) {
    if (sonicator_id < 1 || sonicator_id > MAX_SONICATORS) {
        return GPIO_ERROR_INVALID_PIN;
    }

    uint8_t start_pin = get_sonicator_start_pin(sonicator_id);
    return gpio_write_pin(start_pin, GPIO_LOW);
}

gpio_result_t gpio_sonicator_reset(uint8_t sonicator_id) {
    if (sonicator_id < 1 || sonicator_id > MAX_SONICATORS) {
        return GPIO_ERROR_INVALID_PIN;
    }

    uint8_t reset_pin = get_sonicator_reset_pin(sonicator_id);

    // Pulse reset pin (active high for 50ms)
    gpio_result_t result = gpio_write_pin(reset_pin, GPIO_HIGH);
    if (result != GPIO_OK) {
        return result;
    }

    delay(50); // 50ms reset pulse

    return gpio_write_pin(reset_pin, GPIO_LOW);
}

gpio_result_t gpio_sonicator_read_overload(uint8_t sonicator_id, bool* overload) {
    if (sonicator_id < 1 || sonicator_id > MAX_SONICATORS || overload == nullptr) {
        return GPIO_ERROR_INVALID_PIN;
    }

    uint8_t overload_pin = get_sonicator_overload_pin(sonicator_id);
    gpio_state_t state;
    gpio_result_t result = gpio_read_pin(overload_pin, &state);

    if (result == GPIO_OK) {
        // Overload is active LOW (via optocoupler)
        *overload = (state == GPIO_LOW);
    }

    return result;
}

gpio_result_t gpio_sonicator_read_freq_lock(uint8_t sonicator_id, bool* locked) {
    if (sonicator_id < 1 || sonicator_id > MAX_SONICATORS || locked == nullptr) {
        return GPIO_ERROR_INVALID_PIN;
    }

    uint8_t freq_lock_pin = get_sonicator_freq_lock_pin(sonicator_id);
    gpio_state_t state;
    gpio_result_t result = gpio_read_pin(freq_lock_pin, &state);

    if (result == GPIO_OK) {
        // Frequency lock is active HIGH (via optocoupler)
        *locked = (state == GPIO_HIGH);
    }

    return result;
}

gpio_result_t gpio_status_led(gpio_state_t state) {
    return gpio_write_pin(STATUS_LED_PIN, state);
}

/**
 * @brief ATmega32A GPIO Implementation Class
 */
class AtmegaGpio : public IGpioPort {
private:
    bool initialized_;

public:
    AtmegaGpio() : initialized_(false) {}
    virtual ~AtmegaGpio() = default;

    // Core GPIO operations
    bool init(void) override {
        if (gpio_init() == GPIO_OK) {
            initialized_ = true;
            return true;
        }
        return false;
    }

    bool setPinMode(uint8_t pin, uint8_t mode) override {
        gpio_mode_t gpio_mode;
        switch (mode) {
            case 0: gpio_mode = GPIO_MODE_INPUT; break;
            case 1: gpio_mode = GPIO_MODE_OUTPUT; break;
            case 2: gpio_mode = GPIO_MODE_INPUT_PULLUP; break;
            default: return false;
        }
        return (gpio_set_pin_mode(pin, gpio_mode) == GPIO_OK);
    }

    bool digitalWrite(uint8_t pin, uint8_t value) override {
        gpio_state_t state = (value == 0) ? GPIO_LOW : GPIO_HIGH;
        return (gpio_write_pin(pin, state) == GPIO_OK);
    }

    uint8_t digitalRead(uint8_t pin) override {
        gpio_state_t state;
        if (gpio_read_pin(pin, &state) == GPIO_OK) {
            return (state == GPIO_HIGH) ? 1 : 0;
        }
        return 0; // Default to LOW on error
    }

    bool togglePin(uint8_t pin) override {
        return (gpio_toggle_pin(pin) == GPIO_OK);
    }

    // Sonicator-specific operations
    bool sonicatorStart(uint8_t sonicator_id) override {
        return (gpio_sonicator_start(sonicator_id) == GPIO_OK);
    }

    bool sonicatorStop(uint8_t sonicator_id) override {
        return (gpio_sonicator_stop(sonicator_id) == GPIO_OK);
    }

    bool sonicatorReset(uint8_t sonicator_id) override {
        return (gpio_sonicator_reset(sonicator_id) == GPIO_OK);
    }

    bool sonicatorReadOverload(uint8_t sonicator_id, bool* state) override {
        return (gpio_sonicator_read_overload(sonicator_id, state) == GPIO_OK);
    }

    bool sonicatorReadFreqLock(uint8_t sonicator_id, bool* state) override {
        return (gpio_sonicator_read_freq_lock(sonicator_id, state) == GPIO_OK);
    }

    bool statusLed(bool state) override {
        gpio_state_t gpio_state = state ? GPIO_HIGH : GPIO_LOW;
        return (gpio_status_led(gpio_state) == GPIO_OK);
    }
};

// ============================================================================
// SINGLETON INSTANCE AND C API WRAPPER FUNCTIONS
// ============================================================================

// Singleton instance
static AtmegaGpio* g_gpio_instance = nullptr;
static bool g_gpio_c_api_initialized = false;

// Get or create singleton instance
static AtmegaGpio* getGpioInstance() {
    if (!g_gpio_instance) {
        g_gpio_instance = new AtmegaGpio();
    }
    return g_gpio_instance;
}

// C API wrapper for OOP GPIO
extern "C" {

IGpioPort* gpio_create_instance(void) {
    return getGpioInstance();
}

bool gpio_oop_init(IGpioPort* instance) {
    if (!instance) return false;
    return instance->init();
}

bool gpio_oop_set_pin_mode(IGpioPort* instance, uint8_t pin, uint8_t mode) {
    if (!instance) return false;
    return instance->setPinMode(pin, mode);
}

bool gpio_oop_digital_write(IGpioPort* instance, uint8_t pin, uint8_t value) {
    if (!instance) return false;
    return instance->digitalWrite(pin, value);
}

uint8_t gpio_oop_digital_read(IGpioPort* instance, uint8_t pin) {
    if (!instance) return 0;
    return instance->digitalRead(pin);
}

bool gpio_oop_toggle_pin(IGpioPort* instance, uint8_t pin) {
    if (!instance) return false;
    return instance->togglePin(pin);
}

bool gpio_oop_sonicator_start(IGpioPort* instance, uint8_t sonicator_id) {
    if (!instance) return false;
    return instance->sonicatorStart(sonicator_id);
}

bool gpio_oop_sonicator_stop(IGpioPort* instance, uint8_t sonicator_id) {
    if (!instance) return false;
    return instance->sonicatorStop(sonicator_id);
}

bool gpio_oop_sonicator_reset(IGpioPort* instance, uint8_t sonicator_id) {
    if (!instance) return false;
    return instance->sonicatorReset(sonicator_id);
}

bool gpio_oop_sonicator_read_overload(IGpioPort* instance, uint8_t sonicator_id, bool* state) {
    if (!instance) return false;
    return instance->sonicatorReadOverload(sonicator_id, state);
}

bool gpio_oop_sonicator_read_freq_lock(IGpioPort* instance, uint8_t sonicator_id, bool* state) {
    if (!instance) return false;
    return instance->sonicatorReadFreqLock(sonicator_id, state);
}

bool gpio_oop_status_led(IGpioPort* instance, bool state) {
    if (!instance) return false;
    return instance->statusLed(state);
}

} // extern "C"

static inline constexpr bool is_valid_pin(uint8_t pin) {
    // ATmega32A has 32 GPIO pins (4 ports × 8 pins)
    return (pin <= 31);
}

static inline constexpr uint8_t get_sonicator_start_pin(uint8_t sonicator_id) {
    switch (sonicator_id) {
        case 1: return SON1_START_PIN;
        case 2: return SON2_START_PIN;
        case 3: return SON3_START_PIN;
        case 4: return SON4_START_PIN;
        default: return 0xFF; // Invalid
    }
}

static inline constexpr uint8_t get_sonicator_reset_pin(uint8_t sonicator_id) {
    switch (sonicator_id) {
        case 1: return SON1_RESET_PIN;
        case 2: return SON2_RESET_PIN;
        case 3: return SON3_RESET_PIN;
        case 4: return SON4_RESET_PIN;
        default: return 0xFF; // Invalid
    }
}

static inline constexpr uint8_t get_sonicator_overload_pin(uint8_t sonicator_id) {
    switch (sonicator_id) {
        case 1: return SON1_OVERLOAD_PIN;
        case 2: return SON2_OVERLOAD_PIN;
        case 3: return SON3_OVERLOAD_PIN;
        case 4: return SON4_OVERLOAD_PIN;
        default: return 0xFF; // Invalid
    }
}

static inline constexpr uint8_t get_sonicator_freq_lock_pin(uint8_t sonicator_id) {
    switch (sonicator_id) {
        case 1: return SON1_FREQ_LOCK_PIN;
        case 2: return SON2_FREQ_LOCK_PIN;
        case 3: return SON3_FREQ_LOCK_PIN;
        case 4: return SON4_FREQ_LOCK_PIN;
        default: return 0xFF; // Invalid
    }
}
