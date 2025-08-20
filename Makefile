# Multi-Sonicator I/O Controller Makefile
#
# This Makefile provides convenient targets for building, testing, and
# generating documentation for the Multi-Sonicator project.
#
# Usage: make [target]
# Run 'make help' to see all available targets.

.PHONY: help clean setup test test-unit test-simulation test-embedded build build-dev build-prod docs reports performance memory-check version all \
        emu-stop emu-docker-build emu-docker-test test-emulation test-acceptance test-hil test-hil-if-available test-all \
        emu-docker-run emu-docker-run-smoke emu-docker-smoke-seq

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

# Python interpreter (prefer python3, fallback to python)
PYTHON_BIN := $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null)
ifeq ($(PYTHON_BIN),)
  PYTHON := python3
else
  PYTHON := $(PYTHON_BIN)
endif

# PlatformIO command
# Resolve PlatformIO as:
# 1) 'pio' if present
# 2) 'platformio' if present
# 3) fallback to 'python3 -m platformio'
PIO_BIN := $(shell command -v pio 2>/dev/null || command -v platformio 2>/dev/null)
ifeq ($(PIO_BIN),)
  PIO := python3 -m platformio
else
  PIO := $(PIO_BIN)
endif

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
	@echo "  check-deps      - Check required tooling (python, PlatformIO, optional: simulavr)"
	@echo "  install-pio     - Install PlatformIO via pipx/pip"
	@echo "  bootstrap       - Show how to install prerequisites"
	@echo ""
	@echo "$(GREEN)Testing:$(NC)"
	@echo "  test-all        - Run all tests (unit + acceptance + HIL)"
	@echo "  test-unit       - Run unit tests on desktop (PlatformIO + Unity)"
	@echo "  test-acceptance - Run acceptance tests in simulavr emulator"
	@echo "  test-hil        - Run HIL tests on real hardware (when available)"
	@echo "  test-emulation  - Alias for test-acceptance"
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
	@echo "$(GREEN)Emulation & BDD:$(NC)"
	@echo "  simulavr-build-elf   - Build firmware ELF for emulation"
	@echo "  simulavr-run         - Run simulavr with built ELF"
	@echo "  bdd-simulavr         - Run Behave BDD against simulavr (requires PTY wiring)"
	@echo "  bdd-smoke            - Run smoke-tagged BDD against simulavr"
	@echo "  emu-docker-smoke-seq - Start emulator container, wait for PTY, run smoke, show logs on failure"
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
	@rm -rf .venv-emu  # Remove any leftover venv directories
	@find . -name "*.pyc" -delete
	@find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
	@echo "$(GREEN)Clean complete$(NC)"

## Run all tests (unit + acceptance + HIL when available)
test-all: test-unit test-acceptance test-hil-if-available

## Legacy alias for backwards compatibility
test: test-unit test-acceptance

## Run unit tests on desktop (PlatformIO + Unity)
test-unit: check-pio
	@echo "$(YELLOW)Running unit tests...$(NC)"
	@$(PIO) test -e test_desktop
	@echo "$(GREEN)Unit tests complete$(NC)"

## Run acceptance tests in simulavr emulator (BDD) - Docker-based
test-acceptance: emu-docker-test

## Run HIL tests on real hardware (when available)
test-hil:
	@echo "$(YELLOW)Running HIL tests on real hardware...$(NC)"
	@$(PYTHON) $(SCRIPTS_DIR)/detect_hardware.py || (echo "$(RED)No hardware detected. Use 'make test-acceptance' instead$(NC)" && exit 1)
	@which behave >/dev/null 2>&1 || { echo "$(RED)behave not found. Install with: pip install behave$(NC)"; exit 1; }
	@behave -D profile=hil test/acceptance
	@echo "$(GREEN)HIL tests complete$(NC)"

## Run HIL tests only if hardware is available (no error if not)
test-hil-if-available:
	@echo "$(YELLOW)Checking for hardware for HIL tests...$(NC)"
	@if $(PYTHON) $(SCRIPTS_DIR)/detect_hardware.py >/dev/null 2>&1; then \
		echo "$(GREEN)Hardware detected, running HIL tests...$(NC)"; \
		which behave >/dev/null 2>&1 || { echo "$(RED)behave not found. Install with: pip install behave$(NC)"; exit 1; }; \
		behave -D profile=hil test/acceptance; \
		echo "$(GREEN)HIL tests complete$(NC)"; \
	else \
		echo "$(YELLOW)No hardware detected, skipping HIL tests$(NC)"; \
	fi

## Legacy simulation tests (kept for backwards compatibility)
test-simulation: check-pio
	@echo "$(YELLOW)Running hardware simulation tests...$(NC)"
	@$(PIO) test -e test_desktop --filter test_virtual_sonicator
	@echo "$(GREEN)Simulation tests complete$(NC)"

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

## Check PlatformIO installation (set AUTO_INSTALL_PIO=1 to attempt install)
check-pio:
	@if ! which $(PIO) >/dev/null 2>&1; then \
		if [ "$(AUTO_INSTALL_PIO)" = "1" ]; then \
			echo "$(YELLOW)PlatformIO not found. Attempting installation...$(NC)"; \
			if which pipx >/dev/null 2>&1; then \
				echo "Using pipx"; pipx install platformio || true; \
			else \
				echo "Using pip --user"; python3 -m pip install --user platformio || true; \
			fi; \
		else \
			echo "$(RED)PlatformIO not found. Install with: pipx install platformio || python3 -m pip install --user platformio$(NC)"; exit 1; \
		fi; \
	fi; \
	$(PIO) --version >/dev/null 2>&1 && echo "$(GREEN)PlatformIO found: $$($(PIO) --version)$(NC)"

## Check optional simulavr
check-simulavr:
	@which simulavr >/dev/null 2>&1 && echo "$(GREEN)simulavr found: $$(simulavr --version 2>/dev/null || echo installed)$(NC)" || echo "$(YELLOW)simulavr not found (optional). Install with: brew install simulavr$(NC)"

## Aggregate dependency check
check-deps: check-python check-pio check-simulavr
	@echo "$(GREEN)Dependency check complete$(NC)"

## Explicitly install PlatformIO (non-interactive)
install-pio:
	@echo "$(YELLOW)Installing PlatformIO...$(NC)"
	@if which pipx >/dev/null 2>&1; then \
		pipx install platformio; \
	else \
		python3 -m pip install --user platformio; \
	fi

## Bootstrap developer machine with basic prerequisites (non-destructive)
bootstrap:
	@echo "$(YELLOW)Bootstrap instructions:$(NC)"
	@echo "1) Ensure Python3 is installed (e.g., brew install python)"
	@echo "2) Install PlatformIO: pipx install platformio  (or)  python3 -m pip install --user platformio"
	@echo "3) Optional: brew install simulavr (for emulation targets)"

## Check Python availability and print version
check-python:
	@if ! command -v $(PYTHON) >/dev/null 2>&1; then \
		echo "$(RED)Python not found$(NC)"; \
		echo "Install with Homebrew: brew install python"; \
		exit 1; \
	fi
	@echo "$(GREEN)Python found: $$($(PYTHON) --version)$(NC)"

## Environment check
check-env: check-python check-pio
	@echo "$(GREEN)Environment check complete$(NC)"

## Show project structure
tree:
	@echo "$(YELLOW)Project structure:$(NC)"
	@tree -I '.git|.pio|__pycache__|*.pyc|node_modules' || ls -la

#==============================================================================
# Emulation (simulavr) and BDD - Docker-based
#==============================================================================

## Build firmware ELF for simulavr (development env)
simulavr-build-elf:
	@echo "$(YELLOW)Building firmware ELF for simulavr...$(NC)"
	@$(PIO) run -e development
	@ls -l .pio/build/development/firmware.elf 2>/dev/null || echo "$(RED)ELF not found. Build may have failed.$(NC)"

## Run simulavr with built ELF (no UART PTY wiring yet)
simulavr-run:
	@echo "$(YELLOW)Launching simulavr runner...$(NC)"
	@which simulavr >/dev/null 2>&1 || { echo "$(RED)simulavr not found in PATH. Install simulavr first.$(NC)"; exit 1; }
	@$(PYTHON) scripts/emulation/simavr/run_simulavr.py



## Stop any stray emulators
emu-stop:
	@pkill -f "simulavr" 2>/dev/null || true
	@echo "$(GREEN)Terminated any running simulavr processes$(NC)"

## Build Docker image for emulation tests (Ubuntu with simulavr)
emu-docker-build:
	@echo "$(YELLOW)Building emulation Docker image...$(NC)"
	@docker build -f scripts/emulation/Dockerfile.emu -t msio-emu:latest .
	@echo "$(GREEN)Emulation Docker image built$(NC)"

## Build pysimulavr module using tools/simulavr (Docker builder)
simulavr-python-build:
	@echo "$(YELLOW)Building pysimulavr with Docker...$(NC)"
	@docker build -f tools/simulavr/Dockerfile.optionA -t msio-simulavr-builder:latest tools/simulavr
	@echo "$(GREEN)pysimulavr build finished. Module under ./simulavr/build/pysimulavr$(NC)"


## Run emulation tests inside Docker
emu-docker-test: emu-docker-build
	@echo "$(YELLOW)Running emulation tests in Docker...$(NC)"
	@docker run --rm -t --privileged \
	  -v $$(pwd):/workspace \
	  -w /workspace \
	  msio-emu:latest \
	  bash -lc "set -euo pipefail; \
            /opt/venv/bin/python scripts/emulation/cli.py > /tmp/emu.log 2>&1 & PID=\$$!; \
            for i in $$(seq 1 90); do [ -e /tmp/tty-msio ] && break; sleep 1; done; \
            if [ ! -e /tmp/tty-msio ]; then echo 'Emulator PTY not ready after 90s'; tail -n 200 /tmp/emu.log || true; ps aux || true; exit 1; fi; \
            /opt/venv/bin/behave -D profile=simulavr test/acceptance || STATUS=\$$?; STATUS=\$${STATUS:-0}; \
            if [ \$$STATUS -ne 0 ]; then echo '--- emulator log (last 200) ---'; tail -n 200 /tmp/emu.log || true; ps aux || true; fi; \
            kill \$$PID 2>/dev/null || true; \
            wait \$$PID || true; \
            exit \$$STATUS"
	@echo "$(GREEN)Docker emulation tests complete$(NC)"

## Alias matching help text
test-emulation: test-acceptance

## Run only the smoke BDD in Docker (fast check)
emu-docker-test-smoke: emu-docker-build
	@echo "$(YELLOW)Running emulation SMOKE test in Docker...$(NC)"
	@docker run --rm -t \
	  -v $$(pwd):/workspace \
	  -w /workspace \
	  msio-emu:latest \
	  bash -lc "set -euo pipefail; \
            /opt/venv/bin/python scripts/emulation/cli.py & PID=\$$!; \
            for i in $$(seq 1 90); do [ -e /tmp/tty-msio ] && break; sleep 1; done; \
            /opt/venv/bin/behave -D profile=simulavr -t @smoke test/acceptance; \
            kill \$$PID 2>/dev/null || true; \
            wait \$$PID || true"
	@echo "$(GREEN)Docker emulation SMOKE test complete$(NC)"

# Run-only variants: do not build/pull; use existing local image
## Run emulation tests in Docker without rebuilding image
emu-docker-run:
	@echo "$(YELLOW)Running emulation tests in Docker (no rebuild)...$(NC)"
	@docker image inspect msio-emu:latest >/dev/null 2>&1 || { echo "$(RED)Image msio-emu:latest not found locally. Run 'make emu-docker-build' first.$(NC)"; exit 1; }
	@docker run --rm -t --privileged -v $$(pwd):/workspace -w /workspace msio-emu:latest bash -lc 'set -euo pipefail; /opt/venv/bin/python scripts/emulation/cli.py > /tmp/emu.log 2>&1 & PID=$$!; for i in $$(seq 1 90); do [ -e /tmp/tty-msio ] && break; sleep 1; done; if [ ! -e /tmp/tty-msio ]; then echo "Emulator PTY not ready after 90s"; tail -n 200 /tmp/emu.log || true; ps aux || true; exit 1; fi; /opt/venv/bin/behave -D profile=simulavr test/acceptance || STATUS=$$?; STATUS=$${STATUS:-0}; if [ $$STATUS -ne 0 ]; then echo "--- emulator log (last 200) ---"; tail -n 200 /tmp/emu.log || true; ps aux || true; fi; kill $$PID 2>/dev/null || true; wait $$PID || true; exit $$STATUS'
	@echo "$(GREEN)Docker emulation tests (no rebuild) complete$(NC)"

## Run only the smoke BDD in Docker without rebuilding image
emu-docker-run-smoke:
	@echo "$(YELLOW)Running emulation SMOKE test in Docker (no rebuild)...$(NC)"
	@docker image inspect msio-emu:latest >/dev/null 2>&1 || { echo "$(RED)Image msio-emu:latest not found locally. Run 'make emu-docker-build' first.$(NC)"; exit 1; }
	@docker run --rm -t --privileged -v $$\(pwd\):/workspace -w /workspace msio-emu:latest bash -lc 'set -euo pipefail; /opt/venv/bin/python scripts/emulation/cli.py > /tmp/emu.log 2>&1 & PID=$$!; for i in $$(seq 1 90); do [ -e /tmp/tty-msio ] && break; sleep 1; done; if [ ! -e /tmp/tty-msio ]; then echo "Emulator PTY not ready after 90s"; tail -n 200 /tmp/emu.log || true; ps aux || true; exit 1; fi; /opt/venv/bin/behave -D profile=simulavr -t @smoke test/acceptance || STATUS=$$?; STATUS=$${STATUS:-0}; if [ $$STATUS -ne 0 ]; then echo "--- emulator log (last 200) ---"; tail -n 200 /tmp/emu.log || true; ps aux || true; fi; kill $$PID 2>/dev/null || true; wait $$PID || true; exit $$STATUS'
	@echo "$(GREEN)Docker emulation SMOKE test (no rebuild) complete$(NC)"

## Start emulator container, wait for PTY, run smoke, show logs on failure, cleanup
emu-docker-smoke-seq:
	@echo "$(YELLOW)Starting emulator container and running SMOKE...$(NC)"
	@docker image inspect msio-emu:latest >/dev/null 2>&1 || { echo "$(RED)Image msio-emu:latest not found locally. Run 'make emu-docker-build' first.$(NC)"; exit 1; }
	@docker rm -f msio-emu >/dev/null 2>&1 || true
	@docker run -d --privileged --name msio-emu -v $(shell pwd):/workspace -w /workspace msio-emu:latest \
		bash -lc "/opt/venv/bin/python scripts/emulation/cli.py > /tmp/emu.log 2>&1" >/dev/null
	@echo "Waiting for PTY /tmp/tty-msio ..."
	@for i in $$(seq 1 90); do docker exec msio-emu bash -lc '[ -e /tmp/tty-msio ]' && break; sleep 1; done; \
	 if ! docker exec msio-emu bash -lc '[ -e /tmp/tty-msio ]'; then \
	   echo "$(RED)Emulator PTY not ready after 90s$(NC)"; \
	   echo "--- emulator log (last 200) ---"; \
	   docker exec msio-emu bash -lc 'tail -n 200 /tmp/emu.log || true; ps aux || true'; \
	   docker rm -f msio-emu >/dev/null 2>&1 || true; \
	   exit 1; \
	 fi
	@echo "$(GREEN)PTY is ready. Running smoke...$(NC)"
	@docker exec msio-emu bash -lc '/opt/venv/bin/behave -D profile=simulavr -t @smoke test/acceptance' || STATUS=$$?; STATUS=$${STATUS:-0}; \
	 if [ $$STATUS -ne 0 ]; then \
	   echo "--- emulator log (last 200) ---"; \
	   docker exec msio-emu bash -lc 'tail -n 200 /tmp/emu.log || true; ps aux || true'; \
	 fi; \
	 docker rm -f msio-emu >/dev/null 2>&1 || true; \
	 exit $$STATUS

## Start emulator container with UART↔Modbus bridge, wait for PTY, run smoke, show logs on failure, cleanup
emu-docker-smoke-seq-bridge:
	@echo "$(YELLOW)Starting emulator container with UART↔Modbus bridge and running SMOKE...$(NC)"
	@docker image inspect msio-emu:latest >/dev/null 2>&1 || { echo "$(RED)Image msio-emu:latest not found locally. Run 'make emu-docker-build' first.$(NC)"; exit 1; }
	@docker rm -f msio-emu >/dev/null 2>&1 || true
	@docker run -d --privileged --name msio-emu -e EMU_BRIDGE=1 -v $(shell pwd):/workspace -w /workspace msio-emu:latest \
		bash -lc "/opt/venv/bin/python scripts/emulation/cli.py > /tmp/emu.log 2>&1" >/dev/null
	@echo "Waiting for PTY /tmp/tty-msio ..."
	@for i in $$(seq 1 90); do docker exec msio-emu bash -lc '[ -e /tmp/tty-msio ]' && break; sleep 1; done; \
	 if ! docker exec msio-emu bash -lc '[ -e /tmp/tty-msio ]'; then \
	   echo "$(RED)Emulator PTY not ready after 90s$(NC)"; \
	   echo "--- emulator log (last 200) ---"; \
	   docker exec msio-emu bash -lc 'tail -n 200 /tmp/emu.log || true; ps aux || true'; \
	   docker rm -f msio-emu >/dev/null 2>&1 || true; \
	   exit 1; \
	 fi
	@echo "$(GREEN)PTY is ready. Running smoke...$(NC)"
	@docker exec msio-emu bash -lc '/opt/venv/bin/behave -D profile=simulavr -t @smoke test/acceptance' || STATUS=$$?; STATUS=$${STATUS:-0}; \
	 if [ $$STATUS -ne 0 ]; then \
	   echo "--- emulator log (last 200) ---"; \
	   docker exec msio-emu bash -lc 'tail -n 200 /tmp/emu.log || true; ps aux || true'; \
	 fi; \
	 docker rm -f msio-emu >/dev/null 2>&1 || true; \
	 exit $$STATUS

## Run the SMOKE with RTU server fallback (no rebuild)
emu-docker-run-smoke-rtu:
	@echo "$(YELLOW)Running emulation SMOKE test in Docker with RTU fallback (no rebuild)...$(NC)"
	@docker image inspect msio-emu:latest >/dev/null 2>&1 || { echo "$(RED)Image msio-emu:latest not found locally. Run 'make emu-docker-build' first.$(NC)"; exit 1; }
	@docker run --rm -t --privileged -e EMU_MODE=rtu-server -v $$(pwd):/workspace -w /workspace msio-emu:latest bash -lc 'set -euo pipefail; /opt/venv/bin/python scripts/emulation/cli.py > /tmp/emu.log 2>&1 & PID=$$!; for i in $$(seq 1 30); do [ -e /tmp/tty-msio ] && break; sleep 1; done; if [ ! -e /tmp/tty-msio ]; then echo "RTU server PTY not ready after 30s"; tail -n 200 /tmp/emu.log || true; ps aux || true; exit 1; fi; /opt/venv/bin/behave -D profile=simulavr -t @smoke test/acceptance || STATUS=$$?; STATUS=$${STATUS:-0}; if [ $$STATUS -ne 0 ]; then echo "--- emulator log (last 200) ---"; tail -n 200 /tmp/emu.log || true; ps aux || true; fi; kill $$PID 2>/dev/null || true; wait $$PID || true; exit $$STATUS'

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