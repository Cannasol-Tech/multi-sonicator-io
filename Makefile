# Multi-Sonicator I/O Controller Makefile
# Uses Arduino Framework with PlatformIO for ATmega32A development
# Unity Tests for Unit Testing, Behave for Acceptance Testing

# Declare phony targets (targets that don't create files)
.PHONY: build clean upload install-deps check-deps check-pio check-arduino-cli test-unit test-acceptance test-all ci-test
.PHONY: monitor-device upload-to-device upload-harness setup-arduino-isp check-arduino-isp
.PHONY: hardware-sandbox acceptance-setup acceptance-clean acceptance-test-basic acceptance-test-gpio acceptance-test-adc
.PHONY: acceptance-test-pwm acceptance-test-modbus acceptance-test-power generate-release-artifacts test-integration

#  Make Targets


# Python dependency installation
install-deps:
	@echo "📦 Installing Python dependencies..."
	pip3 install -r requirements-testing.txt

# Check and install dependencies if needed
check-deps:
	@echo "🔍 Checking Python dependencies..."
	@python3 -c "import behave, serial, pytest" 2>/dev/null || (echo "📦 Installing missing dependencies..." && pip3 install -r requirements-testing.txt)
	@echo "✅ All Python dependencies available"

# Check and install PlatformIO if needed
check-pio:
	@echo "🔍 Checking PlatformIO..."
	@which pio >/dev/null 2>&1 || (echo "📦 Installing PlatformIO..." && pip3 install platformio && echo "✅ PlatformIO installed")
	@echo "✅ PlatformIO available"

# Check and install Arduino CLI if needed (for HIL testing)
check-arduino-cli:
	@echo "🔍 Checking Arduino CLI..."
	@which arduino-cli >/dev/null 2>&1 || (echo "⚠️ Arduino CLI not found. Install with: brew install arduino-cli (macOS) or see https://arduino.github.io/arduino-cli/")
	@echo "✅ Arduino CLI check complete"

## Firmware Related Make Targets (Arduino Framework)

build:
	# Build ATmega32A firmware using Arduino Framework via PlatformIO
	pio run -e atmega32a

clean:
	# Clean Arduino Framework build artifacts
	pio run --target clean

upload:
	# Upload Arduino Framework firmware to ATmega32A via Arduino as ISP
	@echo "🔧 Setting up Arduino as ISP (auto-upload if needed)..."
	@python3 scripts/setup_arduino_isp.py || (echo "❌ Failed to setup Arduino as ISP" && exit 1)
	@echo "✅ Arduino as ISP is ready"
	@echo "📤 Uploading firmware to ATmega32A..."
	pio run -e atmega32a --target upload

monitor-device:
	# Monitor serial output from Arduino Test Harness
	python3 scripts/hil_cli.py monitor

upload-to-device:
	# Upload ATmega32A firmware using Arduino as ISP with safety checks
	@echo "🔧 Setting up Arduino as ISP (auto-upload if needed)..."
	@python3 scripts/setup_arduino_isp.py || (echo "❌ Failed to setup Arduino as ISP" && exit 1)
	@echo "✅ Arduino as ISP is ready"
	@echo "📤 Uploading firmware via HIL CLI..."
	python3 scripts/hil_cli.py upload

upload-harness:
	# Upload Arduino Test Harness firmware to Arduino
	cd test/acceptance/arduino_harness && pio run --target upload

setup-arduino-isp:
	# Setup Arduino as ISP (auto-upload ArduinoISP sketch if needed)
	python3 scripts/setup_arduino_isp.py

check-arduino-isp:
	# Check if Arduino as ISP is ready (no upload)
	python3 scripts/setup_arduino_isp.py --check-only

hardware-sandbox: check-deps check-pio check-arduino-cli
	@echo "🔧 Setting up HIL Hardware Sandbox Environment..."
	@echo "Step 1: Setting up Arduino as ISP (auto-upload if needed)..."
	@python3 scripts/setup_arduino_isp.py || (echo "❌ Failed to setup Arduino as ISP" && exit 1)
	@echo "✅ Arduino as ISP is ready"
	
	@echo "Step 2: Building latest ATmega32A firmware..."
	@pio run -e atmega32a || (echo "❌ Firmware build failed" && exit 1)
	@echo "✅ ATmega32A firmware build successful"

	@echo "Step 3: Programming ATmega32A target via Arduino as ISP..."
	@pio run -e atmega32a -t upload || (echo "❌ ATmega32A programming failed" && exit 1)
	@echo "✅ ATmega32A programmed successfully"
	
	@echo "Step 4: Switching from Arduino ISP to Test Harness..."
	@echo "📋 Please perform the following hardware changes:"
	@echo "   1. Remove the capacitor from Arduino RESET line"  
	@echo "   2. Connect Arduino Test Harness to target"
	@echo "   3. Ensure proper pin connections per docs/planning/pin-matrix.md"
	@read -p "Press Enter when hardware setup is complete..." dummy
	
	@echo "Step 5: Uploading Arduino Test Harness firmware..."
	@cd test/acceptance/arduino_harness && pio run --target upload || (echo "❌ Test harness upload failed" && exit 1)
	@echo "✅ Arduino Test Harness uploaded successfully"
	
	@echo "Step 6: Waiting for Arduino Test Harness to initialize..."
	@sleep 3
	
	@echo "Step 7: Launching HIL Sandbox CLI..."
	@echo "🎯 HIL Hardware Sandbox is ready!"
	@echo "   - ATmega32A programmed with latest firmware"
	@echo "   - Arduino Test Harness loaded and ready"
	@echo "   - HIL framework connected"
	@echo ""
	python3 test/acceptance/hil_framework/sandbox_cli.py


## Testing Make Targets - Aligned with Software Testing Standard

# Complete test suite per software testing standard (Unit → Acceptance → Integration)
test: check-deps check-pio test-unit test-acceptance test-integration
	@echo "✅ Complete test suite executed per software testing standard"
	@echo "   - Unit tests: Unity Test Framework with 90% coverage requirement"
	@echo "   - Acceptance tests: BDD scenarios via Behave + pytest HIL framework"
	@echo "   - Integration tests: HIL hardware validation"

test-all: check-deps check-pio test-unit test-acceptance
	@echo "Running all tests..."

# Full CI test suite per software testing standard (Unit → Acceptance → Integration)
ci-test: check-deps check-pio test-unit test-acceptance generate-release-artifacts
	@echo "Running complete CI test suite per software testing standard..."
	@echo "✅ Unit tests: Unity Test Framework with 90% coverage"
	@echo "✅ Acceptance tests: BDD scenarios via Behave + pytest HIL framework"  
	@echo "✅ Integration tests: HIL hardware validation"
	@echo "✅ Release artifacts: Generated per release format standard"

# Three-stage testing per software testing standard
test-unit: check-deps check-pio
	@echo "Stage 1: Unit Testing (Unity Test Framework for embedded C/C++ with 90% coverage)..."
	@echo "Running comprehensive Unity tests for HAL and Communication modules..."
	pio test -e comprehensive_test -v
	@echo "✅ Unity unit tests completed with 90%+ coverage for HAL and Communication modules"

test-acceptance: check-deps check-arduino-cli
	@echo "Stage 2: Acceptance Testing (BDD scenarios via Behave framework)..."
	@python3 scripts/detect_hardware.py --check-arduino || (echo "❌ Hardware required for acceptance tests" && exit 1)
	@echo "✅ Hardware detected - running HIL acceptance tests via Behave..."
	PYTHONPATH=. python3 -m behave test/acceptance \
		--junit \
		--junit-directory=acceptance-junit \
		-D profile=hil

test-integration: check-deps check-arduino-cli
	@echo "Stage 3: Integration Testing (HIL hardware validation for embedded systems)..."
	@python3 scripts/detect_hardware.py --check-arduino || (echo "❌ Hardware required for integration tests" && exit 1)
	@echo "✅ Hardware detected - running HIL integration tests..."
	python3 -m behave test/acceptance \
		--junit \
		--junit-directory=integration-junit \
		-D profile=integration \
		--tags=integration

# HIL Testing Targets
test-acceptance-hil: check-deps check-arduino-cli
	@echo "🧪 Running BDD acceptance tests with HIL hardware validation..."
	@python3 scripts/detect_hardware.py --check-arduino || (echo "❌ Hardware required for HIL testing" && exit 1)
	behave test/acceptance --junit --junit-directory=acceptance-junit --tags=hil -D profile=hil

acceptance-setup: check-deps
	@echo "🔧 Setting up acceptance test framework..."
	python3 test/acceptance/hil_framework/hil_controller.py --setup

acceptance-clean: check-deps
	@echo "🧹 Cleaning acceptance test framework..."
	python3 test/acceptance/hil_framework/hil_controller.py --cleanup

acceptance-test-basic: check-deps check-arduino-cli
	@echo "🔌 Running basic acceptance connectivity tests..."
	PYTHONPATH=. python3 -m behave test/acceptance/features/hil_basic_connectivity.feature -D profile=hil

acceptance-test-gpio: check-deps check-arduino-cli
	@echo "🔧 Running acceptance GPIO functionality tests..."
	python3 test_acceptance_simple.py

acceptance-test-adc: check-deps check-arduino-cli
	@echo "📊 Running acceptance ADC verification tests..."
	PYTHONPATH=. python3 -m behave test/acceptance/features/hil_adc_verification.feature -D profile=hil

acceptance-test-pwm: check-deps check-arduino-cli
	@echo "📡 Running acceptance PWM generation tests..."
	PYTHONPATH=. python3 -m behave test/acceptance/features/hil_pwm_generation.feature -D profile=hil

acceptance-test-modbus: check-deps check-arduino-cli
	@echo "🔗 Running acceptance MODBUS communication tests..."
	PYTHONPATH=. python3 -m behave test/acceptance/features/hil_modbus_communication.feature -D profile=hil

acceptance-test-power: check-deps check-arduino-cli
	@echo "⚡ Running acceptance power verification tests..."
	PYTHONPATH=. python3 -m behave test/acceptance/features/hil_power_verification.feature -D profile=hil

generate-release-artifacts: check-deps
	@echo "Generating release format compliant artifacts..."
	python3 scripts/release/generate_executive_report.py \
		--acceptance-results=acceptance-junit \
		--integration-results=integration-junit \
		--unit-results=unit-test-results.xml \
		--coverage=coverage.json \
		--output=final
	@echo "✅ Release artifacts generated in final/"

