# Multi-Sonicator-IO Web UI User Guide

## Overview

The Multi-Sonicator-IO Web UI provides an intuitive browser-based interface for testing and monitoring the ATmega32A sonicator multiplexer using an Arduino Test Harness. This guide will help you get started with the interface and make the most of its features.

## Getting Started

### Prerequisites

- Arduino Uno R3 with test harness firmware
- ATmega32A DUT (Device Under Test) with sonicator firmware
- USB connections for both devices
- Modern web browser (Chrome, Firefox, Safari)

### Quick Start

1. **Launch Sandbox Mode** (Recommended for first-time users):
   ```bash
   make web-ui-sandbox
   ```
   This will automatically:
   - Build and upload the latest firmware to the ATmega32A
   - Start the web interface
   - Open your browser to the interface

2. **Development Mode** (For ongoing development):
   ```bash
   make web-ui-dev
   ```

3. **Access the Interface**:
   - Open your browser to `http://localhost:3101`
   - The interface will automatically connect to the hardware

## Interface Overview

### Main Components

The web interface is a single-page application with the following main areas:

1. **Hardware Diagram** - Visual representation of Arduino ↔ ATmega32A connections
2. **Pin State Monitor** - Real-time display of all pin states
3. **Control Panel** - Hardware control buttons and inputs
4. **Arduino Command Log** - Live command history (last 5 commands)
5. **Test Automation** - BDD test execution interface
6. **Help System** - Context-sensitive help and tooltips

### Hardware Diagram

The central hardware diagram shows:
- **Arduino Uno R3** (Test Wrapper) on the left
- **ATmega32A** (DUT) on the right  
- **Connection lines** showing pin mappings
- **Real-time pin states** with color coding:
  - 🟢 Green: HIGH (digital) or active
  - 🔴 Red: LOW (digital) or inactive
  - 🟡 Yellow: Analog values (with numeric display)
  - ⚫ Gray: Disconnected or error state

### Pin State Monitoring

All monitored pins are displayed in real-time:

- **Digital Pins**: Show HIGH/LOW states with timestamps
- **Analog Pins**: Display numeric values (0-1023 for ADC)
- **PWM Pins**: Show frequency and duty cycle information
- **Communication Pins**: Indicate data transmission activity

Click on any pin connection to see detailed information including:
- Signal name and direction
- Current state and value
- Last change timestamp
- Pin function description

## Hardware Control

### GPIO Control

For input pins (pins that control the DUT):

1. **Click on a pin** in the hardware diagram
2. **Select desired state** (HIGH/LOW) from the control panel
3. **Click "Set Pin State"** to apply the change
4. **Observe the result** in real-time on the diagram

### ADC Monitoring

Analog pins automatically update their values:
- Values range from 0-1023 (10-bit ADC)
- Updates occur every 500ms by default
- Click on analog pins to see trending graphs

### PWM Generation

For PWM-capable pins:

1. **Select the PWM pin** from the dropdown
2. **Set frequency** (1Hz - 1kHz range)
3. **Set duty cycle** (0-100%)
4. **Click "Generate PWM"** to start
5. **Monitor waveform** in the visual display

### Power Management

Control DUT power supply:
- **Apply Voltage**: Set voltage level (3.3V or 5V)
- **Measure Current**: Monitor power consumption
- **Power Cycle**: Reset the DUT safely

## Test Automation

### Running BDD Tests

The interface integrates with the existing BDD test framework:

1. **Browse Available Tests**:
   - Click "Test Automation" tab
   - View all available test scenarios
   - Filter by tags (gpio, adc, pwm, modbus, etc.)

2. **Select Tests**:
   - Check boxes next to desired tests
   - Use tag presets for common test combinations
   - Search tests by name or description

3. **Execute Tests**:
   - Click "Run Selected Tests"
   - Monitor progress in real-time
   - View results as tests complete

4. **View Results**:
   - See pass/fail status for each test
   - Click on failed tests for detailed error information
   - Export results in JSON, CSV, or HTML format

### Test Categories

- **Basic Hardware**: GPIO, ADC, PWM functionality
- **Communication**: MODBUS, serial communication
- **Safety**: Overload protection, error handling
- **Performance**: Timing, frequency accuracy
- **Integration**: End-to-end system tests

## Arduino Command Log

The command log shows the last 5 commands sent to the Arduino:

- **Sent Commands**: Commands sent from the web interface
- **Responses**: Arduino responses and acknowledgments
- **Timestamps**: When each command was executed
- **Status**: Success/failure indicators

This helps with:
- **Debugging**: See exactly what commands are being sent
- **Learning**: Understand the command protocol
- **Troubleshooting**: Identify communication issues

## Help System

### Getting Help

- **Hover over any element** to see tooltips
- **Click the "?" icon** next to controls for detailed help
- **Press F1** for keyboard shortcuts
- **Click "Help"** in the top menu for this user guide

### Context-Sensitive Help

The help system provides specific guidance based on what you're doing:
- **Pin Control**: Explains pin functions and safe operating ranges
- **Test Execution**: Describes test scenarios and expected outcomes
- **Error Messages**: Provides clear explanations and suggested actions

## Keyboard Shortcuts

- **Ctrl+R**: Refresh hardware connection
- **Ctrl+T**: Open test automation panel
- **Ctrl+L**: Clear Arduino command log
- **Ctrl+H**: Toggle help tooltips
- **Escape**: Cancel current operation
- **Space**: Pause/resume real-time updates

## Troubleshooting

### Common Issues

**Hardware Not Connected**:
- Check USB cables are properly connected
- Verify Arduino appears in device manager
- Try clicking "Reconnect" button
- Restart the web interface

**Pin States Not Updating**:
- Check WebSocket connection status (green indicator)
- Refresh the browser page
- Verify Arduino test harness firmware is loaded

**Test Execution Fails**:
- Ensure hardware is properly connected
- Check that DUT firmware is loaded correctly
- Review Arduino command log for errors
- Try running individual tests to isolate issues

**Slow Response Times**:
- Close other browser tabs to free resources
- Check for USB connection issues
- Restart the backend service

### Error Messages

The interface provides clear error messages with suggested actions:

- **"Hardware not connected"**: Check physical connections
- **"Pin not found"**: Verify pin name matches configuration
- **"Test execution failed"**: Check hardware state and retry
- **"WebSocket disconnected"**: Refresh page to reconnect

## Advanced Features

### Custom Test Sequences

Create custom test sequences by:
1. Selecting multiple individual tests
2. Saving as a custom preset
3. Running the preset for repeated testing

### Data Export

Export test results and pin state data:
- **JSON**: For programmatic analysis
- **CSV**: For spreadsheet analysis  
- **HTML**: For reporting and documentation

### Configuration Management

Advanced users can:
- Modify pin assignments (requires restart)
- Adjust monitoring intervals
- Configure custom test parameters
- Set up automated test schedules

## Best Practices

### Safety

- **Always verify connections** before applying power
- **Start with low voltages** when testing new configurations
- **Monitor current consumption** to prevent damage
- **Use the emergency stop** if anything seems wrong

### Testing

- **Run basic tests first** before complex scenarios
- **Document any custom configurations** for repeatability
- **Save test results** for comparison and analysis
- **Use version control** for firmware changes

### Performance

- **Close unused browser tabs** for better performance
- **Use wired connections** instead of WiFi when possible
- **Monitor system resources** during long test runs
- **Restart services periodically** for optimal performance

## Support

For additional support:
- Check the project README for setup instructions
- Review the API documentation for integration details
- Consult the hardware configuration files for pin mappings
- Contact the development team for technical issues

## Version Information

- **Web UI Version**: 1.1.0
- **API Version**: 1.1.0
- **Compatible Firmware**: v2.0+
- **Browser Requirements**: Modern browsers with WebSocket support
