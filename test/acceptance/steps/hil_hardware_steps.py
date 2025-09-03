#!/usr/bin/env python3
"""
HIL Hard    print("✅ ATmega32A programmed with firmware")are Step Definitions - Core hardware interaction steps for BDD scenarios

This module provides step definitions for basic hardware interactions including
GPIO control, ADC measurements, and hardware state verification.

Author: Cannasol Technologies  
License: Proprietary
"""

import time
from behave import given, when, then

# HIL hardware interaction steps - hardware_interface provided by environment.py


@given('the HIL framework is connected to ATmega32A')
def step_hil_framework_connected(context):
    """Verify HIL framework hardware connection"""
    assert hasattr(context, 'hardware_interface') and context.hardware_interface, "HIL framework not available"
    assert context.hardware_ready, "HIL hardware not ready"
    print("✅ HIL framework connected to ATmega32A")


@given('the ATmega32A is programmed with test firmware')
def step_program_atmega_firmware(context):
    """Program ATmega32A with test firmware via HIL framework"""
    if not hasattr(context, 'hil_controller'):
        assert False, "HIL controller not available"
        
    # Default test firmware - can be overridden by scenario
    firmware_path = getattr(context, 'test_firmware_path', 'test_firmware.hex')
    success = context.hil_controller.program_firmware(firmware_path)
    assert success, f"Failed to program ATmega32A with firmware: {firmware_path}"
    print("✅ ATmega32A programmed with firmware")


@given('the GPIO HAL is loaded on the target')
def step_gpio_hal_loaded(context):
    """Verify GPIO HAL is loaded and operational"""
    # Send command to verify GPIO subsystem
    response = context.hardware_interface.send_command("PING")
    assert response and "PONG" in response, f"GPIO HAL not responding: {response}"
    print("✅ GPIO HAL loaded on target")


@given('pin "{pin}" is configured as input with pull-up')
def step_configure_pin_input_pullup(context, pin):
    """Configure pin as input with pull-up resistor"""
    response = context.hardware_interface.send_command(f"CONFIG_PIN {pin} INPUT_PULLUP")
    assert response and "OK" in response, f"Failed to configure {pin} as input with pull-up"
    print(f"✅ Pin {pin} configured as input with pull-up")


@given('pin "{pin}" is configured as input')
def step_configure_pin_input(context, pin):
    """Configure pin as input"""
    response = context.hardware_interface.send_command(f"CONFIG_PIN {pin} INPUT")
    assert response and "OK" in response, f"Failed to configure {pin} as input"
    print(f"✅ Pin {pin} configured as input")


@when('I set pin "{pin}" to {state}')
def step_set_pin_state(context, pin, state):
    """Set GPIO pin state via HIL framework"""
    pin_state = True if state.upper() == 'HIGH' else False
    success = context.hardware_interface.write_gpio_pin(pin, pin_state)
    assert success, f"Failed to set pin {pin} to {state}"
    
    context.last_pin_set = (pin, pin_state)
    print(f"✅ Pin {pin} set to {state}")


@when('the HIL framework applies {voltage}V to pin "{pin}" externally')
def step_apply_external_voltage(context, voltage, pin):
    """Apply external voltage to pin via HIL framework"""
    # This would be implemented by the Arduino Test Harness
    # For now, simulate by sending command
    response = context.hardware_interface.send_command(f"APPLY_VOLTAGE {pin} {voltage}")
    assert response and "OK" in response, f"Failed to apply {voltage}V to {pin}"
    print(f"✅ Applied {voltage}V to pin {pin}")


@when('the HIL framework applies {voltage}V to pin "{pin}"')
def step_apply_voltage_to_pin(context, voltage, pin):
    """Apply voltage to pin via HIL framework"""
    step_apply_external_voltage(context, voltage, pin)


@when('the HIL framework disconnects the external connection')
def step_disconnect_external_connection(context):
    """Disconnect external connection"""
    response = context.hardware_interface.send_command("DISCONNECT_EXTERNAL")
    assert response and "OK" in response, "Failed to disconnect external connection"
    print("✅ External connection disconnected")


@when('I wait for {seconds:d} seconds')
def step_wait_seconds(context, seconds):
    """Wait for specified number of seconds"""
    print(f"✅ Waiting for {seconds} seconds")
    time.sleep(seconds)


@then('the HIL framework should read pin "{pin}" as {state}')
def step_verify_pin_state(context, pin, state):
    """Verify GPIO pin state via HIL framework"""
    expected_state = True if state.upper() == 'HIGH' else False
    actual_state = context.hardware_interface.read_gpio_pin(pin)
    assert actual_state == expected_state, \
           f"Pin {pin} expected {state}, got {'HIGH' if actual_state else 'LOW'}"
    print(f"📊 Pin {pin} state", state)


@then('reading pin "{pin}" should return {state}')
def step_read_pin_should_return(context, pin, state):
    """Verify pin reading returns expected state"""
    step_verify_pin_state(context, pin, state)


@then('the pin voltage should be approximately {voltage:f}V')
def step_verify_pin_voltage(context, voltage):
    """Verify pin voltage level via HIL framework"""
    if not hasattr(context, 'last_pin_set'):
        assert False, "No pin was previously set"
        
    pin, _ = context.last_pin_set
    adc_value = context.hardware_interface.read_adc_channel(pin)
    assert adc_value is not None, f"Failed to read ADC for pin {pin}"
    
    actual_voltage = context.hardware_interface.adc_to_voltage(adc_value)
    tolerance = 0.2  # ±0.2V tolerance
    
    assert abs(actual_voltage - voltage) <= tolerance, \
           f"Pin {pin} voltage {actual_voltage:.3f}V not within {tolerance}V of {voltage}V"
           
    print(f"📊 Pin {pin} voltage", actual_voltage, "V", f"{voltage}V ±{tolerance}V")


@then('the voltage on pin "{pin}" should be approximately {voltage:f}V')
def step_verify_named_pin_voltage(context, pin, voltage):
    """Verify voltage on named pin"""
    adc_value = context.hardware_interface.read_adc_channel(pin)
    assert adc_value is not None, f"Failed to read ADC for pin {pin}"
    
    actual_voltage = context.hardware_interface.adc_to_voltage(adc_value)
    tolerance = 0.2  # ±0.2V tolerance
    
    assert abs(actual_voltage - voltage) <= tolerance, \
           f"Pin {pin} voltage {actual_voltage:.3f}V not within {tolerance}V of {voltage}V"
           
    print(f"📊 Pin {pin} voltage", actual_voltage, "V", f"{voltage}V ±{tolerance}V")


@then('pin "{pin}" should still be {state}')
def step_verify_pin_state_persistence(context, pin, state):
    """Verify pin state has persisted"""
    expected_state = True if state.upper() == 'HIGH' else False
    actual_state = context.hardware_interface.read_gpio_pin(pin)
    assert actual_state == expected_state, \
           f"Pin {pin} state not persistent - expected {state}, got {'HIGH' if actual_state else 'LOW'}"
    print(f"📊 Pin {pin} persistent state", state)


@then('the firmware should read pin "{pin}" as {state}')
def step_firmware_should_read_pin(context, pin, state):
    """Verify firmware reads pin correctly"""
    # Query firmware for pin state
    response = context.hardware_interface.send_command(f"READ_PIN_FIRMWARE {pin}")
    assert response, f"No response from firmware for pin {pin}"
    
    expected_state = state.upper()
    assert expected_state in response.upper(), \
           f"Firmware read of pin {pin} expected {state}, got {response}"
    print(f"📊 Firmware pin {pin} reading", state)


@then('the pin state should be reported correctly via serial')
def step_verify_serial_pin_report(context):
    """Verify pin state is reported correctly via serial"""
    if not hasattr(context, 'last_pin_set'):
        assert False, "No pin was previously set"
        
    pin, expected_state = context.last_pin_set
    response = context.hardware_interface.send_command(f"REPORT_PIN {pin}")
    assert response, f"No serial response for pin {pin}"
    
    expected_str = "HIGH" if expected_state else "LOW"
    assert expected_str in response.upper(), \
           f"Pin {pin} serial report expected {expected_str}, got {response}"
    print(f"📊 Pin {pin} serial report", expected_str)


@then('the status LED should be visible on the hardware')
def step_verify_status_led_visible(context):
    """Verify status LED is visible (manual verification)"""
    # This step requires manual verification or camera-based verification
    # For automated testing, we verify the electrical signal
    led_state = context.hardware_interface.read_gpio_pin("STATUS_LED")
    assert led_state == True, "Status LED pin is not HIGH"
    print("✅ Status LED verified as visible")


@then('the status LED should be off')
def step_verify_status_led_off(context):
    """Verify status LED is off"""
    led_state = context.hardware_interface.read_gpio_pin("STATUS_LED")
    assert led_state == False, "Status LED pin is not LOW"
    print("✅ Status LED verified as off")
