#ifndef MULTIPLEXER_MASTER_STATE_H
#define MULTIPLEXER_MASTER_STATE_H


#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Master state enumeration for multi-sonicator coordination
 * 
 * Defines the high-level states for coordinating multiple sonicator units.
 * State transitions follow safety requirements and fault isolation protocols.
 */
typedef enum {
    MASTER_STATE_IDLE = 0,              /**< System idle, ready for commands */
    MASTER_STATE_COORDINATED_START = 1, /**< Coordinated start sequence in progress */
    MASTER_STATE_RUNNING = 2,           /**< System running with active units */
    MASTER_STATE_EMERGENCY_STOP = 3,    /**< Emergency stop condition active */
    MASTER_STATE_FAULT_ISOLATION = 4    /**< Fault isolation mode active */
} master_state_t;

#ifdef __cplusplus
}
#endif

#endif // MULTIPLEXER_MASTER_STATE_H
