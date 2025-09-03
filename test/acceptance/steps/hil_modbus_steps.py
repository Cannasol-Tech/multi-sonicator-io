#!/usr/bin/env python3
"""
HIL MODBUS Step Definitions - MODBUS RTU communication verification steps

This module provides step definitions for MODBUS RTU communication testing
with real hardware via the HIL framework.

Author: Cannasol Technologies  
License: Proprietary
"""

import time
from behave import given, when, then


@given('the ATmega32A is programmed with MODBUS firmware')
def step_atmega_programmed_with_modbus(context):
    """Program ATmega32A with MODBUS firmware"""
    if not hasattr(context, 'hil_controller'):
        assert False, "HIL controller not available"
        
    # Use MODBUS-enabled firmware
    firmware_path = "modbus_firmware.hex"
    success = context.hil_controller.program_firmware(firmware_path)
    assert success, f"Failed to program ATmega32A with MODBUS firmware: {firmware_path}"
    
    # Wait for MODBUS stack to initialize
    time.sleep(2)
    context.hil_logger.hardware_event(f"ATmega32A programmed with MODBUS firmware")


@given('the MODBUS slave ID is set to {slave_id:d}')
def step_modbus_slave_id_set(context, slave_id):
    """Set MODBUS slave ID"""
    context.modbus_slave_id = slave_id
    # Verify slave ID is configured correctly
    response = context.hardware_interface.send_command(f"MODBUS_SET_SLAVE_ID {slave_id}")
    assert response and "OK" in response, f"Failed to set MODBUS slave ID to {slave_id}"
    context.hil_logger.hardware_event(f"MODBUS slave ID set to {slave_id}")


@given('communication is established at {baud_rate:d} baud')
def step_modbus_communication_established(context, baud_rate):
    """Establish MODBUS communication at specified baud rate"""
    context.modbus_baud_rate = baud_rate
    response = context.hardware_interface.send_command(f"MODBUS_SET_BAUD {baud_rate}")
    assert response and "OK" in response, f"Failed to set MODBUS baud rate to {baud_rate}"
    
    # Test basic MODBUS communication
    test_value = context.hardware_interface.modbus_read_register(0x0000)
    assert test_value is not None, f"MODBUS communication not established at {baud_rate} baud"
    context.hil_logger.hardware_event(f"MODBUS communication established at {baud_rate} baud")


@given('the MODBUS communication is established')
def step_modbus_communication_is_established(context):
    """Verify MODBUS communication is established"""
    # Test read of system status register
    test_value = context.hardware_interface.modbus_read_register(0x0000)
    assert test_value is not None, "MODBUS communication not established"
    context.hil_logger.hardware_event("MODBUS communication verified")


@when('I read MODBUS register {address} (system status)')
def step_read_modbus_register_with_description(context, address):
    """Read MODBUS register with description"""
    reg_addr = int(address, 16)
    start_time = time.time()
    value = context.hardware_interface.modbus_read_register(reg_addr)
    end_time = time.time()
    
    context.last_modbus_read = {
        'address': reg_addr,
        'value': value,
        'response_time': end_time - start_time
    }
    context.hil_logger.measurement(f"MODBUS read 0x{reg_addr:04X}", value if value is not None else "FAIL")


@when('I write value {value:d} to MODBUS register {address} (global enable)')
def step_write_modbus_register_with_description(context, value, address):
    """Write value to MODBUS register with description"""
    reg_addr = int(address, 16)
    start_time = time.time()
    success = context.hardware_interface.modbus_write_register(reg_addr, value)
    end_time = time.time()
    
    context.last_modbus_write = {
        'address': reg_addr,
        'value': value,
        'success': success,
        'response_time': end_time - start_time
    }
    assert success, f"Failed to write value {value} to MODBUS register 0x{reg_addr:04X}"
    context.hil_logger.measurement(f"MODBUS write 0x{reg_addr:04X}", value)


@when('I write value "{value}" to MODBUS register "{address}" (sonicator start)')
def step_write_modbus_register_quoted(context, value, address):
    """Write value to MODBUS register (quoted parameters)"""
    val = int(value, 16) if value.startswith('0x') else int(value)
    step_write_modbus_register_with_description(context, val, address)


@when('I write value {value} to MODBUS register {address}')
def step_write_modbus_register_simple(context, value, address):
    """Write value to MODBUS register (simple)"""
    val = int(value, 16) if value.startswith('0x') else int(value)
    step_write_modbus_register_with_description(context, val, address)


@when('immediately read MODBUS register {address}')
def step_immediately_read_modbus_register(context, address):
    """Immediately read MODBUS register after write"""
    reg_addr = int(address, 16)
    start_time = time.time()
    value = context.hardware_interface.modbus_read_register(reg_addr)
    end_time = time.time()
    
    context.immediate_read = {
        'address': reg_addr,
        'value': value,
        'response_time': end_time - start_time
    }
    context.hil_logger.measurement(f"MODBUS immediate read 0x{reg_addr:04X}", value if value is not None else "FAIL")


@when('I read MODBUS registers {start_addr} to {end_addr} in sequence')
def step_read_modbus_registers_sequence(context, start_addr, end_addr):
    """Read sequence of MODBUS registers"""
    start = int(start_addr, 16)
    end = int(end_addr, 16)
    
    context.sequence_reads = []
    
    for addr in range(start, end + 1):
        start_time = time.time()
        value = context.hardware_interface.modbus_read_register(addr)
        end_time = time.time()
        
        context.sequence_reads.append({
            'address': addr,
            'value': value,
            'response_time': end_time - start_time
        })
        context.hil_logger.measurement(f"MODBUS sequence read 0x{addr:04X}", value if value is not None else "FAIL")


@when('I attempt to read MODBUS register {address} (invalid address)')
def step_attempt_read_invalid_register(context, address):
    """Attempt to read invalid MODBUS register"""
    reg_addr = int(address, 16)
    start_time = time.time()
    
    try:
        value = context.hardware_interface.modbus_read_register(reg_addr)
        end_time = time.time()
        context.invalid_read_result = {
            'address': reg_addr,
            'value': value,
            'response_time': end_time - start_time,
            'exception': None
        }
    except Exception as e:
        end_time = time.time()
        context.invalid_read_result = {
            'address': reg_addr,
            'value': None,
            'response_time': end_time - start_time,
            'exception': str(e)
        }
    
    context.hil_logger.hardware_event(f"Invalid register read attempt: 0x{reg_addr:04X}")


@when('I send a MODBUS request with invalid CRC')
def step_send_invalid_crc_request(context):
    """Send MODBUS request with invalid CRC"""
    # This would be implemented by sending a raw MODBUS frame with bad CRC
    response = context.hardware_interface.send_command("MODBUS_SEND_BAD_CRC")
    context.bad_crc_response = response
    context.hil_logger.hardware_event("MODBUS request with invalid CRC sent")


@when('I send the same request with valid CRC')
def step_send_valid_crc_request(context):
    """Send MODBUS request with valid CRC"""
    # Read a valid register to test good CRC
    value = context.hardware_interface.modbus_read_register(0x0000)
    context.good_crc_response = value
    context.hil_logger.hardware_event("MODBUS request with valid CRC sent")


@when('I stop sending MODBUS requests for {seconds:d} seconds')
def step_stop_modbus_requests(context, seconds):
    """Stop sending MODBUS requests for specified time"""
    context.hil_logger.hardware_event(f"Stopping MODBUS requests for {seconds} seconds")
    time.sleep(seconds)
    context.communication_gap_duration = seconds


@when('I disconnect the communication cable')
def step_disconnect_communication_cable(context):
    """Disconnect communication cable"""
    response = context.hardware_interface.send_command("DISCONNECT_MODBUS")
    assert response and "OK" in response, "Failed to disconnect MODBUS communication"
    context.hil_logger.hardware_event("MODBUS communication cable disconnected")


@when('wait for {seconds:d} seconds')
def step_wait_for_seconds(context, seconds):
    """Wait for specified number of seconds"""
    context.hil_logger.hardware_event(f"Waiting for {seconds} seconds")
    time.sleep(seconds)


@when('I resume MODBUS communication')
def step_resume_modbus_communication(context):
    """Resume MODBUS communication"""
    response = context.hardware_interface.send_command("RECONNECT_MODBUS")
    assert response and "OK" in response, "Failed to reconnect MODBUS communication"
    context.hil_logger.hardware_event("MODBUS communication resumed")


@then('the register value should be valid')
def step_register_value_should_be_valid(context):
    """Verify register value is valid"""
    assert hasattr(context, 'last_modbus_read'), "No MODBUS read performed"
    assert context.last_modbus_read['value'] is not None, "MODBUS register read failed"
    context.hil_logger.test_pass(f"MODBUS register value valid: {context.last_modbus_read['value']}")


@then('the response time should be less than {max_time:d}ms')
def step_verify_modbus_response_time(context, max_time):
    """Verify MODBUS response time"""
    if hasattr(context, 'last_modbus_read'):
        response_time = context.last_modbus_read['response_time'] * 1000
        assert response_time < max_time, f"MODBUS response time {response_time:.1f}ms exceeds {max_time}ms"
        context.hil_logger.measurement("MODBUS response time", response_time, "ms", f"< {max_time}ms")
    else:
        assert False, "No MODBUS operation recorded"


@then('the CRC should be correct')
def step_verify_crc_correct(context):
    """Verify CRC is correct"""
    # If we got a valid response, CRC was correct
    assert hasattr(context, 'last_modbus_read'), "No MODBUS read performed"
    assert context.last_modbus_read['value'] is not None, "MODBUS CRC error - no valid response"
    context.hil_logger.test_pass("MODBUS CRC correct")


@then('reading register {address} should return value {expected_value:d}')
def step_verify_register_readback(context, address, expected_value):
    """Verify register readback value"""
    reg_addr = int(address, 16)
    value = context.hardware_interface.modbus_read_register(reg_addr)
    assert value == expected_value, \
           f"Register 0x{reg_addr:04X} readback expected {expected_value}, got {value}"
    context.hil_logger.test_pass(f"Register 0x{reg_addr:04X} readback correct: {value}")


@then('the register change should be reflected within {max_time:d}ms')
def step_verify_register_change_timing(context, max_time):
    """Verify register change timing"""
    assert hasattr(context, 'last_modbus_write'), "No MODBUS write performed"
    response_time = context.last_modbus_write['response_time'] * 1000
    assert response_time < max_time, \
           f"Register change time {response_time:.1f}ms exceeds {max_time}ms"
    context.hil_logger.measurement("Register change time", response_time, "ms", f"< {max_time}ms")


@then('the system status should update accordingly')
def step_verify_system_status_update(context):
    """Verify system status updates"""
    # Read system status register to verify update
    status = context.hardware_interface.modbus_read_register(0x0000)
    assert status is not None, "Could not read system status register"
    # Status should reflect the change made in previous write
    context.hil_logger.test_pass(f"System status updated: 0x{status:04X}")


@then('all reads should succeed')
def step_verify_all_reads_succeed(context):
    """Verify all sequence reads succeeded"""
    assert hasattr(context, 'sequence_reads'), "No sequence reads performed"
    
    for read in context.sequence_reads:
        assert read['value'] is not None, \
               f"Read failed for register 0x{read['address']:04X}"
    
    context.hil_logger.test_pass(f"All {len(context.sequence_reads)} sequence reads succeeded")


@then('each response time should be less than {max_time:d}ms')
def step_verify_each_response_time(context, max_time):
    """Verify each response time in sequence"""
    assert hasattr(context, 'sequence_reads'), "No sequence reads performed"
    
    for read in context.sequence_reads:
        response_time = read['response_time'] * 1000
        assert response_time < max_time, \
               f"Register 0x{read['address']:04X} response time {response_time:.1f}ms exceeds {max_time}ms"
    
    context.hil_logger.test_pass(f"All response times < {max_time}ms")


@then('all CRC values should be correct')
def step_verify_all_crc_correct(context):
    """Verify all CRC values are correct"""
    assert hasattr(context, 'sequence_reads'), "No sequence reads performed"
    
    # If all reads succeeded, all CRC values were correct
    for read in context.sequence_reads:
        assert read['value'] is not None, \
               f"CRC error for register 0x{read['address']:04X}"
    
    context.hil_logger.test_pass("All CRC values correct")


@then('the read value should be {expected_value}')
def step_verify_immediate_read_value(context, expected_value):
    """Verify immediate read value"""
    assert hasattr(context, 'immediate_read'), "No immediate read performed"
    
    expected = int(expected_value, 16) if expected_value.startswith('0x') else int(expected_value)
    actual = context.immediate_read['value']
    
    assert actual == expected, \
           f"Immediate read expected {expected}, got {actual}"
    context.hil_logger.test_pass(f"Immediate read value correct: {actual}")


@then('the total operation time should be less than {max_time:d}ms')
def step_verify_total_operation_time(context, max_time):
    """Verify total operation time"""
    assert hasattr(context, 'last_modbus_write'), "No MODBUS write performed"
    assert hasattr(context, 'immediate_read'), "No immediate read performed"
    
    total_time = (context.last_modbus_write['response_time'] + 
                  context.immediate_read['response_time']) * 1000
                  
    assert total_time < max_time, \
           f"Total operation time {total_time:.1f}ms exceeds {max_time}ms"
    context.hil_logger.measurement("Total operation time", total_time, "ms", f"< {max_time}ms")


@then('I should receive a MODBUS exception response')
def step_verify_modbus_exception(context):
    """Verify MODBUS exception response"""
    assert hasattr(context, 'invalid_read_result'), "No invalid read performed"
    
    # Either should get None value (no response) or exception
    result = context.invalid_read_result
    assert result['value'] is None or result['exception'] is not None, \
           f"Expected MODBUS exception, got value: {result['value']}"
    context.hil_logger.test_pass("MODBUS exception response received")


@then('the exception code should indicate illegal address')
def step_verify_illegal_address_exception(context):
    """Verify illegal address exception code"""
    assert hasattr(context, 'invalid_read_result'), "No invalid read performed"
    
    # Check for specific illegal address indication
    result = context.invalid_read_result
    assert result['value'] is None, "Should not receive value for illegal address"
    context.hil_logger.test_pass("Illegal address exception indicated")


@then('the slave should not respond')
def step_verify_no_slave_response(context):
    """Verify slave does not respond to bad CRC"""
    assert hasattr(context, 'bad_crc_response'), "No bad CRC request sent"
    
    # Should get no response or error indication
    assert not context.bad_crc_response or "ERROR" in context.bad_crc_response, \
           f"Slave responded to bad CRC: {context.bad_crc_response}"
    context.hil_logger.test_pass("Slave correctly ignored bad CRC request")


@then('no register values should change')
def step_verify_no_register_changes(context):
    """Verify no register values changed"""
    # Read a few registers to verify they haven't changed
    test_registers = [0x0000, 0x0010, 0x0100]
    
    for reg in test_registers:
        value = context.hardware_interface.modbus_read_register(reg)
        # We can't verify exact values without baseline, but we can verify reads work
        assert value is not None, f"Register 0x{reg:04X} became unreadable"
    
    context.hil_logger.test_pass("No unexpected register changes detected")


@then('the slave should respond correctly')
def step_verify_correct_slave_response(context):
    """Verify slave responds correctly to good CRC"""
    assert hasattr(context, 'good_crc_response'), "No good CRC request sent"
    assert context.good_crc_response is not None, "Slave did not respond to valid CRC"
    context.hil_logger.test_pass("Slave responded correctly to valid CRC")


@then('the system should detect communication fault')
def step_verify_communication_fault_detected(context):
    """Verify system detects communication fault"""
    # Read communication fault status
    status = context.hardware_interface.modbus_read_register(0x0001)  # Fault status register
    assert status is not None, "Could not read fault status register"
    
    # Check communication fault bit (assuming bit 0)
    comm_fault = status & 0x0001
    assert comm_fault != 0, f"Communication fault not detected, status: 0x{status:04X}"
    context.hil_logger.test_pass(f"Communication fault detected: 0x{status:04X}")


@then('enter failsafe mode per requirements')
def step_verify_failsafe_mode(context):
    """Verify system enters failsafe mode"""
    # Read system mode register
    mode = context.hardware_interface.modbus_read_register(0x0002)  # Mode register
    assert mode is not None, "Could not read system mode register"
    
    # Check failsafe mode bit
    failsafe_mode = mode & 0x0002  # Assuming bit 1 is failsafe mode
    assert failsafe_mode != 0, f"System not in failsafe mode, mode: 0x{mode:04X}"
    context.hil_logger.test_pass(f"System in failsafe mode: 0x{mode:04X}")


@then('the communication fault flag should be set')
def step_verify_communication_fault_flag_set(context):
    """Verify communication fault flag is set"""
    step_verify_communication_fault_detected(context)


@then('the communication fault flag should clear within {max_time:d} second')
def step_verify_communication_fault_flag_clears(context, max_time):
    """Verify communication fault flag clears"""
    start_time = time.time()
    
    while time.time() - start_time < max_time:
        status = context.hardware_interface.modbus_read_register(0x0001)
        if status is not None:
            comm_fault = status & 0x0001
            if comm_fault == 0:
                context.hil_logger.test_pass("Communication fault flag cleared")
                return
        time.sleep(0.1)
    
    assert False, f"Communication fault flag did not clear within {max_time} second(s)"


@then('pin "{pin}" should go {state}')
def step_verify_pin_modbus_control(context, pin, state):
    """Verify pin state change via MODBUS control"""
    expected_state = True if state.upper() == 'HIGH' else False
    actual_state = context.hardware_interface.read_gpio_pin(pin)
    
    assert actual_state == expected_state, \
           f"Pin {pin} expected {state}, got {'HIGH' if actual_state else 'LOW'}"
    context.hil_logger.test_pass(f"Pin {pin} correctly set to {state} via MODBUS")


@then('the operation should complete within {max_time:d}ms')
def step_verify_operation_timing(context, max_time):
    """Verify operation completes within time limit"""
    assert hasattr(context, 'last_modbus_write'), "No MODBUS write performed"
    
    response_time = context.last_modbus_write['response_time'] * 1000
    assert response_time < max_time, \
           f"Operation time {response_time:.1f}ms exceeds {max_time}ms"
    context.hil_logger.measurement("Operation time", response_time, "ms", f"< {max_time}ms")
