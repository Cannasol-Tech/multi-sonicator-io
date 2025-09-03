#include <Arduino.h>

// Some MightyCore variants may miss the timer mapping symbol in certain PIO configs.
// Provide a minimal map that marks all pins as NOT_ON_TIMER to satisfy linker.
extern "C" {
#if !defined(digital_pin_to_timer_PGM)
const uint8_t digital_pin_to_timer_PGM[] PROGMEM = {
    // ATmega32 typically exposes up to 32 digital pins via MightyCore
    // Fill with NOT_ON_TIMER entries; PWM will still be configured via our HAL where needed.
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER,
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER,
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER,
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER,
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER,
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER,
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER,
    NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER, NOT_ON_TIMER
};
#endif
}

