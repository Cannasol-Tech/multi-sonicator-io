#!/bin/bash
"""
Setup Simulation Environment for Multi-Sonicator Testing
Prepares the virtual hardware simulation framework
"""

set -e

echo "Setting up Multi-Sonicator Simulation Environment"
echo "=================================================="

# Create simulation directories
mkdir -p test_results/simulation
mkdir -p reports/simulation

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "PlatformIO not found, please install it first"
    exit 1
fi

# Compile simulation components
echo "Compiling simulation components..."
pio run -e test_desktop --target clean
pio run -e test_desktop

# Run basic simulation tests
echo "Running basic simulation tests..."
pio test -e test_desktop --filter test_virtual_sonicator --verbose

# Generate simulation test reports
echo "Generating simulation reports..."
if [ -f "scripts/ci/generate_reports.py" ]; then
    python scripts/ci/generate_reports.py --test-results test_results --output-dir reports/simulation
fi

echo "Simulation environment setup complete"
echo "Available test filters:"
echo "  - test_virtual_sonicator"
echo "  - test_simulation_integration"
echo "  - test_config"
echo "  - test_sonicator"
echo "  - test_system"

echo ""
echo "Usage examples:"
echo "  pio test -e test_desktop --filter test_virtual_sonicator"
echo "  pio test -e test_desktop --filter test_simulation_integration"
echo "  pio test -e test_desktop --verbose"