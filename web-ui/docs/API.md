# Multi-Sonicator-IO Web UI API Documentation

## Overview

The Multi-Sonicator-IO Web UI provides a comprehensive REST API for controlling and monitoring the Arduino Test Harness and ATmega32A DUT (Device Under Test). The API supports real-time hardware control, test automation, and system monitoring.

**Base URL**: `http://localhost:3001/api`  
**WebSocket**: `ws://localhost:3001/ws`  
**API Version**: 1.1.0

## Authentication

Currently, the API does not require authentication as it's designed for local development and testing environments.

## Response Format

All API responses follow a consistent JSON format:

```json
{
  "success": true,
  "data": { ... },
  "timestamp": 1641234567890,
  "error": "Error message (if applicable)"
}
```

## Hardware Control Endpoints

### Health Check

**GET** `/api/health`

Returns system health status and hardware connection information.

**Response:**
```json
{
  "status": "ok",
  "timestamp": 1641234567890,
  "hardware": {
    "connected": true,
    "port": "/dev/ttyUSB0"
  }
}
```

### Pin State Management

#### Get All Pin States

**GET** `/api/pins`

Returns current state of all monitored pins.

**Response:**
```json
{
  "pins": {
    "RESET": {
      "signal": "RESET",
      "arduino_pin": 2,
      "direction": "IN",
      "state": "HIGH",
      "value": 1,
      "timestamp": 1641234567890
    }
  },
  "timestamp": 1641234567890
}
```

#### Get Specific Pin State

**GET** `/api/pins/:signal`

Returns state of a specific pin by signal name.

**Parameters:**
- `signal` (string): Pin signal name (e.g., "RESET", "MOSI", "SCK")

**Response:**
```json
{
  "pin": {
    "signal": "RESET",
    "arduino_pin": 2,
    "direction": "IN",
    "state": "HIGH",
    "value": 1,
    "timestamp": 1641234567890
  },
  "timestamp": 1641234567890
}
```

#### Control Pin State

**POST** `/api/pins/:signal`

Controls the state of a specific input pin.

**Parameters:**
- `signal` (string): Pin signal name

**Request Body:**
```json
{
  "state": "HIGH"
}
```

**Response:**
```json
{
  "success": true,
  "signal": "RESET",
  "state": "HIGH",
  "result": {
    "success": true,
    "response": "OK"
  },
  "timestamp": 1641234567890
}
```

### Hardware Commands

#### Send Generic Command

**POST** `/api/command`

Sends a generic command to the hardware interface.

**Request Body:**
```json
{
  "command": "read_adc",
  "args": ["A0"]
}
```

**Response:**
```json
{
  "command": "read_adc",
  "args": ["A0"],
  "response": {
    "success": true,
    "data": "512"
  },
  "timestamp": 1641234567890
}
```

#### Ping Hardware

**POST** `/api/ping`

Tests communication with the hardware interface.

**Response:**
```json
{
  "success": true,
  "responseTime": 45,
  "result": {
    "success": true,
    "response": "PONG"
  },
  "timestamp": 1641234567890
}
```

### Connection Management

#### Get Connection Status

**GET** `/api/connection`

Returns current hardware connection status.

**Response:**
```json
{
  "connected": true,
  "port": "/dev/ttyUSB0",
  "timestamp": 1641234567890
}
```

#### Reconnect Hardware

**POST** `/api/connection/reconnect`

Attempts to reconnect to the hardware interface.

**Response:**
```json
{
  "success": true,
  "connected": true,
  "port": "/dev/ttyUSB0",
  "timestamp": 1641234567890
}
```

#### Retry Connection

**POST** `/api/connection/retry`

Retries connection with multiple attempts.

**Request Body:**
```json
{
  "maxAttempts": 3,
  "delayMs": 2000
}
```

**Response:**
```json
{
  "success": true,
  "connected": true,
  "port": "/dev/ttyUSB0",
  "attempts": 3,
  "timestamp": 1641234567890
}
```

### Configuration Management

#### Get Configuration

**GET** `/api/config`

Returns current hardware configuration.

**Response:**
```json
{
  "success": true,
  "config": {
    "pins": { ... },
    "settings": { ... }
  },
  "timestamp": 1641234567890
}
```

#### Update Configuration

**POST** `/api/config`

Updates hardware configuration.

**Request Body:**
```json
{
  "config": {
    "pins": { ... },
    "settings": { ... }
  }
}
```

**Response:**
```json
{
  "success": true,
  "config": { ... },
  "timestamp": 1641234567890
}
```

## Test Automation Endpoints

### BDD Test Scenarios

#### Get Available Scenarios

**GET** `/api/test/scenarios`

Returns all available BDD test scenarios.

**Response:**
```json
{
  "scenarios": [
    {
      "name": "GPIO Pin Control Test",
      "description": "Test GPIO pin control functionality",
      "tags": ["gpio", "basic"],
      "feature": "hil_gpio_functionality.feature",
      "pins": ["RESET", "MOSI"]
    }
  ],
  "count": 1,
  "timestamp": 1641234567890
}
```

#### Search Scenarios

**GET** `/api/test/scenarios/search?q=query`

Searches scenarios by text query.

**Query Parameters:**
- `q` (string): Search query for scenario name, description, or tags

**Response:**
```json
{
  "scenarios": [ ... ],
  "query": "gpio",
  "count": 5,
  "timestamp": 1641234567890
}
```

#### Filter Scenarios by Tags

**GET** `/api/test/scenarios/tags/:tags?logic=and|or`

Filters scenarios by tags with AND/OR logic.

**Parameters:**
- `tags` (string): Comma-separated list of tags

**Query Parameters:**
- `logic` (string): "and" or "or" (default: "and")

**Response:**
```json
{
  "scenarios": [ ... ],
  "tags": ["gpio", "basic"],
  "logic": "AND",
  "count": 3,
  "timestamp": 1641234567890
}
```

### Test Execution

#### Execute Test Scenarios

**POST** `/api/test/execute`

Executes selected test scenarios.

**Request Body:**
```json
{
  "scenarios": ["GPIO Pin Control Test", "ADC Reading Test"],
  "execution_id": "exec_1641234567890"
}
```

**Response:**
```json
{
  "success": true,
  "execution_id": "exec_1641234567890",
  "scenarios": ["GPIO Pin Control Test", "ADC Reading Test"],
  "message": "Test execution started",
  "timestamp": 1641234567890
}
```

#### Get Execution Status

**GET** `/api/test/execution`

Returns current test execution status.

**Response:**
```json
{
  "execution": {
    "id": "exec_1641234567890",
    "status": "running",
    "progress": 0.6,
    "scenarios": [ ... ],
    "current_scenario": "ADC Reading Test",
    "start_time": 1641234567890
  },
  "in_progress": true,
  "timestamp": 1641234567890
}
```

#### Stop Test Execution

**POST** `/api/test/stop`

Stops current test execution.

**Response:**
```json
{
  "success": true,
  "message": "Test execution stopped",
  "timestamp": 1641234567890
}
```

### Tags and Organization

#### Get Available Tags

**GET** `/api/test/tags`

Returns all available test tags.

**Response:**
```json
{
  "tags": ["gpio", "adc", "pwm", "modbus", "basic", "advanced"],
  "count": 6,
  "timestamp": 1641234567890
}
```

#### Get Categorized Tags

**GET** `/api/test/tags/categorized`

Returns tags organized by category.

**Response:**
```json
{
  "categories": {
    "Hardware": ["gpio", "adc", "pwm"],
    "Communication": ["modbus", "serial"],
    "Complexity": ["basic", "advanced"]
  },
  "timestamp": 1641234567890
}
```

#### Get Tag Presets

**GET** `/api/test/tags/presets`

Returns predefined tag combinations.

**Response:**
```json
{
  "presets": {
    "Basic Hardware": ["gpio", "basic"],
    "Communication Tests": ["modbus", "serial"],
    "Full Suite": ["gpio", "adc", "pwm", "modbus"]
  },
  "count": 3,
  "timestamp": 1641234567890
}
```

#### Save Tag Preset

**POST** `/api/test/tags/presets`

Saves a custom tag combination.

**Request Body:**
```json
{
  "name": "My Custom Tests",
  "tags": ["gpio", "adc"]
}
```

**Response:**
```json
{
  "success": true,
  "name": "My Custom Tests",
  "tags": ["gpio", "adc"],
  "message": "Tag preset saved successfully",
  "timestamp": 1641234567890
}
```

## WebSocket Events

The WebSocket connection provides real-time updates for hardware state changes and test execution progress.

### Connection

Connect to: `ws://localhost:3001/ws`

### Message Types

#### Pin State Updates

```json
{
  "type": "pin_state_update",
  "data": {
    "signal": "RESET",
    "state": "HIGH",
    "value": 1,
    "timestamp": 1641234567890
  }
}
```

#### Test Progress Updates

```json
{
  "type": "test_progress",
  "data": {
    "execution_id": "exec_1641234567890",
    "progress": 0.75,
    "current_scenario": "PWM Generation Test",
    "status": "running"
  }
}
```

#### Hardware Connection Status

```json
{
  "type": "connection_status",
  "data": {
    "connected": true,
    "port": "/dev/ttyUSB0"
  }
}
```

#### Arduino Command Logging

```json
{
  "type": "arduino_command",
  "data": {
    "command": "write_pin RESET HIGH",
    "response": "OK",
    "timestamp": 1641234567890,
    "direction": "sent"
  }
}
```

## Error Handling

### HTTP Status Codes

- `200` - Success
- `400` - Bad Request (invalid parameters)
- `404` - Not Found (pin/resource not found)
- `409` - Conflict (test already running)
- `500` - Internal Server Error
- `503` - Service Unavailable (hardware not connected)

### Error Response Format

```json
{
  "error": "Pin 'INVALID_PIN' not found",
  "message": "Available pins: RESET, MOSI, MISO, SCK",
  "timestamp": 1641234567890
}
```

## Rate Limiting

The API implements basic rate limiting to prevent hardware overload:
- Hardware commands: 10 requests per second
- Pin state reads: 20 requests per second
- Test execution: 1 concurrent execution

## Hardware Configuration

The API uses `config/hardware-config.yaml` as the SOLE SOURCE OF TRUTH for:
- Pin assignments between Arduino Test Wrapper and ATmega32A DUT
- Signal names and directions
- Hardware connection mapping
- Test point references

## Integration Notes

- The API integrates with existing HIL framework components
- WebSocket connections are automatically managed
- Hardware commands have a 100ms response time requirement
- All pin operations respect the pin matrix configuration
- Test automation uses existing BDD feature files from `test/acceptance/features/`
