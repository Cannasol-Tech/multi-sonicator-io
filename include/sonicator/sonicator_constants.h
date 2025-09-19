
/**
 * @file sonicator_constants.h
 * @brief C-only public constants for sonicator interface (safe for C unit tests)
 * @company Cannasol Technologies
 * @date 2025-09-19
 * @version 1.0.0
 *
 * @details
 * Provides compile-time constants used by the sonicator interface without
 * including any C++ classes. This header is intended for inclusion in C unit
 * tests and low-level modules that only need the constant values.
 *
 * Reference: See include/sonicator/sonicator.h for the full C++ interface.
 */

#ifndef SONICATOR_CONSTANTS_H
#define SONICATOR_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Timing constants
// ----------------------------------------------------------------------------

/** Delay after stop command before transitioning to idle state (ms) */
#define SONICATOR_STOP_DELAY_MS         100

/** Delay after start command before transitioning to running state (ms) */
#define SONICATOR_START_DELAY_MS        50

/** Duration of reset pulse sent to CT2000 (ms) */
#define SONICATOR_RESET_PULSE_MS        20

/** Communication timeout threshold (ms) */
#define SONICATOR_COMM_TIMEOUT_MS       2000

/** Fault condition debounce time (ms) */
#define SONICATOR_FAULT_DEBOUNCE_MS     10

/** Watchdog timeout threshold (ms) */
#define SONICATOR_WATCHDOG_TIMEOUT_MS   1000

// ----------------------------------------------------------------------------
// Amplitude constraints
// ----------------------------------------------------------------------------

/** Minimum allowed amplitude percentage */
#define SONICATOR_MIN_AMPLITUDE_PERCENT 20

/** Maximum allowed amplitude percentage */
#define SONICATOR_MAX_AMPLITUDE_PERCENT 100

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SONICATOR_CONSTANTS_H

