# Story: Acceptance Testing Framework with HIL Integration

## Status: Ready for Development

## Story

As a **firmware developer**,
I want **Behave BDD acceptance tests integrated with Hardware-in-the-Loop (HIL) framework that validates all firmware functionality using real ATmega32A hardware**,
So that **every Epic 1 story can be verified against actual hardware behavior through automated acceptance scenarios, ensuring reliable integration before deployment**.

## Epic Context

### Epic 1: Foundational Control & Communication

Goal: Establish core firmware structure, basic sonicator control, and MODBUS communication layer

## Story Context

### Technology Stack

- **Behave BDD framework** for acceptance test scenarios and step definitions
- **Python-based HIL integration** using PySerial and modbus-tk within acceptance tests
- **ATmega32A target hardware** with Arduino Test Wrapper/Harness for safe HIL operation
- **Arduino as ISP programming** integrated into acceptance test workflows
- **CT2000 sonicator unit** for end-to-end hardware validation in BDD scenarios
- **MODBUS RTU communication verification** through acceptance test features
- **All components contained within `test/acceptance/` directory structure**

### Integration Points

- Arduino as ISP programming interface
- Serial communication for test control and logging
- MODBUS RTU communication testing
- Hardware signal measurement (GPIO, PWM, ADC)
- Test result reporting and CI/CD integration

## Acceptance Criteria

### BDD Acceptance Testing Requirements (Primary)

1. **Behave BDD acceptance tests operational with @hil tagged scenarios**
2. **environment.py configures HIL controller for all hardware scenarios**
3. **BDD feature files cover GPIO, ADC, PWM, MODBUS hardware validation**
4. **Step definitions integrate HIL framework for hardware assertions**
5. **`make test-acceptance` executes HIL hardware scenarios successfully**
6. **Arduino Test Wrapper acts as safe middleman for ATmega32A**
7. **HIL framework programs ATmega32A via Arduino as ISP from acceptance tests**
8. **Real hardware connectivity validation through BDD scenarios**
9. **MODBUS communication verified via acceptance test features**
10. **All HIL components contained within `test/acceptance/` directory**

### Arduino Test Harness Requirements

11. Arduino test harness/wrapper firmware acts as middleman between ATmega32A and HIL framework
12. Arduino harness provides safe sandbox mode for ATmega32A testing
13. Arduino harness can upload firmware to ATmega32A via ISP interface
14. Arduino harness monitors and reports ATmega32A pin states in real-time
15. Arduino harness can control ATmega32A output pins safely

### Sandbox Mode CLI Requirements

16. Interactive sandbox CLI provides continuous loop for hardware interaction
17. `monitor_on` / `monitor_off` commands control real-time pin monitoring
18. When monitoring enabled, pin states automatically displayed every second
19. `help` command displays all available sandbox commands
20. `status` command shows current state of all monitored pins
21. `set` command allows setting output pin states on ATmega32A
22. `reset` command resets the ATmega32A chip
23. `upload` command uploads firmware to ATmega32A from sandbox mode
24. `build` command builds ATmega32A firmware for loading
25. `clean` command performs clean build (wipes previous builds)
26. `quit` command exits sandbox mode and returns to terminal
27. When monitoring off, CLI suggests enabling monitoring every 15 seconds

### Hardware Validation Requirements

28. Pin connectivity validation for all sonicator interface pins per `docs/planning/pin-matrix.md`
29. Arduino Test Wrapper pin assignments match pin matrix Wrapper Pin column exactly
30. Power supply and voltage level verification (24V input, 5V/3.3V rails)
31. Crystal oscillator and timing verification (16MHz external crystal)
32. ADC functionality verification with known reference voltages
33. PWM output verification with oscilloscope measurements

### MODBUS Testing Requirements

34. MODBUS RTU slave communication verification at 115200 baud
35. Register read/write testing for all defined register ranges
36. CRC validation and error handling testing
37. Communication timeout and fault injection testing
38. Protocol compliance verification with standard MODBUS tools

### Quality Requirements

39. Test framework documented with setup and usage instructions
40. Automated test execution with pass/fail reporting
41. Test repeatability and deterministic results
42. Integration with existing build system (Make targets)
43. CI/CD pipeline integration requires real hardware availability

## Technical Implementation Details

### Acceptance Testing Framework with HIL Structure

```
test/acceptance/                 # Primary acceptance testing with HIL (ONLY location)
├── environment.py              # Behave environment with HIL controller
├── features/                   # BDD feature files for hardware scenarios
│   ├── hil_basic_connectivity.feature
│   ├── hil_power_verification.feature
│   ├── hil_gpio_functionality.feature
│   ├── hil_adc_verification.feature
│   ├── hil_pwm_generation.feature
│   └── hil_modbus_communication.feature
├── steps/                      # HIL-integrated step definitions
│   ├── hil_hardware_steps.py
│   ├── hil_connectivity_steps.py
│   ├── hil_modbus_steps.py
│   └── hil_control_steps.py
├── hil_framework/              # HIL Framework supporting acceptance tests
│   ├── __init__.py
│   ├── hil_controller.py       # Behave-integrated HIL controller
│   ├── hardware_interface.py   # Arduino Test Wrapper interface
│   ├── programmer.py           # Arduino as ISP integration
│   ├── sandbox_cli.py          # Manual HIL testing CLI
│   └── logger.py              # Test logging and reporting
├── arduino_harness/            # Arduino Test Wrapper/Harness firmware
│   ├── src/
│   │   ├── main.cpp            # Arduino wrapper firmware
│   │   ├── atmega_interface.cpp # ATmega32A control interface
│   │   ├── sandbox_mode.cpp    # Sandbox mode implementation
│   │   └── pin_monitor.cpp     # Real-time pin monitoring
│   └── platformio.ini          # Arduino harness build config
└── sketches/                   # Test firmware for validation
    └── arduino_test_wrapper/   # Pin assignments per docs/planning/pin-matrix.md
```

### Core HIL Framework Components

#### HIL Controller (`test/acceptance/hil_framework/hil_controller.py`)

```python
class HILController:
    """HIL Controller integrated with Behave acceptance testing framework"""
    
    def __init__(self, config_file='hil_config.yaml'):
        """Initialize HIL test framework with hardware configuration for Behave"""
        
    def setup_hardware(self):
        """Initialize hardware connections and verify basic connectivity"""
        
    def program_firmware(self, firmware_path):
        """Upload firmware to ATmega32A via Arduino as ISP"""
        
    def verify_firmware_version(self):
        """Verify firmware version and build information"""
        
    def run_acceptance_scenario(self, scenario_context):
        """Execute Behave scenario with HIL hardware validation"""
        
    def cleanup_hardware(self):
        """Safe hardware cleanup and disconnect"""
```

#### Behave Environment Integration (`test/acceptance/environment.py`)

```python
from test.acceptance.hil_framework.hil_controller import HILController
from test.acceptance.hil_framework.hardware_interface import HardwareInterface

def before_all(context):
    """Setup HIL framework for all acceptance tests"""
    context.hil_controller = HILController()
    context.hardware_interface = HardwareInterface()
    context.hil_controller.setup_hardware()
    
def before_scenario(context, scenario):
    """Setup HIL for each acceptance scenario"""
    if 'hil' in scenario.tags:
        context.hil_controller.program_firmware("test_firmware.hex")
        context.hardware_ready = True
    else:
        context.hardware_ready = False
        
def after_scenario(context, scenario):
    """Cleanup after each scenario"""
    if context.hardware_ready:
        context.hil_controller.cleanup_hardware()
        
def after_all(context):
    """Final HIL cleanup"""
    if hasattr(context, 'hil_controller'):
        context.hil_controller.cleanup_hardware()
```

#### BDD Feature Example (`test/acceptance/features/hil_gpio_functionality.feature`)

```gherkin
Feature: GPIO Hardware Functionality
  As a firmware developer
  I want to verify GPIO pin functionality with real hardware
  So that I can ensure reliable hardware abstraction layer operation

  @hil @hardware
  Scenario: Digital Output Control
    Given the ATmega32A is programmed with test firmware
    And the HIL framework is connected
    When I set pin PC0 to HIGH
    Then the HIL framework should read pin PC0 as HIGH
    And the voltage on pin PC0 should be approximately 5V
    
  @hil @hardware  
  Scenario: Digital Input Reading
    Given the ATmega32A is programmed with test firmware
    And the HIL framework is connected
    When the HIL framework applies 5V to pin PD3
    Then the firmware should read pin PD3 as HIGH
    And the pin state should be reported correctly via serial
```

#### HIL Step Definitions (`test/acceptance/steps/hil_hardware_steps.py`)

```python
from behave import given, when, then
from test.acceptance.hil_framework.hardware_interface import HardwareInterface

@given('the ATmega32A is programmed with test firmware')
def step_program_atmega_firmware(context):
    """Program ATmega32A with test firmware via HIL framework"""
    success = context.hil_controller.program_firmware("test_firmware.hex")
    assert success, "Failed to program ATmega32A with test firmware"

@given('the HIL framework is connected')
def step_hil_framework_connected(context):
    """Verify HIL framework hardware connection"""
    assert context.hardware_interface.verify_connection(), "HIL framework not connected"

@when('I set pin {pin} to {state}')
def step_set_pin_state(context, pin, state):
    """Set GPIO pin state via HIL framework"""
    pin_state = True if state.upper() == 'HIGH' else False
    context.hardware_interface.write_gpio_pin(pin, pin_state)
    context.last_pin_set = (pin, pin_state)

@then('the HIL framework should read pin {pin} as {state}')
def step_verify_pin_state(context, pin, state):
    """Verify GPIO pin state via HIL framework"""
    expected_state = True if state.upper() == 'HIGH' else False
    actual_state = context.hardware_interface.read_gpio_pin(pin)
    assert actual_state == expected_state, f"Pin {pin} expected {state}, got {'HIGH' if actual_state else 'LOW'}"

@then('the voltage on pin {pin} should be approximately {voltage}V')
def step_verify_pin_voltage(context, pin, voltage):
    """Verify pin voltage level via HIL framework"""
    adc_value = context.hardware_interface.read_adc_channel(pin)
    actual_voltage = context.hardware_interface.adc_to_voltage(adc_value)
    expected_voltage = float(voltage)
    tolerance = 0.2  # ±0.2V tolerance
    assert abs(actual_voltage - expected_voltage) <= tolerance, \
           f"Pin {pin} voltage {actual_voltage}V not within {tolerance}V of {expected_voltage}V"
```

#### Hardware Interface (`test/acceptance/hil_framework/hardware_interface.py`)

```python
class HardwareInterface:
    """Hardware interface for Behave acceptance testing with HIL integration"""
    
    def __init__(self, serial_port='/dev/ttyUSB0', baud_rate=115200):
        """Initialize serial communication with target hardware"""
        
    def verify_connection(self):
        """Verify hardware connection for acceptance tests"""
        
    def send_command(self, command, timeout=1.0):
        """Send command to firmware and wait for response"""
        
    def read_gpio_pin(self, pin):
        """Read digital state of GPIO pin for BDD verification"""
        
    def write_gpio_pin(self, pin, state):
        """Set digital state of GPIO pin for BDD testing"""
        
    def read_adc_channel(self, channel):
        """Read ADC value from specified channel for acceptance tests"""
        
    def adc_to_voltage(self, adc_value):
        """Convert ADC reading to voltage for BDD assertions"""
        
    def measure_pwm_output(self, pin):
        """Measure PWM frequency and duty cycle for acceptance criteria"""
        
    def modbus_read_register(self, address):
        """Read MODBUS register via RTU communication for BDD scenarios"""
        
    def modbus_write_register(self, address, value):
        """Write MODBUS register via RTU communication for BDD scenarios"""
```

#### Arduino Test Harness (`test/acceptance/arduino_harness/src/main.cpp`)

```cpp
// Arduino Test Harness for ATmega32A HIL Testing
// Acts as middleman between Python HIL framework and ATmega32A
// Pin assignments must match docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)

class ATmegaInterface {
    public:
        void begin();
        void programATmega(const char* firmware_path);
        bool resetATmega();
        void enterSandboxMode();
        void exitSandboxMode();
        
        // Pin monitoring and control
        void enablePinMonitoring();
        void disablePinMonitoring();
        bool readPin(uint8_t pin);
        void writePin(uint8_t pin, bool state);
        uint16_t readADC(uint8_t channel);
        
        // Real-time monitoring
        void updatePinStates();
        void sendPinStatus();
    
    private:
        bool sandbox_mode_active;
        bool monitoring_enabled;
        unsigned long last_monitor_update;
};
```

#### Sandbox CLI (`test/acceptance/hil_framework/sandbox_cli.py`)

```python
class SandboxCLI:
    """Interactive sandbox CLI integrated with acceptance testing framework"""
    
    def __init__(self, arduino_interface):
        """Initialize sandbox CLI with Arduino interface"""
        self.arduino = arduino_interface
        self.monitoring_enabled = False
        self.running = True
        self.last_monitor_suggestion = 0
        
    def run_interactive_loop(self):
        """Main interactive command loop for manual acceptance testing"""
        print("🔧 HIL Sandbox Mode - Type 'help' for commands")
        
        while self.running:
            if self.monitoring_enabled:
                self.display_pin_states()
                time.sleep(1)
            else:
                self.suggest_monitoring_every_15_seconds()
                
            # Check for user input without blocking
            command = self.get_user_input_non_blocking()
            if command:
                self.process_command(command)
    
    def process_command(self, command):
        """Process sandbox commands for manual hardware interaction"""
        cmd_parts = command.strip().lower().split()
        if not cmd_parts:
            return
            
        cmd = cmd_parts[0]
        
        if cmd == 'monitor_on':
            self.enable_monitoring()
        elif cmd == 'monitor_off':
            self.disable_monitoring()
        elif cmd == 'help':
            self.show_help()
        elif cmd == 'status':
            self.show_status()
        elif cmd == 'set':
            self.set_pin_state(cmd_parts[1:])
        elif cmd == 'reset':
            self.reset_atmega()
        elif cmd == 'upload':
            self.upload_firmware()
        elif cmd == 'build':
            self.build_firmware()
        elif cmd == 'clean':
            self.clean_build()
        elif cmd == 'run_acceptance':
            self.run_acceptance_tests(cmd_parts[1:])
        elif cmd == 'quit':
            self.quit_sandbox()
        else:
            print(f"❌ Unknown command: {cmd}. Type 'help' for available commands.")
    
    def run_acceptance_tests(self, test_args):
        """Run Behave acceptance tests from sandbox mode"""
        import subprocess
        
        behave_cmd = ['behave', 'test/acceptance']
        if test_args:
            behave_cmd.extend(test_args)
            
        print(f"🧪 Running acceptance tests: {' '.join(behave_cmd)}")
        result = subprocess.run(behave_cmd, capture_output=True, text=True)
        
        print("📊 Acceptance Test Results:")
        print(result.stdout)
        if result.stderr:
            print("❌ Errors:")
            print(result.stderr)
    
    def show_help(self):
        """Display available sandbox commands including acceptance testing"""
        help_text = """
🔧 HIL Sandbox Commands:
  monitor_on     - Enable real-time pin monitoring
  monitor_off    - Disable real-time pin monitoring  
  status         - Show current pin states
  set <pin> <state> - Set output pin state (e.g., 'set PC0 1')
  reset          - Reset ATmega32A chip
  upload         - Upload firmware to ATmega32A
  build          - Build ATmega32A firmware
  clean          - Clean build (wipe previous builds)
  run_acceptance [args] - Run Behave acceptance tests with HIL
  help           - Show this help message
  quit           - Exit sandbox mode
        """
        print(help_text)
```

### Hardware Validation Acceptance Tests

#### BDD Feature Structure

Each hardware validation test is written as a **BDD feature** with **Given-When-Then** scenarios:

**Features Cover:**

- Basic connectivity and power supply verification
- GPIO functionality with real pin state validation
- ADC accuracy with known reference voltage testing  
- PWM generation with frequency/duty cycle measurement
- MODBUS communication protocol compliance testing
- Timing and performance requirements validation

#### Example BDD Feature (`test/acceptance/features/hil_modbus_communication.feature`)

```gherkin
Feature: MODBUS RTU Communication Hardware Validation
  As a PLC integration engineer
  I want to verify MODBUS RTU communication with real hardware
  So that I can ensure reliable industrial automation integration

  Background:
    Given the ATmega32A is programmed with MODBUS firmware
    And the HIL framework is connected
    And the MODBUS slave ID is set to 2
    
  @hil @modbus @hardware
  Scenario: Read System Status Registers
    When I read MODBUS register 0x0000 (system status)
    Then the register value should be valid
    And the response time should be less than 100ms
    And the CRC should be correct
    
  @hil @modbus @hardware
  Scenario: Write and Verify Control Register
    When I write value 1 to MODBUS register 0x0010 (global enable)
    Then reading register 0x0010 should return value 1
    And the register change should be reflected within 100ms
    And the system status should update accordingly
    
  @hil @modbus @hardware  
  Scenario: Communication Fault Detection
    Given the MODBUS communication is established
    When I disconnect the communication cable
    And wait for 2 seconds
    Then the system should detect communication fault
    And enter failsafe mode per FR11 requirements
```

#### Integration with Existing Test Structure

The HIL framework operates within your existing acceptance test structure:

- **`test/acceptance/environment.py`** - Enhanced with HIL controller setup
- **`test/acceptance/features/`** - New `.feature` files for hardware scenarios  
- **`test/acceptance/steps/`** - HIL-specific step definitions
- **All acceptance tests require real hardware** - No emulation alternatives

### BDD Feature Examples for HIL Testing

#### Basic Connectivity Feature (`test/acceptance/features/hil_basic_connectivity.feature`)

```gherkin
Feature: Hardware Basic Connectivity
  As a firmware developer
  I want to verify basic hardware connectivity  
  So that I can ensure the HIL framework can communicate with the ATmega32A

  Background:
    Given the Arduino test harness is connected
    And the ATmega32A target is powered
    And the HIL framework is initialized

  Scenario: Power supply verification
    When I measure the power supply voltages
    Then the 24V input should be within tolerance
    And the 5V rail should be 5.0V ± 0.1V
    And the 3.3V rail should be 3.3V ± 0.1V

  Scenario: Programming interface verification  
    When I attempt to program the ATmega32A via Arduino as ISP
    Then the programming should succeed
    And the firmware version should be readable
    And the target should respond to commands

  Scenario: Serial communication establishment
    When I establish serial communication with the target
    Then I should receive a valid response
    And the communication should be stable for 10 seconds
```

#### GPIO Functionality Feature (`test/acceptance/features/hil_gpio_functionality.feature`)

```gherkin
Feature: GPIO Hardware Functionality
  As a firmware developer
  I want to verify GPIO pin control and monitoring
  So that I can ensure sonicator interface pins work correctly

  Background:
    Given the HIL framework is connected to ATmega32A
    And the GPIO HAL is loaded on the target

  Scenario: Digital output control
    When I set pin "PC0" to HIGH via HIL framework
    Then the pin voltage should be approximately 5.0V
    When I set pin "PC0" to LOW via HIL framework  
    Then the pin voltage should be approximately 0.0V

  Scenario: Digital input reading
    Given pin "PD3" is configured as input with pull-up
    When I apply 0V to pin "PD3" externally
    Then reading pin "PD3" should return LOW
    When I disconnect the external connection
    Then reading pin "PD3" should return HIGH

  Scenario: Pin state persistence
    When I set multiple pins to specific states
    And I wait for 5 seconds
    Then all pin states should remain unchanged
```

#### MODBUS Communication Feature (`test/acceptance/features/hil_modbus_communication.feature`)

```gherkin
Feature: MODBUS RTU Communication
  As a PLC integration engineer
  I want to verify MODBUS RTU communication via HIL
  So that I can ensure the sonicator control interface works correctly

  Background:
    Given the MODBUS RTU slave is running on ATmega32A
    And the HIL framework acts as MODBUS master
    And communication is established at 115200 baud

  Scenario: Register read operations
    When I read register "0x0000" (system status)
    Then I should receive a valid response
    And the response time should be less than 100ms
    
  Scenario: Register write operations  
    When I write value "1" to register "0x0100" (sonicator start)
    Then the write should succeed
    And reading the register should return "1"
    
  Scenario: CRC validation
    When I send a request with invalid CRC
    Then the slave should not respond
    When I send a request with valid CRC
    Then the slave should respond correctly

  Scenario: Communication timeout handling
    When I stop sending MODBUS requests for 2 seconds
    Then the communication fault flag should be set
    And the failsafe mode should activate
```

### Test Configuration and Setup

#### HIL Configuration (`test/acceptance/hil_framework/hil_config.yaml`)

```yaml
hardware:
  target_mcu: ATmega32A
  programmer: arduino_as_isp
  programmer_port: /dev/ttyUSB0
  target_serial_port: /dev/ttyUSB1
  
power_supply:
  input_voltage: 24.0
  regulated_5v: 5.0
  tolerance: 0.1
  
timing:
  crystal_frequency: 16000000
  serial_baud_rate: 115200
  
modbus:
  slave_id: 2
  baud_rate: 115200
  timeout: 1.0
  
# Behave integration settings
behave:
  profile: hil
  junit_output: true
  junit_directory: acceptance-junit
  hardware_required: true  # All tests require real hardware
  
test_equipment:
  oscilloscope_enabled: false
  multimeter_enabled: false
  logic_analyzer_enabled: false
```

### Makefile Integration

```makefile
# Primary target - HIL-integrated acceptance testing
test-acceptance: test-acceptance-hil

test-acceptance-hil:
    @echo "Running BDD acceptance tests with HIL hardware validation..."
    @python scripts/detect_hardware.py --check-arduino || (echo "❌ Hardware required" && exit 1)
    behave test/acceptance --junit --junit-directory=acceptance-junit --tags=hil

# Supporting targets for HIL framework
hardware-sandbox:
    python test/acceptance/hil_framework/sandbox_cli.py

upload-harness:
    cd test/acceptance/arduino_harness && pio run --target upload

monitor-device:
    python scripts/hil_cli.py --monitor

upload-to-device:
    python scripts/hil_cli.py --upload-firmware --check-isp

hil-setup:
    cd test && python -m pip install -r ../requirements-testing.txt
    python test/acceptance/hil_framework/hil_controller.py --setup

hil-clean:
    python test/acceptance/hil_framework/hil_controller.py --cleanup
```

## Definition of Done

- [ ] **Behave BDD acceptance tests operational with HIL integration**
- [ ] **@hil tagged scenarios passing with real ATmega32A hardware**
- [ ] **test/acceptance/ directory contains all HIL framework components**
- [ ] **make test-acceptance successfully executes HIL hardware scenarios**
- [ ] **Arduino Test Wrapper provides safe ATmega32A interface**
- [ ] **BDD features validate GPIO, ADC, PWM, MODBUS functionality**
- [ ] **Step definitions integrate HIL framework for hardware verification**
- [ ] Arduino as ISP programming integrated with acceptance tests
- [ ] Sandbox CLI operational for manual HIL testing and debugging
- [ ] Real-time pin monitoring functional during manual testing
- [ ] Test repeatability verified across multiple hardware runs
- [ ] CI/CD integration configured for HIL hardware testing requirements
- [ ] Documentation complete for HIL acceptance testing setup
- [ ] Framework validated with actual ATmega32A and Arduino hardware
- [ ] Code review completed for acceptance testing framework quality

## Dependencies

### Prerequisites

- Arduino Uno R4 WiFi configured as ISP programmer
- ATmega32A target hardware assembled and powered
- Hardware pin assignments verified against `docs/planning/pin-matrix.md` (SOLE SOURCE OF TRUTH)
- Python 3.8+ with PySerial, modbus-tk, pytest packages
- USB serial adapters for communication
- Basic test equipment (multimeter, optional oscilloscope)

### Enables Next Stories

- Epic 1 Story 1: Project Structure (with HIL verification)
- Epic 1 Story 2: HAL Implementation (with HIL validation)
- Epic 1 Story 3: MODBUS Framework (with HIL protocol testing)
- Epic 1 Story 4: Single Sonicator Control (with HIL hardware testing)

## Risk Assessment

**Primary Risk:** Hardware setup complexity and Arduino as ISP reliability issues
**Mitigation:** Comprehensive setup documentation and backup programming methods
**Verification:** Successful firmware programming and communication establishment

**Secondary Risk:** Test framework complexity may slow development
**Mitigation:** Start with minimal viable framework and expand incrementally
**Verification:** Basic connectivity tests operational before advanced features

**Third Risk:** Hardware availability and setup time requirements
**Mitigation:** Document exact hardware requirements and setup procedures
**Verification:** Complete hardware setup guide with troubleshooting steps

## Notes

- HIL framework must be simple and reliable for continuous use
- Focus on essential hardware validation over comprehensive test coverage initially
- Framework supports manual testing and requires real hardware for CI/CD
- All timing-critical tests must account for hardware response delays
- Test results must be deterministic and repeatable for reliable verification
- Framework will be extended as Epic 1 stories are implemented and require verification
- **Hardware-only approach** - No emulation alternatives supported
- **Pin assignments must strictly follow `docs/planning/pin-matrix.md`** - Any deviations require pin matrix updates first
