#!/usr/bin/env python3
"""
HIL Connectivity Step Definitions - Hardware connectivity and communication steps

This module provides step definitions for hardware connectivity verification,
Arduino Test Harness communication, and system setup validation.

Author: Cannasol Technologies  
License: Proprietary
"""

import time
from behave import given, when, then


@given('the Arduino test harness is connected')
def step_arduino_harness_connected(context):
    """Verify Arduino test harness connection"""
    if not hasattr(context, 'hardware_interface'):
        assert False, "Hardware interface not available"
        
    # Verify Arduino Test Harness communication
    response = context.hardware_interface.send_command("PING")
    assert response and "PONG" in response, "Arduino Test Harness not responding"
    context.hil_logger.hardware_event("Arduino Test Harness connected")


@given('the ATmega32A target is powered')
def step_atmega_target_powered(context):
    """Verify ATmega32A target is powered"""
    # Check power supply status
    response = context.hardware_interface.send_command("CHECK_POWER")
    assert response and "POWER_OK" in response, "ATmega32A target not powered"
    context.hil_logger.hardware_event("ATmega32A target powered")


@given('the HIL framework is initialized')
def step_hil_framework_initialized(context):
    """Verify HIL framework is initialized"""
    assert hasattr(context, 'hil_controller'), "HIL controller not available"
    assert context.hardware_ready, "HIL hardware not ready"
    context.hil_logger.hardware_event("HIL framework initialized")


@when('I send a ping command to the Arduino Test Harness')
def step_send_ping_command(context):
    """Send ping command to Arduino Test Harness"""
    start_time = time.time()
    response = context.hardware_interface.send_command("PING")
    end_time = time.time()
    
    context.ping_response = response
    context.ping_time = end_time - start_time
    context.hil_logger.hardware_event(f"Ping command sent, response: {response}")


@when('I measure the power supply voltages')
def step_measure_power_supply_voltages(context):
    """Measure power supply voltages"""
    # Read power supply status from Arduino Test Harness
    response = context.hardware_interface.send_command("MEASURE_POWER")
    assert response, "No response from power measurement"
    
    # Parse response: "POWER 24.1V 5.02V 3.28V"
    parts = response.split()
    if len(parts) >= 4 and parts[0] == "POWER":
        context.input_24v = float(parts[1].replace('V', ''))
        context.rail_5v = float(parts[2].replace('V', ''))
        context.rail_3v3 = float(parts[3].replace('V', ''))
    else:
        assert False, f"Invalid power measurement response: {response}"
        
    context.hil_logger.measurement("24V input", context.input_24v, "V")
    context.hil_logger.measurement("5V rail", context.rail_5v, "V")
    context.hil_logger.measurement("3.3V rail", context.rail_3v3, "V")


@when('I attempt to program the ATmega32A via Arduino as ISP')
def step_attempt_program_atmega(context):
    """Attempt to program ATmega32A via Arduino as ISP"""
    if not hasattr(context, 'hil_controller'):
        assert False, "HIL controller not available"
        
    # Use test firmware
    firmware_path = "test_firmware.hex"
    context.programming_success = context.hil_controller.program_firmware(firmware_path)
    context.hil_logger.hardware_event(f"Programming attempt: {'success' if context.programming_success else 'failed'}")


@when('I establish serial communication with the target')
def step_establish_serial_communication(context):
    """Establish serial communication with target"""
    # Test serial communication
    response = context.hardware_interface.send_command("HELLO")
    context.serial_response = response
    context.hil_logger.hardware_event(f"Serial communication test: {response}")


@then('I should receive a PONG response')
def step_should_receive_pong(context):
    """Verify PONG response received"""
    assert hasattr(context, 'ping_response'), "No ping response recorded"
    assert context.ping_response and "PONG" in context.ping_response, \
           f"Expected PONG response, got: {context.ping_response}"
    context.hil_logger.test_pass("PONG response received")


@then('the response time should be less than {max_time:d}ms')
def step_verify_response_time(context, max_time):
    """Verify response time is within limit"""
    assert hasattr(context, 'ping_time'), "No ping time recorded"
    time_ms = context.ping_time * 1000
    assert time_ms < max_time, f"Response time {time_ms:.1f}ms exceeds {max_time}ms limit"
    context.hil_logger.measurement("Response time", time_ms, "ms", f"< {max_time}ms")


@then('the 24V input should be within tolerance')
def step_verify_24v_input_tolerance(context):
    """Verify 24V input is within tolerance"""
    assert hasattr(context, 'input_24v'), "No 24V measurement recorded"
    expected = 24.0
    tolerance = 0.5  # ±0.5V tolerance
    assert abs(context.input_24v - expected) <= tolerance, \
           f"24V input {context.input_24v}V not within {tolerance}V of {expected}V"
    context.hil_logger.test_pass(f"24V input within tolerance: {context.input_24v}V")


@then('the 5V rail should be {expected_voltage:f}V ± {tolerance:f}V')
def step_verify_5v_rail_tolerance(context, expected_voltage, tolerance):
    """Verify 5V rail is within tolerance"""
    assert hasattr(context, 'rail_5v'), "No 5V rail measurement recorded"
    assert abs(context.rail_5v - expected_voltage) <= tolerance, \
           f"5V rail {context.rail_5v}V not within {tolerance}V of {expected_voltage}V"
    context.hil_logger.test_pass(f"5V rail within tolerance: {context.rail_5v}V")


@then('the 3.3V rail should be {expected_voltage:f}V ± {tolerance:f}V')
def step_verify_3v3_rail_tolerance(context, expected_voltage, tolerance):
    """Verify 3.3V rail is within tolerance"""
    assert hasattr(context, 'rail_3v3'), "No 3.3V rail measurement recorded"
    assert abs(context.rail_3v3 - expected_voltage) <= tolerance, \
           f"3.3V rail {context.rail_3v3}V not within {tolerance}V of {expected_voltage}V"
    context.hil_logger.test_pass(f"3.3V rail within tolerance: {context.rail_3v3}V")


@then('the programming should succeed')
def step_programming_should_succeed(context):
    """Verify programming succeeded"""
    assert hasattr(context, 'programming_success'), "No programming attempt recorded"
    assert context.programming_success, "ATmega32A programming failed"
    context.hil_logger.test_pass("ATmega32A programming succeeded")


@then('the firmware version should be readable')
def step_firmware_version_readable(context):
    """Verify firmware version can be read"""
    if not hasattr(context, 'hil_controller'):
        assert False, "HIL controller not available"
        
    version = context.hil_controller.verify_firmware_version()
    assert version is not None, "Could not read firmware version"
    context.firmware_version = version
    context.hil_logger.measurement("Firmware version", version)


@then('the target should respond to commands')
def step_target_should_respond(context):
    """Verify target responds to commands"""
    response = context.hardware_interface.send_command("STATUS")
    assert response, "Target not responding to commands"
    context.hil_logger.test_pass(f"Target responding: {response}")


@then('I should receive a valid response')
def step_should_receive_valid_response(context):
    """Verify valid response received"""
    assert hasattr(context, 'serial_response'), "No serial response recorded"
    assert context.serial_response, "No response received via serial"
    context.hil_logger.test_pass(f"Valid serial response: {context.serial_response}")


@then('the communication should be stable for {duration:d} seconds')
def step_verify_communication_stability(context, duration):
    """Verify communication stability over time"""
    start_time = time.time()
    success_count = 0
    test_count = 0
    
    while time.time() - start_time < duration:
        response = context.hardware_interface.send_command("PING")
        test_count += 1
        if response and "PONG" in response:
            success_count += 1
        time.sleep(0.5)  # Test every 500ms
        
    success_rate = success_count / test_count if test_count > 0 else 0
    assert success_rate >= 0.95, f"Communication stability {success_rate*100:.1f}% below 95%"
    
    context.hil_logger.measurement("Communication stability", success_rate * 100, "%", ">= 95%")
    context.hil_logger.test_pass(f"Communication stable for {duration} seconds")
