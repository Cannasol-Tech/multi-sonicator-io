<!--
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Title: Company Wide Makefile Target Standards
* Author: Stephen Boyett
* Company: Axovia AI
* Date: 2025-09-06
* Brief: Company-wide standard for Makefile targets and required behaviors.
* Version: 1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-->

# Company Wide Makefile Target Standards

## Purpose
Ensure all makefile targets are consistent and follow company standards.  Reduce time spent context switching between projects by standardizing makefile targets so that developers can easily switch between projects without having to remember the specific makefile targets for each project.  When working in a Makefile, always refer and strictly adhere to the standards outlined in this document.  If a makefile target is not listed in this document, it is not currently allowed.  The developer should create a pull request to Axovia-AI/axovia-flow to add the new makefile target to this document in the governing repository.  

## Source of Truth

This document is the single source of truth for all makefile targets utilized across the company.  This file is the single source of truth for all pin capabilities utilized across the Nexus Lens Embedded Tester Project.

## Makefile Target Standards

### Update Standards 

The update-standards make target is meant to align all of the Company projects so that they all adhere to the same exact Standard Operating Procedure (SOP) documents.  This target is meant to be run on a regular basis to ensure that all projects are in compliance with the latest standards.  This target should be run at least once a week for all active development projects and at least once a month for all maintenance-mode projects.  This target should also be run before any major releases to ensure that the project is in compliance with the latest standards.

***make update-standards:*** This command should always:
  1. Synchronize the repository with the Single Sole Source of truth: `Axovia-AI/axovia-flow/.axovia-flow/company-standards/`
  2. Update any necessary files to ensure that the makefile is in compliance with the company standards
  3. Update any necessary files to ensure that the project is in compliance with the latest standards

### All

***make all:*** This command should always: 
  1. Clean the project to ensure an empty slate 
  2. Install all dependencies and ensure that the environment is ready for a production build.
  3. Build the latest source code as a production build
  4. Run all of the unit tests and save the coverage-summary.json in `final/` directory per the `.axovia-flow/company-standards/release-format.md`
  5. Run all of the acceptance tests and save the executive-report.json in `final/` directory per the `.axovia-flow/company-standards/release-format.md`
  5.1 Run any other tests that are present for the given repository and save any generated reports in the correct documents per the `.axovia-flow/company-standards/release-format.md`
  6. Run `make preview` to launch a fully functioning local preview of the application
    - ***Note:*** The `make preview` command should always launch all necessary local serversckto have a fully functioning locally run preview of the project.  
  7. 

### Clean

***make clean:*** This command should always:
  1. Remove all generated files and caches
  2. Delete Python bytecode files (*.pyc, *.pyo)
  3. Remove temporary files and directories
  4. Clean up test artifacts and coverage reports

**Axovia Flow Implementation:**
```make
clean:
	@echo "Cleaning up generated files..."
	rm -rf __pycache__/ .pytest_cache/ .coverage htmlcov/
	find . -name "*.pyc" -delete
	find . -name "*.pyo" -delete
	find . -name "*~" -delete
```

### Build

***make build:*** This command should always:
  1. Compile or prepare the application for deployment
  2. Generate any necessary artifacts
  3. Validate the build process

**Note:** Currently not implemented in Axovia Flow as it's a Python-based orchestrator that doesn't require compilation.

### Install

***make install:*** This command should always:
  1. Install all project dependencies
  2. Set up the development environment
  3. Create necessary directories and configuration files
  4. Prepare the system for development or deployment

**Axovia Flow Implementation:**
```make
install:
	@echo "Installing Axovia Flow dependencies..."
	pip install -r requirements.txt
	@echo "Creating necessary directories..."
	mkdir -p state schemas tests
	@echo "Installation complete!"
```

### Uninstall

***make uninstall:*** This command should always:
  1. Remove installed dependencies (where applicable)
  2. Clean up created directories and files
  3. Restore the system to pre-installation state

**Note:** Currently not implemented in Axovia Flow.

## Axovia Flow™ Project Specific Targets

The following targets are implemented in the Axovia Flow project and serve as reference implementations for similar orchestrator-based projects:

### Help and Documentation

***make help:*** (Default target)
  - Display all available make targets with descriptions
  - Provide usage examples
  - Organize targets by category (Setup, Development, Operations)

### Testing Targets

***make test:*** Run unit tests with coverage reporting
```make
test:
	@echo "Running tests with coverage..."
	PYTHONPATH=src python3 -m pytest tests/ --cov=src/orchestrator --cov=src/director --cov-report=html --cov-report=term-missing
```

***make test-acceptance:*** Run behavioral/acceptance tests
```make
test-acceptance:
	@echo "Running acceptance tests..."
	PYTHONPATH=src behave -c config/testing/behave.ini
```

***make test-reports:*** Generate comprehensive test reports
```make
test-reports:
	@echo "Generating test reports..."
	python3 scripts/generate_test_reports.py
```

***make quality-gate:*** Run quality gate validation
```make
quality-gate:
	@echo "Running quality gate validation..."
	python3 scripts/quality_gate.py
```

***make test-all:*** Run complete test suite with reports and quality validation
```make
test-all: test test-acceptance test-reports quality-gate
	@echo "Complete test suite with reports and quality gate validation completed."
```

### Code Quality Targets

***make lint:*** Run code linting and type checking
```make
lint:
	@echo "Running code linting..."
	PYTHONPATH=src python3 -m flake8 src/orchestrator/ src/director_of_flow/ tests/
	PYTHONPATH=src python3 -m mypy src/orchestrator/ src/director_of_flow/
```

***make format:*** Format code using black formatter
```make
format:
	@echo "Formatting code with black..."
	PYTHONPATH=src python3 -m black src/orchestrator/ src/director_of_flow/ tests/
```

### Orchestrator Operations

***make axovia-run:*** Start the Axovia Flow orchestrator service
```make
axovia-run:
	@echo "Starting Axovia Flow Orchestrator..."
	PYTHONPATH=src python -m orchestrator.cli run
```

***make axovia-status:*** Check orchestrator service status
```make
axovia-status:
	@echo "Checking Axovia Flow Orchestrator status..."
	PYTHONPATH=src python -m orchestrator.cli status
```

***make axovia-story:*** Enqueue a story for processing (requires STORY parameter)
```make
axovia-story:
	@if [ -z "$(STORY)" ]; then \
		echo "Error: STORY parameter required. Usage: make axovia-story STORY=S-123"; \
		exit 1; \
	fi
	@echo "Enqueueing story: $(STORY)"
	PYTHONPATH=src python -m orchestrator.cli story $(STORY)
```

### Development Targets

***make dev:*** Start development server with auto-reload and verbose logging
```make
dev:
	@echo "Starting development server with auto-reload..."
	PYTHONPATH=src python -m orchestrator.cli run --verbose
```

***make init:*** Initialize Axovia Flow configuration
```make
init:
	@echo "Initializing Axovia Flow configuration..."
	PYTHONPATH=src python -m orchestrator.cli init
```

***make validate:*** Validate Axovia Flow configuration
```make
validate:
	@echo "Validating Axovia Flow configuration..."
	PYTHONPATH=src python -m orchestrator.cli validate
```

### Future Implementation Targets

The following targets are planned but not yet implemented:

***make db-migrate:*** Run database migrations
***make db-reset:*** Reset database to initial state
***make docker-build:*** Build Docker container
***make docker-run:*** Run application in Docker container

## Usage Examples

```bash
# Setup and installation
make install

# Development workflow
make format
make lint
make test-all

# Run the orchestrator
make axovia-run

# Process a specific story
make axovia-story STORY=S-123

# Development with auto-reload
make dev

# Clean up
make clean
```

## Standards Compliance

All Makefile targets in Axovia Flow projects must:
1. Include descriptive echo statements
2. Use consistent naming conventions
3. Set appropriate PYTHONPATH when needed
4. Handle errors gracefully with proper exit codes
5. Provide clear usage instructions for parameterized targets
