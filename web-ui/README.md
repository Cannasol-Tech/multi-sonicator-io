# Multi-Sonicator-IO Web UI

## Overview

Web-based interface for the Arduino Test Harness providing interactive testing capabilities for the ATmega32A (DUT) sonicator multiplexer. This interface operates in full sandbox mode with continuous monitoring and control of all hardware pins.

## Architecture

```markdown
web-ui/
├── frontend/              # React + TypeScript + Vite
│   ├── src/
│   │   ├── components/    # Reusable UI components
│   │   ├── pages/         # Main application pages
│   │   ├── hooks/         # Custom React hooks
│   │   ├── services/      # API communication services
│   │   ├── types/         # TypeScript type definitions
│   │   ├── styles/        # CSS and styling
│   │   └── utils/         # Utility functions
│   ├── public/            # Static assets
│   └── package.json
├── backend/               # Node.js + Express + WebSocket
│   ├── src/
│   │   ├── routes/        # API endpoints
│   │   ├── services/      # Business logic
│   │   ├── adapters/      # HIL framework integration
│   │   └── websocket/     # Real-time communication
│   └── package.json
└── shared/                # Common types and utilities
```

## Key Features

- **Single Page Application**: No navigation, centralized hardware diagram interface
- **Real-time Pin Monitoring**: Display current state of ALL monitored pins on DUT
- **Interactive Hardware Diagram**: Visual representation using Arduino and ATmega32A icons
- **Arduino Command History**: Real-time tracking of the last 5 commands sent and responses received
- **Sandbox Mode Operation**: Continuous configuration and monitoring loop
- **Comprehensive Help System**: Context-sensitive help for new users
- **Hardware Control**: GPIO, ADC, PWM, and power management controls

## Quick Start

### Launch in Sandbox Mode
```bash
make web-ui-sandbox
```
This command will:
1. Build and upload latest production firmware to ATmega32A via Arduino ISP
2. Launch the web UI backend with HIL integration
3. Start the frontend development server
4. Open the interface in your default browser

### Development Mode
```bash
make web-ui-dev
```

### Production Build
```bash
make web-ui-build
```

## Pin Matrix Integration

The interface uses `docs/planning/pin-matrix.md` as the SOLE SOURCE OF TRUTH for:
- Pin assignments between Arduino Test Wrapper and ATmega32A DUT
- Signal names and directions
- Hardware connection mapping
- Test point references

## Hardware Assets

Visual hardware representation uses:
- `resource/arduino-uno-r3-icon.png` - Arduino Test Wrapper
- `resource/atmega-32-a-icon.png` - ATmega32A DUT

## Testing

The web UI maintains 90% unit test coverage using pytest for consistency with existing test infrastructure.

```bash
# Run web UI tests
make test-web-ui
```

## Integration

The web UI integrates with existing HIL framework components:
- `test/acceptance/hil_framework/hardware_interface.py`
- `scripts/hil_serial.py`
- `test/acceptance/hil_framework/hil_config.yaml`

## Requirements

- Node.js 18+
- Python 3.8+ (for HIL integration)
- Arduino CLI (for firmware programming)
- PlatformIO (for firmware builds)

## Development Guidelines

- **Functionality-First**: Focus on core functionality over aesthetics
- **Real-time Performance**: Hardware commands must respond within 100ms
- **Error Handling**: Comprehensive error messages with suggested actions
- **Help Integration**: Context-sensitive help for all interface elements
