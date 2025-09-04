#!/usr/bin/env python3
"""
HIL Control Step Definitions - ADC, PWM, and advanced control verification steps

This module provides step definitions for ADC measurements, PWM generation,
and advanced hardware control verification.

Author: Cannasol Technologies  
License: Proprietary
"""

import time
import statistics
from behave import given, when, then


@given('the ADC subsystem is initialized')
def step_adc_subsystem_initialized(context):
    """Verify ADC subsystem is initialized"""
    response = context.hardware_interface.send_command("ADC_STATUS")
    assert response and "ADC_OK" in response, "ADC subsystem not initialized"
    context.hil_logger.hardware_event("ADC subsystem initialized")


@given('the ADC reference is set to {voltage:f}V')
def step_adc_reference_set(context, voltage):
    """Set ADC reference voltage"""
    # Use reference voltage from config if not provided
    ref_voltage = voltage if voltage else context.config['testing']['adc_reference_voltage']
    response = context.hardware_interface.send_command(f"ADC_SET_REF {ref_voltage}")
    assert response and "OK" in response, f"Failed to set ADC reference to {ref_voltage}V"
    context.hil_logger.hardware_event(f"ADC reference set to {ref_voltage}V")


@given('the PWM subsystem is initialized')
def step_pwm_subsystem_initialized(context):
    """Verify PWM subsystem is initialized"""
    response = context.hardware_interface.send_command("PWM_STATUS")
    assert response and "PWM_OK" in response, "PWM subsystem not initialized"
    context.hil_logger.hardware_event("PWM subsystem initialized")


@given('pin "{pin}" is configured for PWM output')
def step_pin_configured_for_pwm(context, pin):
    """Configure pin for PWM output"""
    response = context.hardware_interface.send_command(f"CONFIG_PWM {pin}")
    assert response and "OK" in response, f"Failed to configure {pin} for PWM"
    context.hil_logger.hardware_event(f"Pin {pin} configured for PWM output")


@given('the power measurement scaling is {scaling:f} mV/W')
def step_power_measurement_scaling(context, scaling):
    """Set power measurement scaling factor"""
    context.power_scaling = scaling
    context.hil_logger.hardware_event(f"Power measurement scaling: {scaling} mV/W")


@when('I apply {voltage:f}V to ADC channel "{channel}"')
def step_apply_voltage_to_adc_channel(context, voltage, channel):
    """Apply voltage to ADC channel"""
    response = context.hardware_interface.send_command(f"APPLY_ADC_VOLTAGE {channel} {voltage}")
    assert response and "OK" in response, f"Failed to apply {voltage}V to ADC {channel}"
    context.applied_voltage = voltage
    context.adc_channel = channel
    context.hil_logger.hardware_event(f"Applied {voltage}V to ADC channel {channel}")


@when('I simulate {power:d}W power consumption')
def step_simulate_power_consumption(context, power):
    """Simulate power consumption for power sensing"""
    # Calculate voltage for power simulation using scaling factor
    scaling = getattr(context, 'power_scaling', 5.44)  # Default 5.44 mV/W
    voltage = (power * scaling) / 1000.0  # Convert mV to V
    
    step_apply_voltage_to_adc_channel(context, voltage, "POWER_SENSE_4")
    context.simulated_power = power
    context.hil_logger.hardware_event(f"Simulated {power}W power consumption ({voltage:.3f}V)")


@when('I apply a stable {voltage:f}V to ADC channel "{channel}"')
def step_apply_stable_voltage_to_adc(context, voltage, channel):
    """Apply stable voltage to ADC channel for stability testing"""
    step_apply_voltage_to_adc_channel(context, voltage, channel)
    context.stable_test_voltage = voltage


@when('I take {count:d} ADC readings over {duration:d} seconds')
def step_take_multiple_adc_readings(context, count, duration):
    """Take multiple ADC readings for stability analysis"""
    readings = []
    interval = duration / count
    
    for i in range(count):
        adc_value = context.hardware_interface.read_adc_channel(context.adc_channel)
        if adc_value is not None:
            readings.append(adc_value)
        time.sleep(interval)
    
    context.adc_readings = readings
    context.hil_logger.measurement(f"ADC readings count", len(readings), "", f"{count} expected")


@when('I apply {voltage1:f}V to ADC channel "{channel1}"')
def step_apply_voltage_channel1(context, voltage1, channel1):
    """Apply voltage to first ADC channel"""
    step_apply_voltage_to_adc_channel(context, voltage1, channel1)
    context.channel1 = channel1
    context.voltage1 = voltage1


@when('I apply {voltage2:f}V to another available ADC channel')
def step_apply_voltage_channel2(context, voltage2):
    """Apply voltage to second ADC channel"""
    # Use a different channel for independence test
    channel2 = "A0"  # Assuming A0 is available as second channel
    response = context.hardware_interface.send_command(f"APPLY_ADC_VOLTAGE {channel2} {voltage2}")
    assert response and "OK" in response, f"Failed to apply {voltage2}V to ADC {channel2}"
    
    context.channel2 = channel2
    context.voltage2 = voltage2
    context.hil_logger.hardware_event(f"Applied {voltage2}V to ADC channel {channel2}")


@when('I set PWM on pin "{pin}" to {duty:d}% duty cycle at {frequency:d}kHz')
def step_set_pwm_duty_and_frequency(context, pin, duty, frequency):
    """Set PWM duty cycle and frequency"""
    freq_hz = frequency * 1000
    response = context.hardware_interface.send_command(f"SET_PWM {pin} {duty} {freq_hz}")
    assert response and "OK" in response, f"Failed to set PWM on {pin}"
    
    context.pwm_pin = pin
    context.pwm_duty = duty
    context.pwm_frequency = freq_hz
    context.hil_logger.hardware_event(f"Set PWM {pin}: {duty}% @ {frequency}kHz")


@when('I set PWM duty cycle to {duty:d}% on pin "{pin}"')
def step_set_pwm_duty_cycle(context, duty, pin):
    """Set PWM duty cycle"""
    # Use default frequency if not specified
    frequency = getattr(context, 'pwm_frequency', 1000)
    response = context.hardware_interface.send_command(f"SET_PWM {pin} {duty} {frequency}")
    assert response and "OK" in response, f"Failed to set PWM duty cycle on {pin}"
    
    context.pwm_pin = pin
    context.pwm_duty = duty
    context.hil_logger.hardware_event(f"Set PWM duty cycle {pin}: {duty}%")


@when('I set PWM frequency to {frequency:d}Hz on pin "{pin}"')
def step_set_pwm_frequency(context, frequency, pin):
    """Set PWM frequency"""
    # Use default duty cycle if not specified
    duty = getattr(context, 'pwm_duty', 50)
    response = context.hardware_interface.send_command(f"SET_PWM {pin} {duty} {frequency}")
    assert response and "OK" in response, f"Failed to set PWM frequency on {pin}"
    
    context.pwm_pin = pin
    context.pwm_frequency = frequency
    context.hil_logger.hardware_event(f"Set PWM frequency {pin}: {frequency}Hz")


@when('I set PWM to {duty:d}% duty cycle on pin "{pin}"')
def step_set_pwm_simple_duty(context, duty, pin):
    """Set PWM to simple duty cycle"""
    step_set_pwm_duty_cycle(context, duty, pin)


@when('I set PWM to {duty:d}% duty cycle at {frequency:d}kHz on pin "{pin}"')
def step_set_pwm_duty_freq_simple(context, duty, frequency, pin):
    """Set PWM duty cycle and frequency (simple)"""
    step_set_pwm_duty_and_frequency(context, pin, duty, frequency)


@when('I measure PWM for {duration:d} seconds continuously')
def step_measure_pwm_continuously(context, duration):
    """Measure PWM continuously for stability testing"""
    measurements = []
    start_time = time.time()
    
    while time.time() - start_time < duration:
        pwm_data = context.hardware_interface.measure_pwm_output(context.pwm_pin)
        if pwm_data:
            measurements.append(pwm_data)
        time.sleep(0.5)  # Measure every 500ms
    
    context.pwm_measurements = measurements
    context.hil_logger.measurement(f"PWM measurements count", len(measurements))


@when('PWM is enabled on pin "{pin}" with {duty:d}% duty cycle')
def step_enable_pwm_with_duty(context, pin, duty):
    """Enable PWM with specified duty cycle"""
    step_set_pwm_duty_cycle(context, duty, pin)
    context.pwm_enabled = True


@when('I disable PWM on pin "{pin}"')
def step_disable_pwm(context, pin):
    """Disable PWM on pin"""
    response = context.hardware_interface.send_command(f"DISABLE_PWM {pin}")
    assert response and "OK" in response, f"Failed to disable PWM on {pin}"
    context.pwm_enabled = False
    context.hil_logger.hardware_event(f"PWM disabled on {pin}")


@when('I re-enable PWM with the same settings')
def step_reenable_pwm_same_settings(context):
    """Re-enable PWM with same settings"""
    pin = context.pwm_pin
    duty = context.pwm_duty
    frequency = context.pwm_frequency
    
    response = context.hardware_interface.send_command(f"SET_PWM {pin} {duty} {frequency}")
    assert response and "OK" in response, f"Failed to re-enable PWM on {pin}"
    context.pwm_enabled = True
    context.hil_logger.hardware_event(f"PWM re-enabled on {pin}")


@then('the ADC reading should be approximately {expected:d}')
def step_verify_adc_reading(context, expected):
    """Verify ADC reading is approximately expected value"""
    adc_value = context.hardware_interface.read_adc_channel(context.adc_channel)
    assert adc_value is not None, f"Failed to read ADC channel {context.adc_channel}"
    
    tolerance = 20  # ±20 ADC counts tolerance
    assert abs(adc_value - expected) <= tolerance, \
           f"ADC reading {adc_value} not within {tolerance} counts of {expected}"
    context.hil_logger.measurement(f"ADC {context.adc_channel}", adc_value, "counts", f"{expected} ±{tolerance}")


@then('the voltage should be approximately {expected:f}V ± {tolerance:f}V')
def step_verify_voltage_with_tolerance(context, expected, tolerance):
    """Verify voltage is within tolerance"""
    adc_value = context.hardware_interface.read_adc_channel(context.adc_channel)
    assert adc_value is not None, f"Failed to read ADC channel {context.adc_channel}"
    
    actual_voltage = context.hardware_interface.adc_to_voltage(adc_value)
    assert abs(actual_voltage - expected) <= tolerance, \
           f"Voltage {actual_voltage:.3f}V not within {tolerance}V of {expected}V"
    context.hil_logger.measurement(f"Voltage {context.adc_channel}", actual_voltage, "V", f"{expected}V ±{tolerance}V")


@then('the voltage should be approximately {expected:f}V')
def step_verify_voltage_simple(context, expected):
    """Verify voltage is approximately expected"""
    step_verify_voltage_with_tolerance(context, expected, 0.1)


@then('the ADC should read a voltage of approximately {expected:f}V')
def step_verify_adc_voltage(context, expected):
    """Verify ADC reads expected voltage"""
    step_verify_voltage_simple(context, expected)


@then('the calculated power should be approximately {expected:d}W ± {tolerance:d}W')
def step_verify_calculated_power(context, expected, tolerance):
    """Verify calculated power from ADC reading"""
    adc_value = context.hardware_interface.read_adc_channel(context.adc_channel)
    assert adc_value is not None, f"Failed to read ADC channel {context.adc_channel}"
    
    voltage = context.hardware_interface.adc_to_voltage(adc_value)
    scaling = getattr(context, 'power_scaling', 5.44)
    calculated_power = (voltage * 1000) / scaling  # Convert V to mV, then to W
    
    assert abs(calculated_power - expected) <= tolerance, \
           f"Calculated power {calculated_power:.1f}W not within {tolerance}W of {expected}W"
    context.hil_logger.measurement("Calculated power", calculated_power, "W", f"{expected}W ±{tolerance}W")


@then('the standard deviation should be less than {max_stddev:d} ADC counts')
def step_verify_adc_stability(context, max_stddev):
    """Verify ADC stability via standard deviation"""
    assert hasattr(context, 'adc_readings'), "No ADC readings available"
    assert len(context.adc_readings) > 1, "Need multiple readings for stability test"
    
    stddev = statistics.stdev(context.adc_readings)
    assert stddev < max_stddev, \
           f"ADC standard deviation {stddev:.2f} exceeds {max_stddev} counts"
    context.hil_logger.measurement("ADC standard deviation", stddev, "counts", f"< {max_stddev}")


@then('all readings should be within ±{tolerance:d} ADC counts of the mean')
def step_verify_adc_readings_within_tolerance(context, tolerance):
    """Verify all ADC readings are within tolerance of mean"""
    assert hasattr(context, 'adc_readings'), "No ADC readings available"
    
    mean_value = statistics.mean(context.adc_readings)
    outliers = []
    
    for i, reading in enumerate(context.adc_readings):
        if abs(reading - mean_value) > tolerance:
            outliers.append((i, reading))
    
    assert len(outliers) == 0, \
           f"{len(outliers)} readings outside ±{tolerance} counts of mean {mean_value:.1f}"
    context.hil_logger.test_pass(f"All {len(context.adc_readings)} readings within ±{tolerance} counts")


@then('"{channel1}" should read approximately {voltage1:f}V')
def step_verify_channel1_voltage(context, channel1, voltage1):
    """Verify first channel voltage"""
    adc_value = context.hardware_interface.read_adc_channel(channel1)
    assert adc_value is not None, f"Failed to read ADC channel {channel1}"
    
    actual_voltage = context.hardware_interface.adc_to_voltage(adc_value)
    tolerance = 0.1
    assert abs(actual_voltage - voltage1) <= tolerance, \
           f"Channel {channel1} voltage {actual_voltage:.3f}V not within {tolerance}V of {voltage1}V"
    context.hil_logger.measurement(f"Channel {channel1} voltage", actual_voltage, "V", f"{voltage1}V")


@then('the other channel should read approximately {voltage2:f}V')
def step_verify_channel2_voltage(context, voltage2):
    """Verify second channel voltage"""
    channel2 = context.channel2
    adc_value = context.hardware_interface.read_adc_channel(channel2)
    assert adc_value is not None, f"Failed to read ADC channel {channel2}"
    
    actual_voltage = context.hardware_interface.adc_to_voltage(adc_value)
    tolerance = 0.1
    assert abs(actual_voltage - voltage2) <= tolerance, \
           f"Channel {channel2} voltage {actual_voltage:.3f}V not within {tolerance}V of {voltage2}V"
    context.hil_logger.measurement(f"Channel {channel2} voltage", actual_voltage, "V", f"{voltage2}V")


@then('neither channel should affect the other')
def step_verify_channel_independence(context):
    """Verify ADC channels are independent"""
    # Re-read both channels to ensure independence
    channel1_value = context.hardware_interface.read_adc_channel(context.channel1)
    channel2_value = context.hardware_interface.read_adc_channel(context.channel2)
    
    assert channel1_value is not None, f"Channel {context.channel1} affected by other channel"
    assert channel2_value is not None, f"Channel {context.channel2} affected by other channel"
    
    # Voltages should still match expected values
    voltage1 = context.hardware_interface.adc_to_voltage(channel1_value)
    voltage2 = context.hardware_interface.adc_to_voltage(channel2_value)
    
    tolerance = 0.1
    assert abs(voltage1 - context.voltage1) <= tolerance, "Channel 1 affected by channel 2"
    assert abs(voltage2 - context.voltage2) <= tolerance, "Channel 2 affected by channel 1"
    
    context.hil_logger.test_pass("ADC channels are independent")


@then('the HIL framework should measure approximately {expected_duty:d}% duty cycle')
def step_verify_measured_duty_cycle(context, expected_duty):
    """Verify measured PWM duty cycle"""
    pwm_data = context.hardware_interface.measure_pwm_output(context.pwm_pin)
    assert pwm_data is not None, f"Failed to measure PWM on {context.pwm_pin}"
    
    measured_duty = pwm_data['duty_cycle']
    tolerance = 2  # ±2% tolerance
    assert abs(measured_duty - expected_duty) <= tolerance, \
           f"Measured duty cycle {measured_duty:.1f}% not within {tolerance}% of {expected_duty}%"
    context.hil_logger.measurement("PWM duty cycle", measured_duty, "%", f"{expected_duty}% ±{tolerance}%")


@then('the frequency should be approximately {expected_freq:d}Hz ± {tolerance:d}Hz')
def step_verify_measured_frequency_with_tolerance(context, expected_freq, tolerance):
    """Verify measured PWM frequency with tolerance"""
    pwm_data = context.hardware_interface.measure_pwm_output(context.pwm_pin)
    assert pwm_data is not None, f"Failed to measure PWM on {context.pwm_pin}"
    
    measured_freq = pwm_data['frequency']
    assert abs(measured_freq - expected_freq) <= tolerance, \
           f"Measured frequency {measured_freq:.1f}Hz not within {tolerance}Hz of {expected_freq}Hz"
    context.hil_logger.measurement("PWM frequency", measured_freq, "Hz", f"{expected_freq}Hz ±{tolerance}Hz")


@then('the measured duty cycle should be {expected_duty:d}% ± {tolerance:d}%')
def step_verify_duty_cycle_with_tolerance(context, expected_duty, tolerance):
    """Verify duty cycle with specified tolerance"""
    pwm_data = context.hardware_interface.measure_pwm_output(context.pwm_pin)
    assert pwm_data is not None, f"Failed to measure PWM on {context.pwm_pin}"
    
    measured_duty = pwm_data['duty_cycle']
    assert abs(measured_duty - expected_duty) <= tolerance, \
           f"Measured duty cycle {measured_duty:.1f}% not within {tolerance}% of {expected_duty}%"
    context.hil_logger.measurement("PWM duty cycle", measured_duty, "%", f"{expected_duty}% ±{tolerance}%")


@then('the measured frequency should be {expected_freq:d}Hz ± {tolerance:d}Hz')
def step_verify_frequency_with_tolerance(context, expected_freq, tolerance):
    """Verify frequency with specified tolerance"""
    step_verify_measured_frequency_with_tolerance(context, expected_freq, tolerance)


@then('the average voltage should be approximately {expected:f}V')
def step_verify_pwm_average_voltage(context, expected):
    """Verify PWM average voltage"""
    # For PWM, average voltage = duty_cycle * Vcc
    adc_value = context.hardware_interface.read_adc_channel(context.pwm_pin)
    assert adc_value is not None, f"Failed to read PWM voltage on {context.pwm_pin}"
    
    actual_voltage = context.hardware_interface.adc_to_voltage(adc_value)
    tolerance = 0.2
    assert abs(actual_voltage - expected) <= tolerance, \
           f"PWM average voltage {actual_voltage:.3f}V not within {tolerance}V of {expected}V"
    context.hil_logger.measurement("PWM average voltage", actual_voltage, "V", f"{expected}V ±{tolerance}V")


@then('the duty cycle should remain stable within ±{tolerance:d}%')
def step_verify_pwm_duty_stability(context, tolerance):
    """Verify PWM duty cycle stability"""
    assert hasattr(context, 'pwm_measurements'), "No PWM measurements available"
    
    duty_cycles = [m['duty_cycle'] for m in context.pwm_measurements]
    mean_duty = statistics.mean(duty_cycles)
    
    outliers = []
    for i, duty in enumerate(duty_cycles):
        if abs(duty - mean_duty) > tolerance:
            outliers.append((i, duty))
    
    assert len(outliers) == 0, \
           f"{len(outliers)} duty cycle measurements outside ±{tolerance}% of mean {mean_duty:.1f}%"
    context.hil_logger.test_pass(f"PWM duty cycle stable within ±{tolerance}%")


@then('the frequency should remain stable within ±{tolerance:d}Hz')
def step_verify_pwm_frequency_stability(context, tolerance):
    """Verify PWM frequency stability"""
    assert hasattr(context, 'pwm_measurements'), "No PWM measurements available"
    
    frequencies = [m['frequency'] for m in context.pwm_measurements]
    mean_freq = statistics.mean(frequencies)
    
    outliers = []
    for i, freq in enumerate(frequencies):
        if abs(freq - mean_freq) > tolerance:
            outliers.append((i, freq))
    
    assert len(outliers) == 0, \
           f"{len(outliers)} frequency measurements outside ±{tolerance}Hz of mean {mean_freq:.1f}Hz"
    context.hil_logger.test_pass(f"PWM frequency stable within ±{tolerance}Hz")


@then('I should measure PWM signal')
def step_should_measure_pwm_signal(context):
    """Verify PWM signal can be measured"""
    pwm_data = context.hardware_interface.measure_pwm_output(context.pwm_pin)
    assert pwm_data is not None, f"No PWM signal measured on {context.pwm_pin}"
    assert pwm_data['frequency'] > 0, "PWM frequency is zero"
    context.hil_logger.test_pass(f"PWM signal measured: {pwm_data['duty_cycle']:.1f}% @ {pwm_data['frequency']:.1f}Hz")


@then('the pin should be in a static state')
def step_verify_pin_static_state(context):
    """Verify pin is in static state (no PWM)"""
    # Try to measure PWM - should get no signal or error
    pwm_data = context.hardware_interface.measure_pwm_output(context.pwm_pin)
    if pwm_data is not None:
        # If we get data, frequency should be 0 or very low
        assert pwm_data['frequency'] < 10, f"Pin still generating PWM: {pwm_data['frequency']}Hz"
    
    context.hil_logger.test_pass("Pin in static state (no PWM)")


@then('I should measure the same PWM signal again')
def step_verify_same_pwm_signal_restored(context):
    """Verify same PWM signal is restored"""
    pwm_data = context.hardware_interface.measure_pwm_output(context.pwm_pin)
    assert pwm_data is not None, f"No PWM signal after re-enable on {context.pwm_pin}"
    
    # Check duty cycle and frequency match expected values
    duty_tolerance = 2
    freq_tolerance = 50
    
    assert abs(pwm_data['duty_cycle'] - context.pwm_duty) <= duty_tolerance, \
           f"Restored duty cycle {pwm_data['duty_cycle']:.1f}% differs from expected {context.pwm_duty}%"
    assert abs(pwm_data['frequency'] - context.pwm_frequency) <= freq_tolerance, \
           f"Restored frequency {pwm_data['frequency']:.1f}Hz differs from expected {context.pwm_frequency}Hz"
    
    context.hil_logger.test_pass("Same PWM signal restored after re-enable")
