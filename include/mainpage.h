/**
 * @mainpage Multi-Sonicator I/O Controller Documentation
 *
 * @section intro_sec Introduction
 *
 * The **Multi-Sonicator I/O Controller** is an industrial automation controller developed by Cannasol Technologies
 * for simultaneous control of up to 4 CT2000 sonicators. This ATmega32A-based system provides seamless integration
 * with existing Velocio PLC and HMI systems through MODBUS RTU communication, enabling precise multi-unit
 * sonicator orchestration for industrial applications.
 *
 * @section features Key Features
 *
 * - **Multi-Unit Control**: Simultaneous management of up to 4 CT2000 sonicators with coordinated operations
 * - **Industrial Integration**: Robust MODBUS RTU communication with existing automation infrastructure
 * - **DB9 Interface Topology**: Five dedicated DB9 connectors for external system integration
 * - **Safety Systems**: Comprehensive monitoring, alarm management, and emergency stop capabilities
 * - **Cloud Connectivity**: Firebase integration via ESP32 for remote monitoring and data logging
 * - **Hardware-in-the-Loop Testing**: Complete test harness for validation without production hardware
 * - **Web UI**: Modern web interface for monitoring, control, and testing operations
 *
 * @section arch_sec System Architecture
 *
 * @subsection hw_arch Hardware Architecture
 *
 * The system centers around an ATmega32A microcontroller operating at 16MHz with the following key interfaces:
 *
 * - **DB9-0/DB9-5**: Main automation system communication (MODBUS RTU)
 * - **DB9-1 through DB9-4**: Individual sonicator control interfaces
 * - **Power**: 24VDC input with onboard regulation (12V/5V)
 * - **Programming**: Arduino-as-ISP interface for firmware updates
 *
 * Each sonicator interface provides:
 * - Start/Stop control signals
 * - Amplitude control (0-100%)
 * - Frequency monitoring and lock detection
 * - Overload protection and reset
 * - Power sense feedback
 *
 * @subsection sw_arch Software Architecture
 *
 * The firmware is organized into modular components for maintainability and extensibility:
 *
 * - **Communication Layer**: MODBUS RTU protocol implementation with register mapping
 * - **Control Layer**: Multi-unit sonicator coordination and state management
 * - **HAL (Hardware Abstraction Layer)**: Platform-independent hardware interface
 * - **Safety Layer**: Emergency stop, fault detection, and isolation mechanisms
 *
 * @section quick_start Quick Start Guide
 *
 * @subsection hw_setup Hardware Setup
 *
 * 1. **Microcontroller**: ATmega32A with 16MHz external crystal
 * 2. **Programmer**: Arduino Uno R4 WiFi configured as ISP
 * 3. **Power Supply**: 24VDC from main automation system via DB9
 * 4. **Connections**: Wire DB9 connectors according to hardware configuration
 *
 * @subsection sw_setup Software Development Setup
 *
 * 1. **Install PlatformIO** with VSCode extension
 * 2. **Clone Repository** and navigate to project directory
 * 3. **Install Dependencies**:
 *    @code{.bash}
 *    make install-deps
 *    @endcode
 * 4. **Build Project**:
 *    @code{.bash}
 *    make build
 *    @endcode
 * 5. **Upload Firmware**:
 *    @code{.bash}
 *    make upload-to-device
 *    @endcode
 *
 * @subsection web_ui_setup Web UI and Testing
 *
 * For development and testing with the web interface:
 *
 * @code{.bash}
 * # Install web dependencies
 * make web-ui-install
 *
 * # Start development servers
 * make web-ui-dev
 *
 * # Run tests with coverage
 * make web-ui-test
 * @endcode
 *
 * @section modules_sec Core Modules
 *
 * @subsection comm_mod Communication Module
 *
 * Handles all MODBUS RTU communication with the main automation system:
 * - Register read/write operations
 * - Protocol error handling
 * - Response time optimization (<100ms target)
 *
 * @ref modbus_register_manager.h
 * @ref modbus_registers.h
 *
 * @subsection control_mod Control Module
 *
 * Manages multi-unit sonicator coordination and individual unit control:
 * - Coordinated start/stop operations
 * - Per-unit state management
 * - Fault isolation and recovery
 * - Amplitude and timing control
 *
 * @ref Multiplexer.h
 * @ref MultiSonicatorSystem.h
 * @ref sonicator_modbus_bridge.h
 *
 * @subsection hal_mod Hardware Abstraction Layer
 *
 * Provides platform-independent hardware interface:
 * - GPIO management
 * - Timer and PWM control
 * - ADC operations
 * - UART communication
 *
 * @ref hal.h
 *
 * @section config_sec Configuration
 *
 * @subsection hw_config Hardware Configuration
 *
 * The authoritative source for all hardware mappings and pin assignments:
 * @ref config/hardware-config.yaml
 *
 * This file defines:
 * - DB9 connector pin mappings
 * - ATmega32A pin assignments
 * - Signal routing and connectivity
 * - Test harness configurations
 *
 * @subsection sw_config Software Configuration
 *
 * Key configuration parameters defined in headers:
 * - MODBUS communication settings
 * - Timing and timeout values
 * - Safety thresholds and limits
 * - System constants and constraints
 *
 * @section safety_sec Safety and Reliability
 *
 * The system implements multiple layers of safety protection:
 *
 * - **Emergency Stop**: Immediate shutdown capability across all units
 * - **Fault Detection**: Automatic detection of hardware faults and overloads
 * - **Isolation**: Faulty units isolated while others continue operation
 * - **Monitoring**: Continuous telemetry and status reporting
 * - **Reset Mechanisms**: Automatic and manual overload reset capabilities
 *
 * @section testing_sec Testing and Validation
 *
 * Comprehensive testing framework supporting multiple test levels:
 *
 * - **Unit Tests**: Individual component validation
 * - **Integration Tests**: Module interaction verification
 * - **Hardware-in-the-Loop (HIL)**: Full system testing with Arduino test harness
 * - **Acceptance Tests**: End-to-end behavioral validation
 *
 * @section docs_sec Documentation Navigation
 *
 * - @ref page_quick_start "Quick Start Guide"
 * - @ref page_development_setup "Development Environment Setup"
 * - @ref page_testing_guide "Testing Framework Guide"
 * - @ref page_troubleshooting "Troubleshooting Guide"
 * - @ref page_api_reference "API Reference"
 *
 * @section standards_sec Development Standards
 *
 * This project follows Agile Flow company standards:
 * - @ref docs/sop/coding-style.md "Coding Style Standards"
 * - @ref docs/sop/project-structure.md "Project Structure Standards"
 * - @ref docs/sop/sw-testing.md "Software Testing Standards"
 *
 * @section contact_sec Contact and Support
 *
 * - **Project Lead**: Stephen Boyett
 * - **Hardware Advisor**: Josh (CEO)
 * - **Company**: Cannasol Technologies
 * - **Repository**: https://github.com/cannasol-technologies/multi-sonicator-io
 *
 * @section version_sec Version Information
 *
 * - **Current Version**: 2.0.0
 * - **Last Updated**: September 2025
 * - **Doxygen Documentation**: Auto-generated from source comments
 *
 * @author Stephen Boyett
 * @date 2025-09-17
 * @version 2.0.0
 * @copyright 2025 Cannasol Technologies. All rights reserved.
 */
