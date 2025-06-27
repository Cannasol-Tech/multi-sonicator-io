# Changelog

All notable changes to the Multi Sonicator I/O Controller project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial project setup for Multi Sonicator I/O Controller
- PlatformIO configuration for ATmega32A with Arduino framework
- Arduino Uno R4 WiFi as ISP programmer support
- Basic project structure with src/, include/, lib/, test/, docs/ directories
- Git version control setup with comprehensive .gitignore
- Agent framework directory structure for task automation

### Configuration
- ATmega32A @ 16MHz external crystal
- MightyCore board package for Arduino compatibility
- Custom upload protocol for Arduino as ISP
- Fuse settings: LFUSE=0xFF, HFUSE=0xD9, EFUSE=0xFF
- Development and production build environments

### Hardware Support
- Arduino Uno R4 WiFi as ISP programmer
- ISP wiring configuration documented
- ATmega32A pin mapping for DB9 interfaces
- Support for up to 4 sonicator units

## [0.1.0] - 2025-06-27

### Added
- Project initialization
- Phase 1.1 Initial Setup implementation complete
