/**
 * @file sonicator4_controller.h
 * @brief Control loop for Sonicator 4 (unit id = 4, zero-based 3)
 */
#ifndef SONICATOR4_CONTROLLER_H
#define SONICATOR4_CONTROLLER_H

#include <stdint.h>
#include "modules/hal/hal.h"
#include "modules/communication/modbus_register_manager.h"
#include "register_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Sonicator 4 controller (no-op for now)
 */
static inline void son4_init(void) {}

/**
 * @brief Service Sonicator 4 control from registers and update telemetry.
 * Call at 10-100 Hz.
 */
static inline void son4_service(void) {
    // Sonicator 4 is id=4 for HAL; zero-based id 3 for registers
    const uint8_t hal_id = 4;
    const uint8_t reg_id = 3;

    uint16_t start_stop = 0, amplitude_sp = 50;
    if (!register_manager_get_sonicator_control(reg_id, &start_stop, &amplitude_sp)) {
        return;
    }

    // Clamp amplitude to 20-100%
    if (amplitude_sp < 20) amplitude_sp = 20;
    if (amplitude_sp > 100) amplitude_sp = 100;

    sonicator_control_t ctrl;
    ctrl.start = (start_stop != 0);
    ctrl.amplitude_percent = (uint8_t)amplitude_sp;
    // Reset command is write-only; we treat nonzero as momentary reset
    ctrl.reset_overload = false;

    (void)hal_control_sonicator(hal_id, &ctrl);

    sonicator_status_t st = {0};
    if (hal_read_sonicator_status(hal_id, &st) == HAL_OK) {
        uint16_t flags = 0;
        if (st.frequency_locked) flags |= SON_STATUS_FREQ_LOCK;
        if (st.overload)         flags |= SON_STATUS_OVERLOAD;
        if (ctrl.start)          flags |= SON_STATUS_RUNNING;

        uint16_t power_w = (st.power_watts < 0) ? 0 : (uint16_t)st.power_watts;
        register_manager_update_sonicator_status(
            reg_id,
            power_w,
            st.frequency_hz,
            ctrl.amplitude_percent, // Treat setpoint as actual until feedback available
            flags
        );
    }
}

#ifdef __cplusplus
}
#endif

#endif // SONICATOR4_CONTROLLER_H

