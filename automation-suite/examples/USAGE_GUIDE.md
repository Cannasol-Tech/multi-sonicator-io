# Automation Suite Usage Guide

This guide provides practical examples for using the automation suite tools in different project types and environments.

## Quick Start

### 1. Basic Setup
```bash
# Copy automation suite to your project
cp -r automation-suite/ /path/to/your/project/

# Install dependencies
cd your-project
pip install -r automation-suite/requirements.txt
```

### 2. Configure for Your Project Type
```bash
# Create project-specific configuration
cp automation-suite/examples/performance-config.yaml my-project-config.yaml

# Edit the configuration file
# Set project_type to: platformio, cmake, npm, python, or generic
```

### 3. Run Basic Tools
```bash
# Detect hardware
python automation-suite/ci-tools/hardware_detection.py

# Run performance benchmarks
python automation-suite/ci-tools/performance_benchmark.py --config my-project-config.yaml

# Generate documentation
python automation-suite/documentation/generate_docs.py

# Run test scenarios
python automation-suite/testing/scenario_runner.py --scenarios automation-suite/examples/integration-scenarios.yaml
```

## Project Type Examples

### PlatformIO Projects

#### Configuration (`platformio-config.yaml`)
```yaml
project_type: "platformio"
build:
  environments:
    - "test_desktop"
    - "arduino_uno"
    - "esp32dev"
  iterations: 3
  clean_build: true
monitoring:
  memory_threshold: "256MB"
thresholds:
  build_time_warning: 120
  test_time_warning: 60
```

#### Usage
```bash
# Hardware detection with PlatformIO profiles
python automation-suite/ci-tools/hardware_detection.py --config platformio-config.yaml

# Performance benchmarking
python automation-suite/ci-tools/performance_benchmark.py \
  --config platformio-config.yaml \
  --environment arduino_uno \
  --iterations 5

# Test execution
python automation-suite/testing/scenario_runner.py \
  --scenarios platformio-scenarios.yaml \
  --config platformio-config.yaml
```

#### Custom Test Scenarios (`platformio-scenarios.yaml`)
```yaml
scenarios:
  - name: "HARDWARE_COMPILATION_TEST"
    description: "Test compilation for different hardware targets"
    steps:
      - type: "command"
        description: "Clean build environment"
        command: "pio run --target clean"
        timeout: 30
      
      - type: "command"
        description: "Build for Arduino Uno"
        command: "pio run -e arduino_uno"
        timeout: 120
      
      - type: "validation"
        description: "Check firmware binary exists"
        validation_type: "file_exists"
        file_path: ".pio/build/arduino_uno/firmware.hex"

  - name: "UNIT_TEST_EXECUTION"
    description: "Run unit tests on desktop environment"
    steps:
      - type: "command"
        description: "Run unit tests"
        command: "pio test -e test_desktop"
        timeout: 180
        expected_output: "PASSED"
```

### Node.js/NPM Projects

#### Configuration (`nodejs-config.yaml`)
```yaml
project_type: "npm"
build:
  environments: ["development", "production"]
  iterations: 2
  parallel_jobs: 4
monitoring:
  memory_threshold: "1GB"
  cpu_threshold: 70
thresholds:
  build_time_warning: 60
  test_time_warning: 30
```

#### Usage
```bash
# Install and build
npm install
python automation-suite/ci-tools/performance_benchmark.py --config nodejs-config.yaml

# Test scenarios
python automation-suite/testing/scenario_runner.py --scenarios nodejs-scenarios.yaml
```

#### Test Scenarios (`nodejs-scenarios.yaml`)
```yaml
scenarios:
  - name: "NPM_BUILD_TEST"
    description: "Test NPM build process"
    steps:
      - type: "command"
        description: "Install dependencies"
        command: "npm install"
        timeout: 120
      
      - type: "command"
        description: "Run build"
        command: "npm run build"
        timeout: 180
      
      - type: "validation"
        description: "Check build output"
        validation_type: "file_exists"
        file_path: "dist"

  - name: "UNIT_TEST_SUITE"
    description: "Run NPM test suite"
    steps:
      - type: "command"
        description: "Run tests"
        command: "npm test"
        timeout: 120
        expected_output: "All tests passed"
```

### Python Projects

#### Configuration (`python-config.yaml`)
```yaml
project_type: "python"
build:
  environments: ["development"]
  iterations: 2
test:
  parallel: true
  timeout: 120
monitoring:
  memory_threshold: "512MB"
```

#### Usage
```bash
# Setup virtual environment
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# Run automation tools
python automation-suite/ci-tools/performance_benchmark.py --config python-config.yaml
python automation-suite/testing/scenario_runner.py --scenarios python-scenarios.yaml
```

### CMake Projects

#### Configuration (`cmake-config.yaml`)
```yaml
project_type: "cmake"
build:
  environments: ["debug", "release"]
  parallel_jobs: 8
thresholds:
  build_time_warning: 180
  build_time_error: 600
```

#### Custom Build Commands
For CMake projects, you may need to customize the build commands in the performance benchmark tool:

```python
# In a custom configuration script
def cmake_build_commands():
    return {
        'clean': ['make', 'clean'],
        'build': ['make', '-j8'],
        'test': ['make', 'test']
    }
```

## GitHub Actions Integration

### Basic Workflow Setup
```yaml
# .github/workflows/automation-suite.yml
name: Automation Suite CI

on: [push, pull_request]

jobs:
  automation:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
    
    - name: Setup Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
    
    - name: Install automation suite
      run: pip install -r automation-suite/requirements.txt
    
    - name: Hardware detection
      run: python automation-suite/ci-tools/hardware_detection.py
    
    - name: Performance benchmarks
      run: python automation-suite/ci-tools/performance_benchmark.py
    
    - name: Run tests
      run: python automation-suite/testing/scenario_runner.py
    
    - name: Generate documentation
      run: python automation-suite/documentation/generate_docs.py
```

### Advanced Workflow with Matrix Strategy
```yaml
strategy:
  matrix:
    project_type: [platformio, cmake, python]
    environment: [development, production]
    
steps:
- name: Run benchmarks
  run: |
    python automation-suite/ci-tools/performance_benchmark.py \
      --config configs/${{ matrix.project_type }}-config.yaml \
      --environment ${{ matrix.environment }}
```

## Local Development Workflow

### Makefile Integration
```makefile
# Add to your project's Makefile

# Automation suite targets
automation-setup:
	pip install -r automation-suite/requirements.txt

automation-detect:
	python automation-suite/ci-tools/hardware_detection.py

automation-benchmark:
	python automation-suite/ci-tools/performance_benchmark.py --config my-config.yaml

automation-test:
	python automation-suite/testing/scenario_runner.py --scenarios my-scenarios.yaml

automation-docs:
	python automation-suite/documentation/generate_docs.py

automation-all: automation-detect automation-benchmark automation-test automation-docs
	@echo "Automation suite execution complete"

.PHONY: automation-setup automation-detect automation-benchmark automation-test automation-docs automation-all
```

### Pre-commit Hooks
```bash
# Add to .git/hooks/pre-commit
#!/bin/bash
echo "Running automation suite pre-commit checks..."

# Hardware detection
python automation-suite/ci-tools/hardware_detection.py --output /tmp/hardware_check.json

# Quick performance check
python automation-suite/ci-tools/performance_benchmark.py \
  --config quick-config.yaml \
  --build-only \
  --iterations 1

# Basic test scenarios
python automation-suite/testing/scenario_runner.py \
  --scenarios automation-suite/examples/integration-scenarios.yaml

echo "Pre-commit checks passed!"
```

## Customization Examples

### Custom Hardware Profile
```yaml
# In hardware detection config
hardware_profiles:
  my_custom_device:
    serial_vid_pid: ['0x1234:0x5678']
    description_patterns: ['MyDevice', 'Custom Hardware']
    baudrate: 115200
    test_command: 'STATUS'
    expected_responses: ['OK', 'READY']
```

### Custom Test Step Function
```python
# custom_steps.py
def validate_firmware_checksum(step, scenario, context):
    """Custom validation function for firmware integrity"""
    firmware_path = step.get('firmware_path')
    expected_checksum = step.get('expected_checksum')
    
    # Calculate actual checksum
    import hashlib
    with open(firmware_path, 'rb') as f:
        actual_checksum = hashlib.md5(f.read()).hexdigest()
    
    if actual_checksum == expected_checksum:
        scenario.log(f"Firmware checksum validated: {actual_checksum}")
        return True
    else:
        scenario.log(f"Checksum mismatch: expected {expected_checksum}, got {actual_checksum}")
        return False

# Register the custom function
runner = TestScenarioRunner()
runner.register_custom_function('validate_firmware_checksum', validate_firmware_checksum)
```

### Custom Performance Metrics
```python
# In performance benchmark configuration
def collect_custom_metrics():
    """Collect project-specific performance metrics"""
    metrics = {}
    
    # Example: Memory usage analysis
    import psutil
    process = psutil.Process()
    metrics['memory_percent'] = process.memory_percent()
    
    # Example: File system usage
    metrics['build_artifacts_size'] = get_directory_size('.pio/build/')
    
    return metrics
```

## Troubleshooting

### Common Issues

#### 1. Permission Errors
```bash
# Fix permission issues for hardware detection
sudo usermod -a -G dialout $USER
# Logout and login again
```

#### 2. Missing Dependencies
```bash
# Install project-specific dependencies
case "$PROJECT_TYPE" in
  "platformio")
    pip install platformio
    ;;
  "cmake")
    sudo apt-get install cmake build-essential
    ;;
  "npm")
    curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
    sudo apt-get install -y nodejs
    ;;
esac
```

#### 3. Timeout Issues
```yaml
# Increase timeouts in configuration
thresholds:
  build_time_error: 1200  # 20 minutes
  test_time_error: 600    # 10 minutes

test:
  timeout: 600  # 10 minutes per test scenario
```

#### 4. Simulation Mode Not Working
```bash
# Force simulation mode
export HARDWARE_SIMULATION=true
python automation-suite/ci-tools/hardware_detection.py --force-simulation
```

### Debug Mode
```bash
# Enable verbose output
python automation-suite/ci-tools/performance_benchmark.py --verbose
python automation-suite/testing/scenario_runner.py --debug
```

## Best Practices

### 1. Configuration Management
- Keep project-specific configurations in version control
- Use environment variables for sensitive settings
- Maintain separate configs for development/staging/production

### 2. Test Organization
- Group related test scenarios in separate files
- Use descriptive names for scenarios and steps
- Include cleanup steps for test isolation

### 3. Performance Monitoring
- Set realistic performance thresholds
- Monitor trends over time
- Use performance baselines for comparison

### 4. Documentation
- Generate documentation regularly
- Include examples in API documentation
- Keep user manuals up to date

### 5. CI/CD Integration
- Use hardware simulation in CI environments
- Cache dependencies to improve build times
- Parallelize independent jobs
- Upload artifacts for debugging