# Comprehensive Scenario Execution Guide
## Multi-Sonicator I/O Controller Testing Framework

**Version:** 1.0  
**Date:** 2025-09-04  
**Total Scenarios:** 155 scenarios across 31 feature files

---

## 🎯 **QUICK START - EXECUTE ALL READY SCENARIOS**

### **Run All Implemented Scenarios (119 scenarios)**
```bash
# Execute all scenarios that are ready for testing
make test-acceptance --tags="not @pending"

# Alternative: Use behave directly
cd test/acceptance
behave --tags="not @pending" features/
```

### **Run Specific Test Categories**
```bash
# Hardware-in-the-Loop tests (45 scenarios)
make test-acceptance --tags="@hil and not @pending"

# System integration tests (25 scenarios)  
make test-acceptance --tags="@system and not @pending"

# MODBUS interface tests (20 scenarios)
make test-acceptance --tags="@modbus and not @pending"

# Safety and emergency tests (8 scenarios)
make test-acceptance --tags="@safety and not @pending"

# Performance and timing tests (6 scenarios)
make test-acceptance --tags="@performance and not @pending"
```

---

## 📋 **SCENARIO CATEGORIES AND EXECUTION**

### **1. CORE SYSTEM TESTING (119 Ready Scenarios)**

#### **Hardware-in-the-Loop (HIL) - 45 Scenarios**
```bash
# Basic connectivity and power
behave --tags="@connectivity" features/

# GPIO functionality  
behave --tags="@gpio" features/

# ADC measurement verification
behave --tags="@adc" features/

# PWM generation testing
behave --tags="@pwm" features/

# MODBUS communication
behave --tags="@modbus" features/

# Power supply verification
behave --tags="@power" features/
```

#### **System Integration - 25 Scenarios**
```bash
# Complete system startup and operation
behave --tags="@system and @integration" features/system_integration_complete.feature

# Complete MODBUS register interface
behave --tags="@modbus and @interface" features/complete_modbus_interface.feature
```

#### **Functional Testing - 35 Scenarios**
```bash
# Individual sonicator control
behave --tags="@amplitude or @start-stop" features/

# System metrics and monitoring
behave --tags="@system-metrics" features/

# Safety systems
behave --tags="@safety" features/

# Communication protocols
behave --tags="@communication" features/
```

#### **Configuration and Validation - 14 Scenarios**
```bash
# Configuration validation
behave --tags="@config" features/

# CI/CD integration
behave --tags="@ci" features/

# Timing validation
behave --tags="@timing" features/
```

### **2. ADVANCED SYSTEM TESTING (36 Pending Scenarios)**

#### **Production Validation - 13 Scenarios**
```bash
# When production features are implemented:
behave --tags="@production and @validation" features/production_validation.feature
```

#### **End-to-End Workflows - 12 Scenarios**  
```bash
# When workflow features are implemented:
behave --tags="@workflow and @production" features/end_to_end_workflows.feature
```

#### **Regulatory Compliance - 14 Scenarios**
```bash
# When compliance features are implemented:
behave --tags="@compliance and @regulatory" features/regulatory_compliance.feature
```

#### **Advanced Features - 14 Scenarios**
```bash
# When advanced features are implemented:
behave --tags="@advanced" features/advanced_features.feature
```

---

## 🔧 **SCENARIO MANAGEMENT COMMANDS**

### **Analyze Scenario Status**
```bash
# Get overview of all scenarios
make manage-pending-scenarios

# Update @pending tags for newly implemented features
make update-pending-scenarios

# Validate BDD syntax for all scenarios
cd test/acceptance && behave --dry-run features/
```

### **Selective Testing by Implementation Status**
```bash
# Test only implemented functionality
behave --tags="not @pending" features/

# Test specific modules
behave --tags="@hil and not @pending" features/
behave --tags="@modbus and not @pending" features/
behave --tags="@system and not @pending" features/

# Test by priority
behave --tags="@critical and not @pending" features/
behave --tags="@safety and not @pending" features/
```

---

## 📊 **EXECUTION STRATEGIES**

### **Development Testing Strategy**
1. **Daily:** Run smoke tests and critical scenarios
   ```bash
   behave --tags="@smoke" features/
   behave --tags="@critical and not @pending" features/
   ```

2. **Weekly:** Run full implemented test suite
   ```bash
   make test-acceptance --tags="not @pending"
   ```

3. **Release:** Run comprehensive validation
   ```bash
   make ci-test  # Full CI pipeline including all ready scenarios
   ```

### **Feature Development Strategy**
1. **Before Implementation:** Review pending scenarios for the feature
   ```bash
   behave --tags="@feature-name and @pending" --dry-run features/
   ```

2. **During Implementation:** Remove @pending tags as functionality is completed
   ```bash
   # Edit feature files to remove @pending tags
   make update-pending-scenarios
   ```

3. **After Implementation:** Validate all scenarios for the feature
   ```bash
   behave --tags="@feature-name and not @pending" features/
   ```

---

## 🎯 **SCENARIO EXECUTION BY FEATURE AREA**

### **Sonicator Control Features**
```bash
# Amplitude control (4 scenarios)
behave features/amplitude_control.feature

# Start/stop control (4 scenarios)  
behave features/start_stop.feature

# Overload management (2 scenarios)
behave --tags="@overload" features/

# Multi-sonicator coordination (6 scenarios)
behave --tags="@system and @integration" features/system_integration_complete.feature
```

### **Communication Features**
```bash
# MODBUS protocol (20 scenarios)
behave features/complete_modbus_interface.feature

# Basic communication (2 scenarios)
behave features/communication.feature

# Error handling (6 scenarios)
behave --tags="@error-handling" features/
```

### **Safety Features**
```bash
# Emergency stop (4 scenarios)
behave --tags="@safety" features/

# Watchdog (2 scenarios)
behave --tags="@watchdog" features/

# Fault detection (4 scenarios)
behave --tags="@fault-handling" features/
```

### **Monitoring Features**
```bash
# System metrics (4 scenarios)
behave features/system.feature

# Power monitoring (6 scenarios)
behave --tags="@power" features/

# Frequency monitoring (2 scenarios)
behave --tags="@frequency" features/
```

---

## 🚀 **CONTINUOUS INTEGRATION EXECUTION**

### **CI Pipeline Stages**
```bash
# Stage 1: Configuration validation
make validate-config

# Stage 2: Unit tests with coverage
make test-unit

# Stage 3: BDD syntax validation
cd test/acceptance && behave --dry-run features/

# Stage 4: Implemented scenario execution
make test-acceptance --tags="not @pending"

# Stage 5: Generate reports
make generate-traceability-report
```

### **Local CI Simulation**
```bash
# Run complete local CI pipeline
make ci-local

# Run specific CI stages
python3 scripts/ci_test_runner.py --stage validation
python3 scripts/ci_test_runner.py --stage unit
python3 scripts/ci_test_runner.py --stage acceptance
```

---

## 📈 **PROGRESS TRACKING**

### **Implementation Progress**
- **Current Status:** 119/155 scenarios ready (76.8%)
- **Pending Implementation:** 36/155 scenarios (23.2%)
- **Coverage:** All core functionality covered

### **Feature Implementation Roadmap**
1. **Phase 1 (Complete):** Core system functionality - 119 scenarios
2. **Phase 2 (Planned):** Production validation - 13 scenarios  
3. **Phase 3 (Planned):** Operational workflows - 12 scenarios
4. **Phase 4 (Future):** Regulatory compliance - 14 scenarios
5. **Phase 5 (Future):** Advanced features - 14 scenarios

### **Monitoring Commands**
```bash
# Check implementation progress
python3 scripts/manage_pending_scenarios.py

# Generate progress reports
make generate-traceability-report

# Validate test coverage
make test-unit  # Check unit test coverage
```

---

## 🔍 **TROUBLESHOOTING SCENARIO EXECUTION**

### **Common Issues**
1. **Hardware Not Available:** Use `--tags="not @hil"` to skip hardware tests
2. **Missing Dependencies:** Run `make check-deps` to verify setup
3. **Configuration Issues:** Run `make validate-config` to check setup
4. **Syntax Errors:** Run `behave --dry-run features/` to validate

### **Debug Commands**
```bash
# Verbose execution with detailed output
behave -v --tags="@scenario-tag" features/

# Execute single scenario for debugging
behave features/feature_file.feature:line_number

# Check scenario step definitions
behave --steps-catalog features/
```

---

**This comprehensive scenario suite provides complete testing coverage for the Multi-Sonicator I/O Controller system, from basic hardware validation through advanced production workflows and regulatory compliance. Execute scenarios based on your current implementation status and testing needs.**
