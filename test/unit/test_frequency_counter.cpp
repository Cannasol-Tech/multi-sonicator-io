/**
 * @file test_frequency_counter.cpp
 * @brief Unit tests for frequency counter module
 * @author Cannasol Technologies
 * @date 2025-01-10
 * @version 1.0.0
 * 
 * Tests the ISR-based frequency counting functionality including
 * initialization, edge counting, frequency calculation, and error handling.
 */

#include "unity.h"
#include "frequency_counter.h"
#include <Arduino.h>

// Test configuration
#define TEST_TIMEOUT_MS 5000

// Mock micros() for controlled testing
static uint32_t mock_micros_value = 0;
static bool use_mock_micros = false;

// Override micros() for testing
uint32_t micros() {
    if (use_mock_micros) {
        return mock_micros_value;
    }
    return 0; // Default for tests that don't use timing
}

void setUp(void) {
    // Reset mock timing
    mock_micros_value = 0;
    use_mock_micros = false;
    
    // Initialize frequency counter system
    frequency_counter_init();
}

void tearDown(void) {
    // Reset all statistics
    frequency_reset_stats(0xFF);
}

/**
 * @brief Test initialization of frequency counter system
 */
void test_frequency_counter_init(void) {
    // Verify initialization returns success
    bool result = frequency_counter_init();
    TEST_ASSERT_TRUE(result);
    
    // Verify all channels are initialized to zero state
    for (uint8_t i = 0; i < 4; i++) {
        frequency_stats_t stats;
        bool stats_result = frequency_get_stats(i, &stats);
        
        TEST_ASSERT_TRUE(stats_result);
        TEST_ASSERT_EQUAL(0, stats.current_frequency);
        TEST_ASSERT_EQUAL(0, stats.total_measurements);
        TEST_ASSERT_EQUAL(0, stats.error_count);
        TEST_ASSERT_EQUAL(0, stats.total_edges);
    }
}

/**
 * @brief Test getting current frequency (should be 0 initially)
 */
void test_frequency_get_current_initial(void) {
    for (uint8_t i = 0; i < 4; i++) {
        uint16_t frequency = frequency_get_current(i);
        TEST_ASSERT_EQUAL(0, frequency);
    }
}

/**
 * @brief Test invalid channel ID handling
 */
void test_frequency_invalid_channel(void) {
    // Test invalid channel for frequency_get_current
    uint16_t freq = frequency_get_current(4); // Invalid channel
    TEST_ASSERT_EQUAL(0, freq);
    
    freq = frequency_get_current(255); // Invalid channel
    TEST_ASSERT_EQUAL(0, freq);
    
    // Test invalid channel for frequency_calculate
    freq = frequency_calculate(4);
    TEST_ASSERT_EQUAL(0, freq);
    
    // Test invalid channel for frequency_get_stats
    frequency_stats_t stats;
    bool result = frequency_get_stats(4, &stats);
    TEST_ASSERT_FALSE(result);
    
    // Test invalid channel for frequency_get_error
    frequency_error_t error = frequency_get_error(4);
    TEST_ASSERT_EQUAL(FREQ_ERROR_INVALID_CHANNEL, error);
}

/**
 * @brief Test NULL pointer handling in get_stats
 */
void test_frequency_get_stats_null_pointer(void) {
    bool result = frequency_get_stats(0, NULL);
    TEST_ASSERT_FALSE(result);
}

/**
 * @brief Test frequency calculation with insufficient edges
 */
void test_frequency_calculate_insufficient_edges(void) {
    use_mock_micros = true;
    mock_micros_value = 0;
    
    // Simulate initialization time
    frequency_counter_init();
    
    // Simulate measurement window completion with insufficient edges
    // Manually set the measurement_ready flag and low edge count
    freq_counters[0].edge_count = 5; // Less than FREQ_MIN_EDGES (10)
    freq_counters[0].window_start_time = 0;
    freq_counters[0].measurement_ready = true;
    
    mock_micros_value = FREQ_SAMPLE_WINDOW_MS * 1000; // 1 second later
    
    uint16_t frequency = frequency_calculate(0);
    TEST_ASSERT_EQUAL(0, frequency); // Should return 0 for insufficient edges
    
    // Check that error count was incremented
    frequency_stats_t stats;
    frequency_get_stats(0, &stats);
    TEST_ASSERT_GREATER_THAN(0, stats.error_count);
}

/**
 * @brief Test frequency calculation with valid measurement
 */
void test_frequency_calculate_valid_measurement(void) {
    use_mock_micros = true;
    mock_micros_value = 0;
    
    // Initialize
    frequency_counter_init();
    
    // Simulate a 2000 Hz input signal (200 edges in 100ms)
    // This should result in 20000 Hz output (2000 * 10)
    freq_counters[0].edge_count = 200;
    freq_counters[0].window_start_time = 0;
    freq_counters[0].measurement_ready = true;
    
    mock_micros_value = 100000; // 100ms later
    
    uint16_t frequency = frequency_calculate(0);
    
    // Expected: (200 edges * 1000000 µs) / 100000 µs = 2000 Hz raw
    // With 10x multiplier: 2000 * 10 = 20000 Hz
    TEST_ASSERT_EQUAL(20000, frequency);
    
    // Check statistics
    frequency_stats_t stats;
    frequency_get_stats(0, &stats);
    TEST_ASSERT_EQUAL(20000, stats.current_frequency);
    TEST_ASSERT_EQUAL(1, stats.total_measurements);
    TEST_ASSERT_EQUAL(0, stats.error_count);
}

/**
 * @brief Test frequency calculation with out-of-range result
 */
void test_frequency_calculate_out_of_range(void) {
    use_mock_micros = true;
    mock_micros_value = 0;
    
    frequency_counter_init();
    
    // Simulate impossibly high frequency (will exceed range limits)
    freq_counters[0].edge_count = 1000; // Very high edge count
    freq_counters[0].window_start_time = 0;
    freq_counters[0].measurement_ready = true;
    freq_counters[0].calculated_frequency = 20000; // Previous valid value
    
    mock_micros_value = 10000; // 10ms later - very short window
    
    uint16_t frequency = frequency_calculate(0);
    
    // Should return previous valid frequency due to out-of-range result
    TEST_ASSERT_EQUAL(20000, frequency);
    
    // Check that error count was incremented
    frequency_stats_t stats;
    frequency_get_stats(0, &stats);
    TEST_ASSERT_GREATER_THAN(0, stats.error_count);
}

/**
 * @brief Test error detection - no signal
 */
void test_frequency_error_no_signal(void) {
    use_mock_micros = true;
    mock_micros_value = 0;
    
    frequency_counter_init();
    
    // Simulate no edges for extended period (> 2 seconds)
    mock_micros_value = 3000000; // 3 seconds
    
    frequency_error_t error = frequency_get_error(0);
    TEST_ASSERT_EQUAL(FREQ_ERROR_NO_SIGNAL, error);
}

/**
 * @brief Test statistics reset functionality
 */
void test_frequency_reset_stats(void) {
    // Set some non-zero statistics
    freq_counters[0].total_measurements = 10;
    freq_counters[0].error_count = 5;
    freq_counters[0].noise_filter_count = 3;
    freq_counters[0].total_edges = 100;
    
    // Reset single channel
    bool result = frequency_reset_stats(0);
    TEST_ASSERT_TRUE(result);
    
    // Verify reset
    frequency_stats_t stats;
    frequency_get_stats(0, &stats);
    TEST_ASSERT_EQUAL(0, stats.total_measurements);
    TEST_ASSERT_EQUAL(0, stats.error_count);
    TEST_ASSERT_EQUAL(0, stats.noise_filter_count);
    TEST_ASSERT_EQUAL(0, stats.total_edges);
}

/**
 * @brief Test reset all channels
 */
void test_frequency_reset_all_channels(void) {
    // Set non-zero statistics for all channels
    for (uint8_t i = 0; i < 4; i++) {
        freq_counters[i].total_measurements = 10 + i;
        freq_counters[i].error_count = 5 + i;
    }
    
    // Reset all channels
    bool result = frequency_reset_stats(0xFF);
    TEST_ASSERT_TRUE(result);
    
    // Verify all channels reset
    for (uint8_t i = 0; i < 4; i++) {
        frequency_stats_t stats;
        frequency_get_stats(i, &stats);
        TEST_ASSERT_EQUAL(0, stats.total_measurements);
        TEST_ASSERT_EQUAL(0, stats.error_count);
    }
}

/**
 * @brief Test ISR statistics functions
 */
void test_frequency_isr_stats(void) {
    uint32_t total_interrupts, last_isr_time;
    
    bool result = frequency_get_isr_stats(&total_interrupts, &last_isr_time);
    TEST_ASSERT_TRUE(result);
    
    // Test NULL pointer handling
    result = frequency_get_isr_stats(NULL, &last_isr_time);
    TEST_ASSERT_FALSE(result);
    
    result = frequency_get_isr_stats(&total_interrupts, NULL);
    TEST_ASSERT_FALSE(result);
}

/**
 * @brief Test noise filter configuration
 */
void test_frequency_noise_filter_config(void) {
    // Test single channel
    bool result = frequency_set_noise_filter(0, false);
    TEST_ASSERT_TRUE(result);
    
    result = frequency_set_noise_filter(0, true);
    TEST_ASSERT_TRUE(result);
    
    // Test all channels
    result = frequency_set_noise_filter(0xFF, false);
    TEST_ASSERT_TRUE(result);
    
    // Test invalid channel
    result = frequency_set_noise_filter(4, true);
    TEST_ASSERT_FALSE(result);
}

/**
 * @brief Test measurement window configuration
 */
void test_frequency_measurement_window_config(void) {
    // Test valid window sizes
    bool result = frequency_set_window(0, 500);  // 500ms
    TEST_ASSERT_TRUE(result);
    
    result = frequency_set_window(0, 2000);  // 2 seconds
    TEST_ASSERT_TRUE(result);
    
    // Test invalid window sizes
    result = frequency_set_window(0, 50);   // Too small
    TEST_ASSERT_FALSE(result);
    
    result = frequency_set_window(0, 10000); // Too large
    TEST_ASSERT_FALSE(result);
    
    // Test invalid channel
    result = frequency_set_window(4, 1000);
    TEST_ASSERT_FALSE(result);
}

// Test runner setup
void test_frequency_counter_module(void) {
    RUN_TEST(test_frequency_counter_init);
    RUN_TEST(test_frequency_get_current_initial);
    RUN_TEST(test_frequency_invalid_channel);
    RUN_TEST(test_frequency_get_stats_null_pointer);
    RUN_TEST(test_frequency_calculate_insufficient_edges);
    RUN_TEST(test_frequency_calculate_valid_measurement);
    RUN_TEST(test_frequency_calculate_out_of_range);
    RUN_TEST(test_frequency_error_no_signal);
    RUN_TEST(test_frequency_reset_stats);
    RUN_TEST(test_frequency_reset_all_channels);
    RUN_TEST(test_frequency_isr_stats);
    RUN_TEST(test_frequency_noise_filter_config);
    RUN_TEST(test_frequency_measurement_window_config);
}

// Arduino-style setup for running tests
void setup() {
    delay(2000); // Wait for serial monitor
    UNITY_BEGIN();
    test_frequency_counter_module();
    UNITY_END();
}

void loop() {
    // Tests run once in setup()
}