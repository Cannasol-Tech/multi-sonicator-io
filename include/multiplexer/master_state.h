#ifndef MULTIPLEXER_MASTER_STATE_H
#define MULTIPLEXER_MASTER_STATE_H


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MASTER_STATE_IDLE = 0,
    MASTER_STATE_COORDINATED_START = 1,
    MASTER_STATE_RUNNING = 2,
    MASTER_STATE_EMERGENCY_STOP = 3,
    MASTER_STATE_FAULT_ISOLATION = 4
} master_state_t;

#ifdef __cplusplus
}
#endif

#endif // MULTIPLEXER_MASTER_STATE_H
