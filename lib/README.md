# Libraries Directory

This directory contains reusable libraries for the Multi-Sonicator I/O Controller.

## Structure

- `SonicatorInterface/` - Sonicator control and monitoring library
- `ModbusRTU/` - MODBUS RTU slave implementation
- `SafetyMonitor/` - Safety monitoring and emergency stop library
- `SystemController/` - Main system orchestration library

## Usage

Libraries are automatically included by PlatformIO based on `#include` statements in the source code.

## Development

Each library should have:
- `library.json` - Library metadata
- `src/` - Source files
- `include/` - Public header files
- `examples/` - Usage examples
- `test/` - Library-specific tests