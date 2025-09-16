#ifndef SONICATOR_CONTROL_H
#define SONICATOR_CONTROL_H

/**
 * @brief Sonicator control commands
 */
typedef enum {
    SONICATOR_CMD_NONE = 0,         //< No command pending
    SONICATOR_CMD_START,            //< Start sonicator
    SONICATOR_CMD_STOP,             //< Stop sonicator  
    SONICATOR_CMD_RESET_OVERLOAD,   //< Reset overload condition
    SONICATOR_CMD_EMERGENCY_STOP    //< Emergency stop (immediate)
} sonicator_command_t;

#endif // SONICATOR_CONTROL_H
