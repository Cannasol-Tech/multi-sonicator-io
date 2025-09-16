# Multi-Sonicator-IO Modular Configuration System

## Overview

The Multi-Sonicator-IO system now uses a **modular YAML configuration architecture** that separates different configuration concerns into dedicated files. This improves maintainability, version control, and team collaboration.

## Configuration Architecture

### Master Configuration
- **File**: `sonic-multiplexer.yaml`
- **Purpose**: Main configuration file that imports and coordinates all modular configs
- **Contains**: Project metadata, architecture overview, environment settings, and module references

### Modular Configuration Files

| File | Purpose | Scope |
|------|---------|-------|
| `pinout-cfg.yaml` | Hardware pin assignments | ATmega32A pin mappings, port definitions, ADC channels |
| `connections-cfg.yaml` | Physical connections | DB9 connectors, test harness, cable specifications |
| `communication-cfg.yaml` | Communication protocols | MODBUS RTU, serial settings, performance parameters |
| `sonicator-cfg.yaml` | Sonicator specifications | CT2000 specs, operational limits, channel definitions |

## File Structure

```
config/
├── sonic-multiplexer.yaml      # Master configuration
├── pinout-cfg.yaml             # Hardware pin assignments
├── connections-cfg.yaml        # DB9 and harness connections
├── communication-cfg.yaml      # MODBUS and serial settings
├── sonicator-cfg.yaml          # CT2000 sonicator specifications
├── README.md                   # This documentation
└── hardware-config.yaml        # Legacy (deprecated)
```

## Usage Examples

### Loading Configuration in Python

```python
from scripts.config_loader import ModularConfigLoader

# Initialize loader
loader = ModularConfigLoader('config')

# Load master and all modules
loader.load_master_config()
loader.load_all_modules()

# Get specific configurations
pin_config = loader.get_pin_config('PB0')
sonicator_config = loader.get_sonicator_config(4)
modbus_config = loader.get_modbus_config()
```

### Command Line Usage

```bash
# Load and validate all configurations
python scripts/config-loader.py --validate

# Show specific pin configuration
python scripts/config-loader.py --pin PB0

# Show sonicator 4 configuration
python scripts/config-loader.py --sonicator 4

# Export combined configuration for backward compatibility
python scripts/config-loader.py --export legacy-config.yaml
```

### C/C++ Integration

```c
// Example: Access pin configuration in firmware
#include "config_parser.h"

// Get pin number for sonicator 4 start control
uint8_t start_pin = get_pin_number("START_4");  // Returns 22 (PC0)

// Get ADC channel for sonicator 1 power sensing
uint8_t adc_channel = get_adc_channel("POWER_SENSE_1");  // Returns 4
```

## Configuration Details

### 1. Pinout Configuration (`pinout-cfg.yaml`)

**Contains:**
- ATmega32A pin assignments by port (PA, PB, PC, PD)
- Signal names and functions
- MODBUS register mappings
- ADC channel assignments
- Hardware specifications

**Example:**
```yaml
dut:
  pins:
    PB0:
      pin: 1
      signal: "FREQ_DIV10_4"
      direction: "IN"
      sonicator: 4
      modbus_register: "REG_SON4_FREQ_DIV10_HZ"
```

### 2. Connections Configuration (`connections-cfg.yaml`)

**Contains:**
- DB9 connector pinouts for each sonicator
- Test harness connection mappings
- Cable specifications and requirements
- Electrical characteristics

**Example:**
```yaml
db9_connectors:
  DB9-4:
    description: "CT2000 Sonicator 4 interface connector"
    pins:
      1: 
        signal: "OVERLOAD_4"
        dut_pin: "PD3"
        function: "Overload detection from CT2000"
```

### 3. Communication Configuration (`communication-cfg.yaml`)

**Contains:**
- MODBUS RTU parameters (baud rate, slave ID, timeouts)
- Supported function codes
- Error handling configuration
- Performance specifications

**Example:**
```yaml
modbus:
  slave_id: 2
  serial:
    baud_rate: 115200
    data_bits: 8
    stop_bits: 1
    parity: "none"
```

### 4. Sonicator Configuration (`sonicator-cfg.yaml`)

**Contains:**
- CT2000 sonicator specifications
- Operational parameters and limits
- Safety interlocks and protection
- Individual channel configurations

**Example:**
```yaml
sonicators:
  - id: 4
    name: "sonicator4"
    specifications:
      model: "CT2000-1800"
      power_rating_watts: 1800
      frequency_range_hz: [18000, 22000]
```

## Migration from Legacy Configuration

### Automatic Migration

The modular system is designed to be backward compatible. The legacy `hardware-config.yaml` file is deprecated but still supported.

```bash
# Generate combined config from modular files
python scripts/config-loader.py --export combined-config.yaml

# Validate migration
python scripts/config-loader.py --validate
```

### Manual Migration Steps

1. **Backup existing configuration**:
   ```bash
   cp config/hardware-config.yaml config/hardware-config.yaml.backup
   ```

2. **Load and validate new modular configuration**:
   ```bash
   python scripts/config-loader.py --validate
   ```

3. **Update code references** (if needed):
   - Replace direct YAML parsing with modular config loader
   - Update pin number references
   - Update MODBUS parameter access

## Validation and Testing

### Configuration Validation

The system includes comprehensive validation:

```bash
# Full validation
python scripts/config-loader.py --validate

# Check for pin conflicts
python scripts/validate-pins.py

# Verify MODBUS register mappings
python scripts/validate-modbus.py
```

### Validation Checks

- **Pin Conflicts**: Ensures no physical pin is assigned to multiple signals
- **DB9 Consistency**: Validates DB9 connector pin assignments
- **MODBUS Mapping**: Verifies register address consistency
- **Sonicator Mapping**: Confirms sonicator-to-hardware mappings
- **Cross-References**: Validates references between modules

## Development Workflow

### Making Configuration Changes

1. **Identify the appropriate module** for your change
2. **Edit the specific module file** (not the master file)
3. **Validate the configuration**:
   ```bash
   python scripts/config-loader.py --validate
   ```
4. **Test the changes** with HIL or unit tests
5. **Commit the changes** with descriptive commit message

### Adding New Configuration

1. **Determine the appropriate module** or create a new one
2. **Update the master configuration** to reference new modules
3. **Add validation rules** for the new configuration
4. **Update documentation** and examples

## Best Practices

### Configuration Management

- **Single Source of Truth**: Each configuration aspect has one authoritative location
- **Version Control**: All configuration files are version controlled
- **Change Tracking**: Use descriptive commit messages for configuration changes
- **Validation**: Always validate configuration changes before deployment

### File Organization

- **Logical Separation**: Keep related configuration together in appropriate modules
- **Clear Naming**: Use descriptive names for configuration keys
- **Documentation**: Include descriptions and comments for complex configurations
- **Cross-References**: Use clear references between related configurations

### Testing and Validation

- **Automated Validation**: Use the config loader validation features
- **Integration Testing**: Test configuration changes with HIL system
- **Regression Testing**: Ensure changes don't break existing functionality
- **Documentation Updates**: Keep documentation current with configuration changes

## Troubleshooting

### Common Issues

1. **Module Not Found**:
   ```
   Error: Module file not found: config/pinout-cfg.yaml
   ```
   **Solution**: Ensure all module files exist in the config directory

2. **YAML Parsing Error**:
   ```
   Error: YAML parsing error in config/pinout-cfg.yaml: ...
   ```
   **Solution**: Check YAML syntax, indentation, and special characters

3. **Pin Conflict**:
   ```
   Error: Pin conflict: Physical pin 22 used by multiple signals
   ```
   **Solution**: Review pin assignments and resolve conflicts

4. **Missing Cross-Reference**:
   ```
   Error: DB9-4 pin 1 references unknown DUT pin: PD3
   ```
   **Solution**: Ensure referenced pins exist in pinout configuration

### Debug Commands

```bash
# Show detailed configuration for debugging
python scripts/config-loader.py --pin PB0 --sonicator 4 --db9 DB9-4

# Export combined config for inspection
python scripts/config-loader.py --export debug-config.yaml

# Validate with verbose output
python scripts/config-loader.py --validate --verbose
```

## Future Enhancements

### Planned Features

- **Configuration Templates**: Templates for common configurations
- **Environment-Specific Configs**: Development, testing, production variants
- **Configuration Diff Tools**: Compare configuration versions
- **Automated Migration Tools**: Migrate from legacy formats
- **Configuration Validation Rules**: Custom validation rules
- **Configuration Documentation Generator**: Auto-generate documentation

### Integration Opportunities

- **CI/CD Integration**: Automated configuration validation in pipelines
- **Configuration Management Systems**: Integration with enterprise config management
- **Monitoring Integration**: Configuration change monitoring and alerting
- **Backup and Recovery**: Automated configuration backup and recovery

## Support and Maintenance

### Getting Help

- **Documentation**: Refer to this README and inline documentation
- **Validation Tools**: Use built-in validation and debugging tools
- **Issue Tracking**: Report issues via GitHub issues
- **Technical Support**: Contact development team for complex issues

### Maintenance Schedule

- **Regular Validation**: Weekly automated validation
- **Documentation Updates**: Monthly documentation review
- **Configuration Audits**: Quarterly configuration audits
- **Legacy Cleanup**: Annual legacy configuration cleanup
