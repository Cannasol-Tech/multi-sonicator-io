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

The web UI maintains ≥85% unit test coverage using pytest for consistency with existing test infrastructure.

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

## Deployment

### Development Deployment

1. **Install Dependencies**:
   ```bash
   make web-ui-install
   ```

2. **Start Development Servers**:
   ```bash
   make web-ui-dev
   ```
   - Frontend: http://localhost:3101
   - Backend API: http://localhost:3001/api

3. **Run Tests**:
   ```bash
   make web-ui-test
   ```

### Production Deployment

1. **Build Production Assets**:
   ```bash
   make web-ui-build
   ```

2. **Production Server Setup**:
   ```bash
   # Install production dependencies
   cd web-ui/backend && npm ci --production
   cd web-ui/frontend && npm ci --production

   # Start production server
   cd web-ui/backend && npm start
   ```

3. **Environment Variables**:
   ```bash
   export NODE_ENV=production
   export PORT=3001
   export FRONTEND_PORT=3101
   ```

### Sandbox Mode Deployment

For complete hardware testing environment:

```bash
make web-ui-sandbox
```

This comprehensive command:
1. Builds latest production firmware (PlatformIO)
2. Auto-detects Arduino hardware port
3. Uploads Arduino as ISP sketch
4. Programs ATmega32A DUT with production firmware
5. Uploads Arduino Test Harness firmware
6. Starts web UI backend with HIL integration
7. Launches frontend development server
8. Opens browser to interface
9. Verifies hardware connectivity

### Docker Deployment (Coming Soon)

Docker support is planned for easier deployment and development environment consistency.

## Documentation

- **[API Documentation](docs/API.md)** - Complete REST API reference
- **[User Guide](docs/USER_GUIDE.md)** - End-user interface documentation
- **[Architecture Decision Record](docs/ADR-001-Integration-Architecture.md)** - Technical architecture decisions

## Troubleshooting

### Common Issues

**Port Conflicts**:
```bash
# Kill processes on web UI ports
make web-ui-stop
```

**Hardware Connection Issues**:
```bash
# Check hardware detection
python3 scripts/detect_hardware.py

# Manual reconnection
curl -X POST http://localhost:3001/api/connection/reconnect
```

**Test Coverage Issues**:
```bash
# Run tests with verbose output
web-ui/venv/bin/python -m pytest web-ui/tests/ -v --cov-report=term-missing
```

**Build Failures**:
```bash
# Clean and rebuild
make web-ui-clean
make web-ui-install
make web-ui-build
```
