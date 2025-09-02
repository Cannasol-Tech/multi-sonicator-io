/**
 * @file unity_config.h
 * @brief Unity test framework configuration for Multi-Sonicator I/O Controller
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

// Memory optimization for ATMEGA32A
#define UNITY_EXCLUDE_FLOAT
#define UNITY_EXCLUDE_DOUBLE
#define UNITY_EXCLUDE_FLOAT_PRINT

// Use 16-bit integers for line numbers to save memory
#define UNITY_LINE_TYPE uint16_t

// Custom output for Arduino Serial
#ifdef ARDUINO
    #include <Arduino.h>
    #define UNITY_OUTPUT_START()    Serial.begin(115200)
    #define UNITY_OUTPUT_CHAR(a)    Serial.write(a)
    #define UNITY_OUTPUT_FLUSH()    Serial.flush()
    #define UNITY_OUTPUT_COMPLETE() Serial.end()
#else
    // Native platform uses stdio
    #include <stdio.h>
    #define UNITY_OUTPUT_CHAR(a)    putchar(a)
    #define UNITY_OUTPUT_FLUSH()    fflush(stdout)
#endif

// Memory constraints
#define UNITY_MAX_ARRAY_LENGTH 32
#define UNITY_SUPPORT_64 0

#endif // UNITY_CONFIG_H
