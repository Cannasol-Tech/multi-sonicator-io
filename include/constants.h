

/**
 * @file constants.h
 * @brief Constants for timing intervals in the main loop
 * @author Stephen Boyett
 * @date 2025-09-16
 * @version 1.0.0
 */


#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============================================================================
// TIMING CONSTANTS
// ============================================================================

/**
 * @brief Default amplitude for sonicators
*/
#define DEFAULT_SONICATOR_AMPLITUDE 80

/**
 * @brief Main Loop Interval for MODBUS processing
*/
const unsigned long MODBUS_PROCESS_INTERVAL_MS = 5;

/**
 * @brief Main Loop Interval for multiplexer updates
*/  
const unsigned long MULTIPLEXER_UPDATE_INTERVAL_MS = 10;

/**
 * @brief Main Loop Interval for MODBUS register sync
*/
const unsigned long MODBUS_SYNC_INTERVAL_MS = 50;



#endif // CONSTANTS_H