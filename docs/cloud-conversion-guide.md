# Cloud Conversion Guide - Multi-Sonicator-IO Raw Data Processing

## Overview

The Multi-Sonicator-IO ATmega32A controller reports **raw sensor data** to MODBUS registers for maximum efficiency and flexibility. All unit conversions and scaling are performed in the cloud/PLC layer, allowing for:

- **Maximum ATmega32A Performance**: No floating-point math or complex calculations
- **Calibration Flexibility**: Adjust scaling factors without firmware updates  
- **Full Precision**: Raw ADC data preserves all measurement precision
- **Future-Proof**: Easy to add new conversion algorithms or calibration

## Raw Data Registers

### Power Monitoring Registers
| Register | Address | Raw Data Range | Description |
|----------|---------|----------------|-------------|
| REG_SON1_POWER_RAW_ADC | 0x0110 | 0-1023 | Sonicator 1 raw ADC power reading |
| REG_SON2_POWER_RAW_ADC | 0x0130 | 0-1023 | Sonicator 2 raw ADC power reading |
| REG_SON3_POWER_RAW_ADC | 0x0150 | 0-1023 | Sonicator 3 raw ADC power reading |
| REG_SON4_POWER_RAW_ADC | 0x0170 | 0-1023 | Sonicator 4 raw ADC power reading |

### Frequency Monitoring Registers  
| Register | Address | Raw Data Range | Description |
|----------|---------|----------------|-------------|
| REG_SON1_FREQ_DIV10_HZ | 0x0111 | 1800-2200 | Sonicator 1 frequency ÷10 (18-22kHz) |
| REG_SON2_FREQ_DIV10_HZ | 0x0131 | 1800-2200 | Sonicator 2 frequency ÷10 (18-22kHz) |
| REG_SON3_FREQ_DIV10_HZ | 0x0151 | 1800-2200 | Sonicator 3 frequency ÷10 (18-22kHz) |
| REG_SON4_FREQ_DIV10_HZ | 0x0171 | 1800-2200 | Sonicator 4 frequency ÷10 (18-22kHz) |

## Conversion Formulas

### Power Conversion (ADC → Watts)

**Hardware Specifications:**
- **CT2000 Power Output**: 0-8.160V DC (0-100% power operation)
- **Power Scaling**: 5.44mV = 1 WATT
- **ATmega32A ADC**: 10-bit (0-1023), 5V reference
- **Maximum Measurable**: ~919W (limited by 5V ADC reference)

**Conversion Formula:**
```python
def adc_to_power_watts(adc_raw):
    """Convert raw ADC reading to power in watts"""
    # Step 1: Convert ADC to voltage
    voltage = (adc_raw * 5.0) / 1023.0
    
    # Step 2: Apply CT2000 power scaling (5.44mV/W)
    power_watts = voltage / 0.00544  # 5.44mV/W = 0.00544V/W
    
    return power_watts

# Alternative single-step formula:
def adc_to_power_direct(adc_raw):
    """Direct conversion formula"""
    return adc_raw * 0.8988  # Pre-calculated: 5.0 / (1023 * 0.00544)
```

**Example Conversions:**
```python
# Example values
adc_raw = 272   # Raw ADC reading
power = adc_to_power_watts(272)  # Result: ~245W

adc_raw = 556   # Raw ADC reading  
power = adc_to_power_watts(556)  # Result: ~500W

adc_raw = 1023  # Maximum ADC reading
power = adc_to_power_watts(1023) # Result: ~919W (ADC saturated)
```

### Frequency Conversion (Div10 → Hz)

**Hardware Specifications:**
- **CT2000 Frequency Range**: 18kHz - 22kHz (typical 20kHz)
- **CT2000 FREQ_DIV10 Output**: Frequency ÷ 10 for digital monitoring
- **Raw Register Values**: 1800-2200 (representing 18-22kHz)

**Conversion Formula:**
```python
def freq_div10_to_hz(freq_div10_raw):
    """Convert frequency ÷10 reading to actual frequency in Hz"""
    return freq_div10_raw * 10

# Example conversions
freq_raw = 2000  # Raw register value
actual_freq = freq_div10_to_hz(2000)  # Result: 20,000 Hz (20kHz)

freq_raw = 1850  # Raw register value
actual_freq = freq_div10_to_hz(1850)  # Result: 18,500 Hz (18.5kHz)
```

### Voltage Conversion (ADC → Volts)

**For diagnostic purposes:**
```python
def adc_to_voltage(adc_raw):
    """Convert raw ADC reading to voltage"""
    return (adc_raw * 5.0) / 1023.0

# Example
adc_raw = 512   # Mid-scale ADC reading
voltage = adc_to_voltage(512)  # Result: ~2.5V
```

## Implementation Examples

### Python Cloud Processing
```python
class SonicatorDataProcessor:
    def __init__(self):
        self.power_scaling = 0.00544  # 5.44mV/W in V/W
        self.adc_max = 1023
        self.adc_ref_voltage = 5.0
        
    def process_sonicator_data(self, modbus_data):
        """Process raw MODBUS data into engineering units"""
        results = {}
        
        for sonicator_id in range(1, 5):  # Sonicators 1-4
            # Power conversion
            power_adc = modbus_data[f'son{sonicator_id}_power_raw']
            power_watts = self.adc_to_power(power_adc)
            
            # Frequency conversion  
            freq_div10 = modbus_data[f'son{sonicator_id}_freq_div10']
            freq_hz = freq_div10 * 10
            
            results[f'sonicator_{sonicator_id}'] = {
                'power_watts': power_watts,
                'frequency_hz': freq_hz,
                'power_adc_raw': power_adc,
                'freq_div10_raw': freq_div10
            }
            
        return results
    
    def adc_to_power(self, adc_raw):
        """Convert ADC to power with bounds checking"""
        if adc_raw > self.adc_max:
            adc_raw = self.adc_max
            
        voltage = (adc_raw * self.adc_ref_voltage) / self.adc_max
        power = voltage / self.power_scaling
        
        return max(0.0, power)  # Ensure non-negative
```

### PLC Ladder Logic (Structured Text)
```pascal
FUNCTION ADC_TO_POWER : REAL
VAR_INPUT
    ADC_Raw : INT;  (* Raw ADC value 0-1023 *)
END_VAR
VAR
    Voltage : REAL;
    Power : REAL;
END_VAR

(* Convert ADC to voltage *)
Voltage := INT_TO_REAL(ADC_Raw) * 5.0 / 1023.0;

(* Convert voltage to power using 5.44mV/W scaling *)
Power := Voltage / 0.00544;

(* Return result *)
ADC_TO_POWER := Power;
```

## Calibration and Accuracy

### Power Measurement Accuracy
- **ADC Resolution**: ~4.9mV per count (5V/1023)
- **Power Resolution**: ~0.9W per count (4.9mV / 5.44mV/W)
- **Measurement Range**: 0-919W (limited by 5V ADC reference)
- **Linearity**: Excellent within 0-5V range

### Frequency Measurement Accuracy
- **Resolution**: 10Hz per count (due to ÷10 scaling)
- **Range**: 18-22kHz (1800-2200 raw values)
- **Accuracy**: ±10Hz (limited by ÷10 resolution)

### Calibration Recommendations
1. **Per-Unit Calibration**: Each sonicator may have slight scaling variations
2. **Temperature Compensation**: Consider temperature effects on CT2000 scaling
3. **Periodic Verification**: Compare against external power meters
4. **Offset Correction**: Account for any DC offset in ADC readings

## Error Handling

### Power Reading Validation
```python
def validate_power_reading(adc_raw, sonicator_id):
    """Validate power ADC reading"""
    if adc_raw < 0 or adc_raw > 1023:
        raise ValueError(f"Invalid ADC reading {adc_raw} for sonicator {sonicator_id}")
    
    if adc_raw >= 1020:  # Near saturation
        print(f"Warning: Power ADC saturated for sonicator {sonicator_id}")
    
    return True
```

### Frequency Reading Validation  
```python
def validate_frequency_reading(freq_div10, sonicator_id):
    """Validate frequency reading"""
    if freq_div10 < 1700 or freq_div10 > 2300:
        raise ValueError(f"Frequency out of range: {freq_div10} for sonicator {sonicator_id}")
    
    return True
```

This raw data approach maximizes ATmega32A efficiency while providing full flexibility for cloud-based processing and calibration.
