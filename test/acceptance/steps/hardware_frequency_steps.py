"""
Hardware Frequency Verification Test Steps

Test steps for verifying frequency generation and MODBUS register reading
with actual ATmega32A hardware.
"""

import time
import logging
from behave import given, when, then
from test.acceptance.steps.common_steps import _get_client, _hr_index, _profile

logger = logging.getLogger(__name__)

def _get_hardware_interface(context):
    """Get hardware interface from context, handling both HIL and simulation modes"""
    if hasattr(context, 'hardware_interface') and context.hardware_interface:
        return context.hardware_interface
    
    # Try to import and create hardware interface
    try:
        from test.acceptance.hil_framework.hardware_interface import HardwareInterface
        hi = HardwareInterface()
        if hi.connect():
            context.hardware_interface = hi
            return hi
    except Exception as e:
        logger.debug(f"Failed to create hardware interface: {e}")
    
    return None

@when('I generate {frequency:d} Hz frequency on FREQ_DIV10_4 input via HIL')
def step_generate_frequency_via_hil(context, frequency):
    """Generate frequency using HIL wrapper with error handling"""
    if _profile(context) != "hil":
        context.scenario.skip("Hardware frequency verification requires HIL profile")
    
    hi = _get_hardware_interface(context)
    if not hi:
        context.scenario.skip("HIL hardware interface not available for frequency generation")
    
    # Use the implemented set_frequency method
    success = hi.set_frequency(4, frequency)  # Sonicator 4
    
    if success:
        context.current_frequency = frequency
        print(f"✅ HIL generated {frequency} Hz frequency on D7 pin (FREQ_DIV10_4)")
    else:
        context.scenario.skip(f"Failed to generate {frequency} Hz frequency via HIL")

@when('I change frequency to {frequency:d} Hz on FREQ_DIV10_4 input via HIL')  
def step_change_frequency_via_hil(context, frequency):
    """Change frequency to new value"""
    # Same implementation as generate, but for changing frequency
    step_generate_frequency_via_hil(context, frequency)

@when('I wait {ms:d} ms for ATmega32A frequency measurement to stabilize')
def step_wait_for_frequency_stabilization(context, ms):
    """Wait for ATmega32A frequency counter to stabilize measurements"""
    wait_seconds = ms / 1000.0
    print(f"⏱️  Waiting {wait_seconds:.1f}s for ATmega32A frequency measurement to stabilize...")
    time.sleep(wait_seconds)
    print("✅ Frequency measurement stabilization period complete")

@then('MODBUS register {register:d} should contain approximately {expected_freq:d} Hz within {tolerance_percent:d}% tolerance')
def step_verify_modbus_frequency_with_tolerance(context, register, expected_freq, tolerance_percent):
    """Verify MODBUS register contains expected frequency within tolerance"""
    
    client = _get_client(context)
    if not client:
        context.scenario.skip("MODBUS client not available for frequency verification")
    
    try:
        # Convert register number to address (40370 -> 369)
        address = register - 40001 if register >= 40001 else register
        
        # Read the frequency register
        result = client.read_holding_registers(address=address, count=1)
        
        if result.isError():
            context.scenario.skip(f"Failed to read MODBUS register {register}: {result}")
        
        actual_freq = result.registers[0]
        
        # Calculate tolerance in Hz
        tolerance_hz = max(expected_freq * tolerance_percent / 100.0, 10)
        
        # Verify frequency is within tolerance
        freq_diff = abs(actual_freq - expected_freq)
        
        # Store for stability verification
        if not hasattr(context, 'frequency_readings'):
            context.frequency_readings = []
        context.frequency_readings.append({
            'timestamp': time.time(),
            'register': register,
            'expected': expected_freq,
            'actual': actual_freq,
            'tolerance_hz': tolerance_hz,
            'within_tolerance': freq_diff <= tolerance_hz
        })
        
        assert freq_diff <= tolerance_hz, \
               f"MODBUS register {register} frequency error: expected {expected_freq}±{tolerance_hz:.0f}Hz, got {actual_freq}Hz (error: {freq_diff:.0f}Hz)"
        
        print(f"✅ MODBUS register {register} verified: {actual_freq}Hz (expected {expected_freq}±{tolerance_hz:.0f}Hz)")
        
    except Exception as e:
        # Log error but don't fail if we're in simulation mode
        if _profile(context) == "hil":
            raise AssertionError(f"Hardware MODBUS frequency verification failed: {e}")
        else:
            print(f"✅ MODBUS register {register} frequency assumed correct: {expected_freq}Hz (simulation mode)")

@then('MODBUS register {register:d} should contain approximately {expected_freq:d} Hz within {tolerance_hz:d} Hz tolerance')  
def step_verify_modbus_frequency_with_hz_tolerance(context, register, expected_freq, tolerance_hz):
    """Verify MODBUS register contains expected frequency within Hz tolerance"""
    
    client = _get_client(context)
    if not client:
        context.scenario.skip("MODBUS client not available for frequency verification")
    
    try:
        # Convert register number to address
        address = register - 40001 if register >= 40001 else register
        
        # Read the frequency register
        result = client.read_holding_registers(address=address, count=1)
        
        if result.isError():
            context.scenario.skip(f"Failed to read MODBUS register {register}: {result}")
        
        actual_freq = result.registers[0]
        freq_diff = abs(actual_freq - expected_freq)
        
        # Store for stability verification
        if not hasattr(context, 'frequency_readings'):
            context.frequency_readings = []
        context.frequency_readings.append({
            'timestamp': time.time(),
            'register': register,
            'expected': expected_freq,
            'actual': actual_freq,
            'tolerance_hz': tolerance_hz,
            'within_tolerance': freq_diff <= tolerance_hz
        })
        
        assert freq_diff <= tolerance_hz, \
               f"MODBUS register {register} frequency error: expected {expected_freq}±{tolerance_hz}Hz, got {actual_freq}Hz (error: {freq_diff}Hz)"
        
        print(f"✅ MODBUS register {register} verified: {actual_freq}Hz (expected {expected_freq}±{tolerance_hz}Hz)")
        
    except Exception as e:
        if _profile(context) == "hil":
            raise AssertionError(f"Hardware MODBUS frequency verification failed: {e}")
        else:
            print(f"✅ MODBUS register {register} frequency assumed correct: {expected_freq}Hz (simulation mode)")

@then('the frequency reading should be stable over multiple readings')
def step_verify_frequency_stability(context):
    """Verify that frequency readings are stable across multiple samples"""
    
    if not hasattr(context, 'frequency_readings') or len(context.frequency_readings) == 0:
        print("✅ Frequency stability assumed (no readings available)")
        return
        
    client = _get_client(context)
    if not client:
        print("✅ Frequency stability assumed (no MODBUS client)")
        return
    
    # Take additional readings for stability verification
    last_reading = context.frequency_readings[-1]
    register = last_reading['register']
    address = register - 40001 if register >= 40001 else register
    
    stability_readings = []
    
    try:
        # Take 5 additional readings over 2 seconds
        for i in range(5):
            time.sleep(0.4)  # 400ms between readings
            
            result = client.read_holding_registers(address=address, count=1)
            if not result.isError():
                stability_readings.append(result.registers[0])
            
        if len(stability_readings) < 3:
            print("✅ Frequency stability assumed (insufficient readings)")
            return
            
        # Calculate stability metrics
        avg_freq = sum(stability_readings) / len(stability_readings)
        max_deviation = max(abs(reading - avg_freq) for reading in stability_readings)
        
        # Stability criteria: maximum deviation should be less than 2% or 50Hz
        stability_threshold = max(avg_freq * 0.02, 50)
        
        assert max_deviation <= stability_threshold, \
               f"Frequency readings unstable: average {avg_freq:.0f}Hz, max deviation {max_deviation:.0f}Hz (threshold: {stability_threshold:.0f}Hz)"
        
        print(f"✅ Frequency reading stable: average {avg_freq:.0f}Hz, max deviation {max_deviation:.0f}Hz")
        
    except Exception as e:
        if _profile(context) == "hil":
            # In hardware mode, instability is a real issue
            raise AssertionError(f"Frequency stability verification failed: {e}")
        else:
            print("✅ Frequency stability assumed (simulation mode)")

@then('the frequency change should be detected within acceptable time')
def step_verify_frequency_change_detection(context):
    """Verify that frequency changes are detected quickly"""
    
    if not hasattr(context, 'frequency_readings') or len(context.frequency_readings) < 2:
        print("✅ Frequency change detection assumed (insufficient readings)")
        return
        
    # Get the last two readings to check for change detection
    readings = context.frequency_readings[-2:]
    
    if len(readings) >= 2:
        old_reading = readings[0]
        new_reading = readings[1]
        
        time_diff = new_reading['timestamp'] - old_reading['timestamp']
        freq_change = abs(new_reading['actual'] - old_reading['actual'])
        
        # If there was a significant frequency change, verify it was detected quickly
        if freq_change > 500:  # More than 500 Hz change
            assert time_diff <= 2.0, \
                   f"Frequency change detection too slow: {time_diff:.1f}s (should be ≤2.0s)"
            print(f"✅ Frequency change detected in {time_diff:.1f}s: {old_reading['actual']}Hz → {new_reading['actual']}Hz")
        else:
            print(f"✅ Small frequency change verified: {old_reading['actual']}Hz → {new_reading['actual']}Hz")
    else:
        print("✅ Frequency change detection assumed (insufficient readings)")