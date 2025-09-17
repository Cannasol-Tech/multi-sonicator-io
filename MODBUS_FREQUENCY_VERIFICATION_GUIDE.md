# MODBUS Frequency Verification Implementation Guide

## Overview

This document provides implementation suggestions for verifying frequency generation and reading functionality by reading the MODBUS register for FREQ_DIV10_4 in the actual ATmega32A hardware.

## Current Implementation Status

### ✅ Already Implemented
- **Arduino HIL Wrapper**: Generates actual frequency ticks on D7 pin (FREQ_DIV10_4)
- **DUT Frequency Counter**: ATmega32A measures frequency via `frequency_calculate()`
- **MODBUS Register Mapping**: Register 0x0171 (40370) contains `sonicator_status_t.frequency_hz`
- **Test Infrastructure**: MODBUS client setup and register reading capabilities

### 🎯 Target Implementation
Verify the complete frequency pipeline by:
1. HIL generates frequency → Arduino D7 pin
2. ATmega32A measures frequency → Updates register 40370
3. Test reads MODBUS register → Verifies correct frequency value

## Technical Specifications

### MODBUS Register Details
```c
// Register Definition
REG_SON4_FREQ_DIV10_HZ = 0x0171  // Sonicator 4 frequency register
MODBUS Address = 40001 + 0x0171 = 40370

// Data Format
- Type: uint16_t
- Range: 0-65535 Hz
- Content: Actual measured frequency from frequency_calculate()
- Update: Via update_modbus_registers() in main loop
```

### Hardware Connection
```
Arduino HIL Wrapper (D7) → ATmega32A (PB0/FREQ_DIV10_4) → Frequency Counter → MODBUS Register 40370
```

## Implementation Options

### Option 1: Enhanced Integration Test (RECOMMENDED)

Create a comprehensive test that exercises the complete frequency pipeline with physical hardware verification.

**Advantages:**
- ✅ Tests actual hardware integration
- ✅ Verifies complete signal chain
- ✅ Uses existing test infrastructure
- ✅ Provides real-world validation

**Implementation:**
```python
@scenario @hil @hardware @frequency @critical
def test_frequency_modbus_verification():
    """
    Test frequency generation and MODBUS register verification
    with actual ATmega32A hardware.
    """
    # 1. Set up HIL wrapper connection
    # 2. Generate test frequency on Arduino D7
    # 3. Wait for ATmega32A frequency measurement
    # 4. Read MODBUS register 40370
    # 5. Verify frequency value within tolerance
```

### Option 2: Standalone MODBUS Reader Tool

Create a dedicated tool for reading and monitoring the frequency register.

**Advantages:**
- ✅ Can be used independently
- ✅ Real-time monitoring capability
- ✅ Debugging tool for development

**Implementation:**
```python
# Standalone frequency monitor tool
def monitor_frequency_register():
    """
    Continuously monitor MODBUS register 40370
    for frequency changes.
    """
```

### Option 3: Web UI Integration

Add frequency monitoring to the existing web UI for visual verification.

**Advantages:**
- ✅ Visual feedback
- ✅ Real-time display
- ✅ User-friendly interface

## Recommended Implementation Approach

### Phase 1: Enhanced Acceptance Test
1. **Create new acceptance test scenario** for frequency verification
2. **Use existing MODBUS infrastructure** in `common_steps.py`
3. **Add frequency-specific test steps** for register reading
4. **Implement tolerance-based verification** (±5% frequency tolerance)

### Phase 2: MODBUS Reader Utility
1. **Create standalone Python script** for frequency monitoring
2. **Add real-time display** of register values
3. **Include logging capability** for test data collection

### Phase 3: Documentation and Validation
1. **Document test procedures** for hardware verification
2. **Create test reports** showing successful verification
3. **Validate with multiple frequency values**

## Implementation Details

### Test Case Structure
```gherkin
@acceptance @hil @hardware @frequency @critical
Scenario: Hardware MODBUS Frequency Verification
  Given the Arduino HIL wrapper is connected to ATmega32A
  And the MODBUS RTU connection is established
  When I generate 2000 Hz frequency on FREQ_DIV10_4 input
  And I wait 1000 ms for frequency measurement to stabilize  
  Then MODBUS register 40370 should contain approximately 20000 Hz
  And the frequency reading should be stable within ±100 Hz
```

### Python Implementation Skeleton
```python
def verify_hardware_frequency_modbus(frequency_hz: int, tolerance_percent: float = 5.0):
    """
    Verify frequency generation and MODBUS register reading
    with actual ATmega32A hardware.
    
    Args:
        frequency_hz: Target frequency to generate (Hz)
        tolerance_percent: Acceptable tolerance percentage
        
    Returns:
        dict: Test results with measured vs expected values
    """
    
    results = {
        'target_frequency': frequency_hz,
        'expected_modbus_value': frequency_hz * 10,  # ÷10 scaling compensation
        'actual_modbus_value': None,
        'success': False,
        'error': None
    }
    
    try:
        # 1. Set up hardware interface
        hil = HardwareInterface()
        if not hil.connect():
            raise Exception("Failed to connect to HIL wrapper")
            
        # 2. Set up MODBUS client
        modbus_client = setup_modbus_client()
        if not modbus_client.connected:
            raise Exception("Failed to connect to ATmega32A via MODBUS")
            
        # 3. Generate frequency
        if not hil.set_frequency(4, frequency_hz):
            raise Exception(f"Failed to set frequency to {frequency_hz} Hz")
            
        # 4. Wait for measurement stabilization
        time.sleep(1.0)  # Allow frequency counter to measure
        
        # 5. Read MODBUS register 40370 (REG_SON4_FREQ_DIV10_HZ)
        register_addr = 40370 - 40001  # Convert to 0-based
        response = modbus_client.read_holding_registers(
            address=register_addr, 
            count=1
        )
        
        if response.isError():
            raise Exception(f"MODBUS read failed: {response}")
            
        # 6. Verify frequency value
        actual_value = response.registers[0]
        expected_value = frequency_hz * 10  # ATmega32A should read 10x the input
        
        tolerance = max(expected_value * tolerance_percent / 100.0, 10)
        
        results['actual_modbus_value'] = actual_value
        results['success'] = abs(actual_value - expected_value) <= tolerance
        
        if not results['success']:
            results['error'] = f"Frequency mismatch: expected {expected_value}±{tolerance} Hz, got {actual_value} Hz"
            
    except Exception as e:
        results['error'] = str(e)
        
    finally:
        # Clean up connections
        if 'hil' in locals():
            hil.disconnect()
        if 'modbus_client' in locals():
            modbus_client.close()
            
    return results
```

### Test Matrix
| Input Frequency (Hz) | Expected MODBUS Value (Hz) | Tolerance (Hz) | Test Purpose |
|---------------------|---------------------------|----------------|--------------|
| 0 | 0 | ±10 | Zero/Off state |
| 1800 | 18000 | ±900 | Low frequency range |
| 2000 | 20000 | ±1000 | Nominal frequency |
| 2200 | 22000 | ±1100 | High frequency range |

## File Modifications Required

### 1. New Acceptance Test File
**File:** `test/acceptance/features/hardware_frequency_verification.feature`
- Add new feature for hardware frequency verification
- Include multiple test scenarios with different frequencies

### 2. Enhanced Test Steps
**File:** `test/acceptance/steps/hardware_frequency_steps.py`
- Add MODBUS frequency verification steps
- Include timing and tolerance handling

### 3. Utility Script
**File:** `scripts/frequency_modbus_monitor.py`  
- Standalone frequency monitoring tool
- Real-time register reading and display

### 4. Configuration Updates
**File:** `config/hil_config.yaml`
- Add frequency verification timeouts
- Configure MODBUS connection parameters

## Success Criteria

### Hardware Verification ✅
- [ ] Arduino generates measurable frequency signal
- [ ] ATmega32A frequency counter detects signal
- [ ] MODBUS register 40370 updates with correct frequency
- [ ] Multiple frequency values tested successfully

### Test Integration ✅
- [ ] Acceptance test passes with physical hardware
- [ ] MODBUS communication established and verified
- [ ] Frequency tolerance validation working
- [ ] Error handling for connection failures

### Documentation ✅
- [ ] Test procedures documented
- [ ] Hardware setup instructions provided
- [ ] Troubleshooting guide created
- [ ] Results validation process defined

## Risk Mitigation

### Hardware Connection Issues
- **Risk:** MODBUS communication failure
- **Mitigation:** Robust connection retry logic, clear error messages

### Timing Issues  
- **Risk:** Frequency measurement not stabilized
- **Mitigation:** Configurable wait times, multiple reading verification

### Frequency Accuracy
- **Risk:** Measurement outside tolerance
- **Mitigation:** Realistic tolerance values, calibration procedures

## Next Steps

1. **Implement Phase 1**: Create enhanced acceptance test
2. **Test with Hardware**: Verify with actual ATmega32A board
3. **Document Results**: Create test report with screenshots
4. **Expand Test Coverage**: Add more frequency values and edge cases
5. **Create Monitoring Tool**: Standalone MODBUS frequency monitor

## Conclusion

The implementation will provide comprehensive verification of the frequency generation and reading pipeline using actual ATmega32A hardware. This addresses the comment requirement for verifying functionality through MODBUS register reading while building on the existing test infrastructure.

The recommended approach balances thorough testing with practical implementation, ensuring robust verification of the frequency measurement capabilities.