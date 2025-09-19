/**
 * @file mock_hal_impl.cpp
 * @brief Definitions of MockHal static members
 */
#include "mock_hal_impl.h"

bool MockHal::gpio_initialized = false;
uint8_t MockHal::pin_states[32] = {0};
uint8_t MockHal::pin_modes[32] = {0};

bool MockHal::adc_initialized = false;
uint16_t MockHal::adc_channel_values[8] = {512, 256, 768, 384, 640, 128, 896, 0};
bool MockHal::adc_conversion_started = false;
bool MockHal::adc_conversion_complete = false;

bool MockHal::pwm_initialized = false;
uint8_t MockHal::pwm_duty_cycles[4] = {0};
uint32_t MockHal::pwm_frequencies[4] = {0};
bool MockHal::pwm_channels_enabled[4] = {false};

bool MockHal::timer_initialized = false;
bool MockHal::timer_expired_flags[4] = {false};
uint32_t MockHal::timer_start_times[4] = {0};
uint32_t MockHal::timer_periods[4] = {0};
uint16_t MockHal::timer_values[4] = {0};
uint32_t MockHal::mock_millis = 0;
uint32_t MockHal::mock_micros = 0;

bool MockHal::uart_initialized = false;
uint32_t MockHal::uart_baud_rate = 9600;
uint8_t MockHal::uart_tx_buffer[256] = {0};
uint8_t MockHal::uart_rx_buffer[256] = {0};
uint16_t MockHal::uart_tx_head = 0;
uint16_t MockHal::uart_rx_head = 0;
uint16_t MockHal::uart_tx_tail = 0;
uint16_t MockHal::uart_rx_tail = 0;