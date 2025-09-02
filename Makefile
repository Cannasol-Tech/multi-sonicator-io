# Multi-Sonicator I/O Controller Makefile
# Uses Arduino Framework with PlatformIO for ATmega32A development

#  Make Targets

## Firmware Related Make Targets (Arduino Framework)

build:
	# Build ATmega32A firmware using Arduino Framework via PlatformIO
	pio run -e atmega32a

clean:
	# Clean Arduino Framework build artifacts
	pio run --target clean

upload:
	# Upload Arduino Framework firmware to ATmega32A via USBASP
	pio run -e atmega32a --target upload

upload-to-device:
	# Upload Arduino Framework firmware to device using USBASP programmer
	pio run -e atmega32a --target upload

build-emulator:
	# TODO: Implement make target to build the emulator docker image with Arduino Framework
	    # - IMPORTANT:  This make target should result with a running simulavr docker container with the latest Arduino firmware loaded
		# - This make target should output the docker container information

monitor-emulator: 
	# TODO:  This make target should evoke a cli (python script) that interacts with the emulator the same way we interact for hil-sandbox mode

monitor-device: 
	# TODO:  This make target should evoke a cli (python script) that monitors the serial output coming from the Arduino Test Wrapper / Harness	

upload-to-device: 
   # TODO: Upload the ATMEGA32A Firmware to the device using the arduino as an ISP, This should first check to see if the Arduino as ISP script is running on the arduino and if it is - it should tell the user and ask if the user wants to continue to upload again or not 

upload-to-emulator: 
   # TODO: Upload the ATMEGA32A Firmware to the emulator using the emulators

upload-harness: 
	# TODO: This make target should upload the ardunio test harness to the Arduino that is connected to the computer acting as a middle man for the ATMEGA32A chip

hardware-sandbox:
    # TODO:  This make target should evoke a cli (python script) that interacts with the user until they reach a point to being connected to the hardware in a "sandbox" mode 
	    # - The sandbox mode should consist of the following: 
		#     1. The user should be able to send commands to the Arduino Test Wrapper / Harness
		# 	  2. The python cli should be in a continuous loop with the user able to send "monitor_on" or "monitor_off" commands 
		#     3. The "monitor_on" and "monitor_off" commands should be able to turn on and off the monitoring of the AtMega32A chip .
		#     4. When Monitoring is on - the python cli should automatically print the state of all of the monitored pins to the CLI every second 
		#     5. The python cli should have a "quit" command that will exit the sandbox mode and return the user to the terminal 
		#     6. The python cli should have a "help" command that will print the list of available commands to the user 
		#     7. The python cli should have a "status" command that will print the current state of the monitored pins to the CLI 
		#     8. The python cli should have a "set" command that will allow the user to set the state of the output pins on the AtMega32A chip 
		#     9. The python cli should have a "reset" command that will reset the AtMega32A chip 
		#    10. The python cli should have a "upload" command that will upload the ATMEGA32A firmware to the device or emulator 
		#    11. The python cli should have a "build" command that will build the ATMEGA32A firmware for loading on the device or emulator 
		#    12. The python cli should have a "clean" command that will clean the ATMEGA32A firmware build - This should wipe all previous builds and generated files so we ensure a clean new build 
		#    13. When monitoring is turned off -- the CLI should notify the user and suggest turning it on every 15 seconds ( implemented with a count in the main infinite loop )


## Testing Make Targets - Aligned with Software Testing Standard
test-all: test-unit test-hil
	@echo "Running all tests..."

# Full CI test suite per software testing standard (Unit → Acceptance → Integration)
ci-test: test-unit test-acceptance test-integration generate-release-artifacts
	@echo "Running complete CI test suite per software testing standard..."
	@echo "✅ Unit tests: pytest with 90% coverage"
	@echo "✅ Acceptance tests: BDD scenarios via HIL hardware"  
	@echo "✅ Integration tests: HIL hardware validation"
	@echo "✅ Release artifacts: Generated per release format standard"

# Three-stage testing per software testing standard
test-unit: 
	@echo "Stage 1: Unit Testing (pytest with 90% coverage)..."
	pytest tests/unit/ \
		--cov=src \
		--cov-report=json:coverage.json \
		--cov-report=html:htmlcov \
		--junit-xml=unit-test-results.xml \
		--cov-fail-under=90

test-acceptance:
	@echo "Stage 2: Acceptance Testing (BDD scenarios via HIL)..."
	@python scripts/detect_hardware.py --check-arduino || (echo "❌ Hardware required for acceptance tests" && exit 1)
	@echo "✅ Hardware detected - running HIL acceptance tests..."
	behave test/acceptance \
		--junit \
		--junit-directory=acceptance-junit \
		-D profile=hil

test-integration:
	@echo "Stage 3: Integration Testing (HIL hardware validation)..."
	@python scripts/detect_hardware.py --check-hil || (echo "❌ Hardware required for integration tests" && exit 1)
	@echo "✅ Hardware detected - running HIL integration tests..."
	behave test/acceptance \
		--junit \
		--junit-directory=integration-junit \
		-D profile=hil \
		--tags="@integration"

test-unit-legacy:
	@echo "Running legacy PlatformIO Unity tests..."
	pio test -e test_desktop

test-emulation:
	@echo "Running emulation tests with JUnit reports..."
	behave test/acceptance \
		--junit \
		--junit-directory=acceptance-junit \
		-D profile=simulavr

test-hil:
	@echo "Running HIL tests with JUnit reports..."
	behave test/acceptance \
		--junit \
		--junit-directory=acceptance-junit \
		-D profile=hil

generate-release-artifacts:
	@echo "Generating release format compliant artifacts..."
	python scripts/release/generate_executive_report.py \
		--acceptance-results=acceptance-junit \
		--integration-results=integration-junit \
		--unit-results=unit-test-results.xml \
		--coverage=coverage.json \
		--output=final
	@echo "✅ Release artifacts generated in final/"

