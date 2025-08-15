# Multi-Sonicator I/O Controller Makefile
# 
# This Makefile provides convenient targets for building, testing, and 
# generating documentation for the Multi-Sonicator project.
#
# Usage: make [target]
# Run 'make help' to see all available targets.

.PHONY: help clean setup test test-unit test-simulation test-embedded build build-dev build-prod docs reports performance memory-check version all \
        emu-venv emu-test emu-stop emu-docker-build emu-docker-test

# Default target
.DEFAULT_GOAL := help

# Colors for output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

# Project directories
SCRIPTS_DIR = scripts
CI_SCRIPTS_DIR = $(SCRIPTS_DIR)/ci
RELEASE_SCRIPTS_DIR = $(SCRIPTS_DIR)/release
TEST_RESULTS_DIR = test_results
REPORTS_DIR = reports
DOCS_DIR = docs

# Python interpreter
PYTHON = python

# PlatformIO command
PIO = pio

## Display this help message
help:
	@echo "$(BLUE)Multi-Sonicator I/O Controller Build System$(NC)"
	@echo ""
	@echo "$(YELLOW)Available targets:$(NC)"
	@echo ""
	@echo "$(GREEN)Setup and Environment:$(NC)"
	@echo "  setup           - Setup development environment"
	@echo "  clean           - Clean build artifacts and temporary files"
	@echo "  version         - Display current project version"
	@echo ""
	@echo "$(GREEN)Testing:$(NC)"
	@echo "  test            - Run all tests (unit + simulation)"
	@echo "  test-unit       - Run unit tests on desktop"
	@echo "  test-simulation - Run hardware simulation tests"
	@echo "  test-embedded   - Run embedded tests (requires hardware)"
	@echo "  test-verbose    - Run tests with verbose output"
	@echo ""
	@echo "$(GREEN)Building:$(NC)"
	@echo "  build           - Build all environments (dev + prod)"
	@echo "  build-dev       - Build development firmware"
	@echo "  build-prod      - Build production firmware"
	@echo "  build-clean     - Clean and rebuild all"
	@echo ""
	@echo "$(GREEN)Documentation and Reports:$(NC)"
	@echo "  docs            - Generate project documentation"
	@echo "  reports         - Generate test and build reports"
	@echo "  performance     - Run performance benchmarks"
	@echo "  memory-check    - Run memory leak detection"
	@echo ""
	@echo "$(GREEN)Release Management:$(NC)"
	@echo "  version-current - Show current version"
	@echo "  version-bump    - Bump version (interactive)"
	@echo "  release-check   - Check if ready for release"
	@echo ""
	@echo "$(GREEN)Complete Workflows:$(NC)"
	@echo "  all             - Build everything and run all tests"
	@echo "  ci              - Run CI pipeline locally"
	@echo "  pre-commit      - Run pre-commit checks"
	@echo ""
	@echo "$(YELLOW)Examples:$(NC)"
	@echo "  make setup      # First time setup"
	@echo "  make test       # Quick test run"
	@echo "  make build      # Build firmware"
	@echo "  make all        # Complete build and test"

## Setup development environment
setup:
	@echo "$(YELLOW)Setting up development environment...$(NC)"
	@mkdir -p $(TEST_RESULTS_DIR) $(REPORTS_DIR)
	@if [ -f "$(CI_SCRIPTS_DIR)/setup_environment.sh" ]; then \
		chmod +x $(CI_SCRIPTS_DIR)/setup_environment.sh && \
		$(CI_SCRIPTS_DIR)/setup_environment.sh; \
	fi
	@if [ -f "$(CI_SCRIPTS_DIR)/setup_simulation.sh" ]; then \
		chmod +x $(CI_SCRIPTS_DIR)/setup_simulation.sh && \
		$(CI_SCRIPTS_DIR)/setup_simulation.sh; \
	fi
	@echo "$(GREEN)Environment setup complete$(NC)"

## Clean build artifacts and temporary files
clean:
	@echo "$(YELLOW)Cleaning build artifacts...$(NC)"
	@$(PIO) run --target clean
	@rm -rf .pio/build
	@rm -rf $(TEST_RESULTS_DIR)
	@rm -rf $(REPORTS_DIR)
	@rm -rf .coverage
	@find . -name "*.pyc" -delete
	@find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
	@echo "$(GREEN)Clean complete$(NC)"

## Run all tests (unit + simulation)
test: test-unit test-simulation

## Run unit tests on desktop
test-unit:
	@echo "$(YELLOW)Running unit tests...$(NC)"
	@$(PIO) test -e test_desktop
	@echo "$(GREEN)Unit tests complete$(NC)"

## Run hardware simulation tests
test-simulation:
	@echo "$(YELLOW)Running hardware simulation tests...$(NC)"
	@$(PIO) test -e test_desktop --filter test_virtual_sonicator
	@echo "$(GREEN)Simulation tests complete$(NC)"

## Run embedded tests (requires hardware)
test-embedded:
	@echo "$(YELLOW)Running embedded tests...$(NC)"
	@$(PYTHON) $(SCRIPTS_DIR)/detect_hardware.py || (echo "$(RED)No hardware detected. Use 'make test-simulation' instead$(NC)" && exit 1)
	@$(PIO) test -e test_embedded
	@echo "$(GREEN)Embedded tests complete$(NC)"

## Run tests with verbose output
test-verbose:
	@echo "$(YELLOW)Running tests with verbose output...$(NC)"
	@$(PIO) test -e test_desktop --verbose

## Build all environments (dev + prod)
build: build-dev build-prod

## Build development firmware
build-dev:
	@echo "$(YELLOW)Building development firmware...$(NC)"
	@$(PIO) run -e development
	@echo "$(GREEN)Development build complete$(NC)"

## Build production firmware
build-prod:
	@echo "$(YELLOW)Building production firmware...$(NC)"
	@$(PIO) run -e production
	@echo "$(GREEN)Production build complete$(NC)"

## Clean and rebuild all
build-clean: clean build

## Generate project documentation
docs:
	@echo "$(YELLOW)Generating documentation...$(NC)"
	@$(PYTHON) $(SCRIPTS_DIR)/generate_docs.py
	@echo "$(GREEN)Documentation generated$(NC)"

## Generate test and build reports
reports:
	@echo "$(YELLOW)Generating reports...$(NC)"
	@mkdir -p $(REPORTS_DIR)
	@if [ -d "$(TEST_RESULTS_DIR)" ]; then \
		$(PYTHON) $(CI_SCRIPTS_DIR)/generate_reports.py --test-results $(TEST_RESULTS_DIR); \
	else \
		echo "$(YELLOW)No test results found. Run 'make test' first.$(NC)"; \
	fi
	@echo "$(GREEN)Reports generated$(NC)"

## Run performance benchmarks
performance:
	@echo "$(YELLOW)Running performance benchmarks...$(NC)"
	@$(PYTHON) $(CI_SCRIPTS_DIR)/performance_benchmark.py
	@echo "$(GREEN)Performance benchmarks complete$(NC)"

## Run memory leak detection
memory-check:
	@echo "$(YELLOW)Running memory leak detection...$(NC)"
	@$(PYTHON) $(CI_SCRIPTS_DIR)/memory_leak_detector.py
	@echo "$(GREEN)Memory check complete$(NC)"

## Show current project version
version-current:
	@echo "$(YELLOW)Current project version:$(NC)"
	@$(PYTHON) $(RELEASE_SCRIPTS_DIR)/version_bump.py --current

## Bump project version (interactive)
version-bump:
	@echo "$(YELLOW)Version bump (interactive):$(NC)"
	@$(PYTHON) $(RELEASE_SCRIPTS_DIR)/version_bump.py

## Check if project is ready for release
release-check: test build docs
	@echo "$(YELLOW)Checking release readiness...$(NC)"
	@$(PYTHON) $(RELEASE_SCRIPTS_DIR)/version_bump.py --current
	@echo "$(GREEN)Release check complete$(NC)"

## Build everything and run all tests
all: clean setup build test docs reports
	@echo "$(GREEN)Complete build and test cycle finished$(NC)"

## Run CI pipeline locally
ci: setup test build performance memory-check docs reports
	@echo "$(GREEN)Local CI pipeline complete$(NC)"

## Run pre-commit checks
pre-commit: test build
	@echo "$(GREEN)Pre-commit checks complete$(NC)"

# Display current version
version: version-current

# Additional utility targets

## Check PlatformIO installation
check-pio:
	@which $(PIO) > /dev/null || (echo "$(RED)PlatformIO not found. Install with: pip install platformio$(NC)" && exit 1)
	@echo "$(GREEN)PlatformIO found: $$($(PIO) --version)$(NC)"

## Check Python dependencies
check-python:
	@which $(PYTHON) > /dev/null || (echo "$(RED)Python not found$(NC)" && exit 1)
	@echo "$(GREEN)Python found: $$($(PYTHON) --version)$(NC)"

## Environment check
check-env: check-python check-pio
	@echo "$(GREEN)Environment check complete$(NC)"

## Show project structure
tree:
	@echo "$(YELLOW)Project structure:$(NC)"
	@tree -I '.git|.pio|__pycache__|*.pyc|node_modules' || ls -la

#==============================================================================
# Emulation (simulavr) and BDD
#==============================================================================

EMU_VENV=.venv-emu
EMU_PY=$(EMU_VENV)/bin/python
EMU_PIP=$(EMU_VENV)/bin/pip
EMU_BEHAVE=$(EMU_VENV)/bin/behave

## Prepare Python venv for emulation tests
emu-venv:
	@echo "$(YELLOW)Creating emulation venv...$(NC)"
	@test -d $(EMU_VENV) || python3 -m venv $(EMU_VENV)
	@$(EMU_PIP) install --upgrade pip
	@$(EMU_PIP) install -r scripts/emulation/requirements-emulation.txt
	@echo "$(GREEN)Emulation venv ready$(NC)"

## Run emulation BDD tests using simulavr in venv
emu-test: emu-venv
	@echo "$(YELLOW)Running emulation tests (simulavr) ...$(NC)"
	@which simulavr >/dev/null 2>&1 || { echo "$(RED)simulavr not found in PATH. Install simulavr first.$(NC)"; exit 1; }
	@set -e; \
	ELF_RUN_CMD="$(EMU_PY) scripts/emulation/run_simulavr.py"; \
	# start emulator in background
	$$ELF_RUN_CMD & \
	EMU_PID=$$!; \
	echo "simulavr PID: $$EMU_PID"; \
	sleep 2; \
	# run Behave with simulavr profile; pending scenarios are skipped by default
	$(EMU_BEHAVE) -D profile=simulavr || STATUS=$$?; \
	# stop emulator
	kill $$EMU_PID 2>/dev/null || true; \
	wait $$EMU_PID 2>/dev/null || true; \
	if [ -n "$$STATUS" ]; then exit $$STATUS; fi
	@echo "$(GREEN)Emulation tests complete$(NC)"

## Stop any stray emulators
emu-stop:
	@pkill -f "simulavr" 2>/dev/null || true
	@echo "$(GREEN)Terminated any running simulavr processes$(NC)"

## Build Docker image for emulation tests (Ubuntu with simulavr)
emu-docker-build:
	@echo "$(YELLOW)Building emulation Docker image...$(NC)"
	@docker build -f scripts/emulation/Dockerfile.emu -t msio-emu:latest .
	@echo "$(GREEN)Emulation Docker image built$(NC)"

## Run emulation tests inside Docker
emu-docker-test: emu-docker-build
	@echo "$(YELLOW)Running emulation tests in Docker...$(NC)"
	@docker run --rm -t \
	  -v $$(pwd):/workspace \
	  -w /workspace \
	  msio-emu:latest \
	  bash -lc "python3 -m venv .venv-emu && . .venv-emu/bin/activate && pip install -r scripts/emulation/requirements-emulation.txt && python3 scripts/emulation/run_simulavr.py & PID=\$$!; sleep 2; behave -D profile=simulavr; kill \$$PID; wait \$$PID || true"
	@echo "$(GREEN)Docker emulation tests complete$(NC)"

## Monitor serial output (requires hardware)
monitor:
	@echo "$(YELLOW)Starting serial monitor...$(NC)"
	@$(PIO) device monitor

## Upload firmware (requires hardware and programmer)
upload:
	@echo "$(YELLOW)Uploading firmware...$(NC)"
	@$(PYTHON) $(SCRIPTS_DIR)/detect_hardware.py || (echo "$(RED)No hardware detected$(NC)" && exit 1)
	@$(PIO) run -e production --target upload

## Show detailed help for specific target
help-%:
	@echo "$(YELLOW)Help for target: $*$(NC)"
	@make -n $* 2>/dev/null || echo "$(RED)Target '$*' not found$(NC)"

#==============================================================================
# Automation Suite Targets
#==============================================================================

## Setup automation suite dependencies
automation-setup:
	@echo "$(YELLOW)Setting up automation suite...$(NC)"
	@$(PYTHON) automation-suite/run.py setup

## Run hardware detection
automation-detect:
	@echo "$(YELLOW)Running hardware detection...$(NC)"
	@$(PYTHON) automation-suite/run.py detect

## Run performance benchmarks
automation-benchmark:
	@echo "$(YELLOW)Running performance benchmarks...$(NC)"
	@$(PYTHON) automation-suite/run.py benchmark --config automation-suite/examples/performance-config.yaml

## Run test scenarios
automation-test:
	@echo "$(YELLOW)Running automation test scenarios...$(NC)"
	@$(PYTHON) automation-suite/run.py test --scenarios automation-suite/examples/integration-scenarios.yaml

## Generate documentation with automation suite
automation-docs:
	@echo "$(YELLOW)Generating documentation...$(NC)"
	@$(PYTHON) automation-suite/run.py docs --formats markdown html

## Run complete automation pipeline
automation-all: automation-setup
	@echo "$(YELLOW)Running complete automation pipeline...$(NC)"
	@$(PYTHON) automation-suite/run.py all --config automation-suite/examples/performance-config.yaml

## Force simulation mode for automation tools
automation-simulate:
	@echo "$(YELLOW)Running automation in simulation mode...$(NC)"
	@$(PYTHON) automation-suite/run.py detect --force-simulation
	@$(PYTHON) automation-suite/run.py test --scenarios automation-suite/examples/integration-scenarios.yaml

## Clean automation suite generated files
automation-clean:
	@echo "$(YELLOW)Cleaning automation suite files...$(NC)"
	@rm -f hardware_detection.json performance_report.json test_results.json automation_summary.json
	@rm -rf generated-docs/ automation-suite/generated-docs/