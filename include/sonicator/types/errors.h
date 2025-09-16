#ifndef SONICATOR_ERRORS_H
#define SONICATOR_ERRORS_H


/**
 * @brief Sonicator fault conditions
 */
typedef enum {
    SONICATOR_FAULT_NONE = 0,       //< No fault
    SONICATOR_FAULT_OVERLOAD = 0x01,    //< Overload detected
    SONICATOR_FAULT_FREQ_UNLOCK = 0x02, //< Frequency lock lost
    SONICATOR_FAULT_COMM_TIMEOUT = 0x04, //< Communication timeout
    SONICATOR_FAULT_HARDWARE = 0x08,     //< Hardware fault
    SONICATOR_FAULT_WATCHDOG = 0x10      //< Watchdog timeout
} sonicator_fault_t;


#endif // SONICATOR_ERRORS_H
