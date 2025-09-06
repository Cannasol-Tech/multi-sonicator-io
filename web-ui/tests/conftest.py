"""
Pytest configuration and fixtures for Multi-Sonicator-IO Web UI tests
"""
import pytest
import asyncio
import json
import time
from unittest.mock import Mock, MagicMock, patch
from typing import Dict, Any, Generator

# Test fixtures for hardware state
@pytest.fixture
def mock_pin_states() -> Dict[str, Any]:
    """Mock pin states based on pin-matrix.md (SOLE SOURCE OF TRUTH)"""
    return {
        'FREQ_DIV10_4': {
            'pin': 'D7',
            'signal': 'FREQ_DIV10_4',
            'direction': 'IN',
            'state': 'LOW',
            'timestamp': int(time.time() * 1000),
            'description': 'Frequency ÷10 input to ATmega32A PB0'
        },
        'FREQ_LOCK_4': {
            'pin': 'D8',
            'signal': 'FREQ_LOCK_4', 
            'direction': 'IN',
            'state': 'LOW',
            'timestamp': int(time.time() * 1000),
            'description': 'Frequency lock input to ATmega32A PB4'
        },
        'OVERLOAD_4': {
            'pin': 'A2',
            'signal': 'OVERLOAD_4',
            'direction': 'IN', 
            'state': 'LOW',
            'timestamp': int(time.time() * 1000),
            'description': 'Overload input to ATmega32A PD3'
        },
        'START_4': {
            'pin': 'A3',
            'signal': 'START_4',
            'direction': 'OUT',
            'state': 'LOW',
            'timestamp': int(time.time() * 1000),
            'description': 'Start output from ATmega32A PC0'
        },
        'RESET_4': {
            'pin': 'A4',
            'signal': 'RESET_4',
            'direction': 'OUT',
            'state': 'LOW',
            'timestamp': int(time.time() * 1000),
            'description': 'Reset output from ATmega32A PC1'
        },
        'POWER_SENSE_4': {
            'pin': 'A1',
            'signal': 'POWER_SENSE_4',
            'direction': 'ANALOG',
            'state': 512,  # Mid-range ADC value
            'timestamp': int(time.time() * 1000),
            'description': 'Power sense analog from ATmega32A PA7'
        },
        'AMPLITUDE_ALL': {
            'pin': 'D9',
            'signal': 'AMPLITUDE_ALL',
            'direction': 'IN',
            'state': 'LOW',
            'timestamp': int(time.time() * 1000),
            'description': 'Amplitude PWM input to ATmega32A PD7'
        },
        'UART_RXD': {
            'pin': 'D10',
            'signal': 'UART_RXD',
            'direction': 'IN',
            'state': 'HIGH',
            'timestamp': int(time.time() * 1000),
            'description': 'UART RX to ATmega32A PD0'
        },
        'UART_TXD': {
            'pin': 'D11',
            'signal': 'UART_TXD',
            'direction': 'OUT',
            'state': 'HIGH',
            'timestamp': int(time.time() * 1000),
            'description': 'UART TX from ATmega32A PD1'
        },
        'STATUS_LED': {
            'pin': 'D12',
            'signal': 'STATUS_LED',
            'direction': 'OUT',
            'state': 'LOW',
            'timestamp': int(time.time() * 1000),
            'description': 'Status LED from ATmega32A PD2'
        }
    }

@pytest.fixture
def mock_hardware_interface():
    """Mock hardware interface for testing"""
    mock = Mock()
    mock.isConnected.return_value = True
    mock.getSerialPort.return_value = '/dev/ttyUSB0'
    mock.getPinStates.return_value = {}
    mock.sendCommand.return_value = {
        'success': True,
        'timestamp': int(time.time() * 1000)
    }
    return mock

@pytest.fixture
def mock_websocket_message():
    """Factory for creating mock WebSocket messages"""
    def _create_message(msg_type: str, data: Any = None):
        return {
            'type': msg_type,
            'data': data or {},
            'timestamp': int(time.time() * 1000)
        }
    return _create_message

@pytest.fixture
def mock_connection_status():
    """Mock connection status"""
    return {
        'connected': True,
        'port': '/dev/ttyUSB0',
        'lastSeen': int(time.time() * 1000),
        'error': None
    }

@pytest.fixture
def mock_hardware_state(mock_pin_states, mock_connection_status):
    """Complete mock hardware state"""
    return {
        'connection': mock_connection_status,
        'pins': mock_pin_states,
        'lastUpdate': int(time.time() * 1000)
    }

@pytest.fixture
def mock_python_process():
    """Mock Python subprocess for HIL integration"""
    mock = Mock()
    mock.stdout = Mock()
    mock.stderr = Mock()
    mock.stdin = Mock()
    mock.poll.return_value = None  # Process running
    mock.returncode = None
    return mock

@pytest.fixture
def event_loop():
    """Create an instance of the default event loop for the test session."""
    loop = asyncio.new_event_loop()
    yield loop
    loop.close()

# Test utilities
class MockWebSocketServer:
    """Mock WebSocket server for testing"""
    def __init__(self):
        self.clients = []
        self.messages = []
    
    def add_client(self, client):
        self.clients.append(client)
    
    def broadcast(self, message):
        self.messages.append(message)
        for client in self.clients:
            if hasattr(client, 'send'):
                client.send(json.dumps(message))

@pytest.fixture
def mock_websocket_server():
    """Mock WebSocket server"""
    return MockWebSocketServer()

# Hardware command fixtures
@pytest.fixture
def sample_hardware_commands():
    """Sample hardware commands for testing"""
    return [
        {'command': 'PING', 'expectResponse': True},
        {'command': 'WRITE_PIN D7 HIGH', 'expectResponse': True},
        {'command': 'READ_PIN D7', 'expectResponse': True},
        {'command': 'READ_ADC A1', 'expectResponse': True},
        {'command': 'SET_PWM D9 1000 50', 'expectResponse': True},
        {'command': 'STATUS', 'expectResponse': True}
    ]

# Pin history fixtures
@pytest.fixture
def sample_pin_history():
    """Sample pin history entries"""
    base_time = int(time.time() * 1000)
    return [
        {
            'signal': 'FREQ_DIV10_4',
            'pin': 'D7',
            'previousState': 'LOW',
            'newState': 'HIGH',
            'timestamp': base_time,
            'direction': 'IN'
        },
        {
            'signal': 'START_4',
            'pin': 'A3',
            'previousState': 'LOW',
            'newState': 'HIGH',
            'timestamp': base_time + 1000,
            'direction': 'OUT'
        },
        {
            'signal': 'POWER_SENSE_4',
            'pin': 'A1',
            'previousState': 512,
            'newState': 768,
            'timestamp': base_time + 2000,
            'direction': 'ANALOG'
        }
    ]

# Arduino Command Logging Test Fixtures
@pytest.fixture
def sample_arduino_commands() -> list:
    """Sample Arduino commands for testing"""
    base_time = int(time.time() * 1000)
    return [
        {
            'direction': 'sent',
            'data': '{"type":"pin_read","pin":"D7"}',
            'timestamp': base_time,
            'type': 'pin_read'
        },
        {
            'direction': 'received',
            'data': 'HIGH',
            'timestamp': base_time + 100,
            'type': 'response'
        },
        {
            'direction': 'sent',
            'data': '{"type":"pin_write","pin":"D8","value":"LOW"}',
            'timestamp': base_time + 200,
            'type': 'pin_write'
        },
        {
            'direction': 'received',
            'data': 'OK',
            'timestamp': base_time + 300,
            'type': 'response'
        },
        {
            'direction': 'sent',
            'data': '{"type":"analog_read","pin":"A0"}',
            'timestamp': base_time + 400,
            'type': 'analog_read'
        },
        {
            'direction': 'received',
            'data': '512',
            'timestamp': base_time + 500,
            'type': 'response'
        }
    ]

@pytest.fixture
def mock_arduino_hardware_interface():
    """Mock hardware interface with Arduino command logging capabilities"""
    interface = Mock()
    interface.connected = True
    interface.pythonProcess = Mock()
    interface.pythonProcess.stdin = Mock()
    interface.emit = Mock()
    interface.lastLogTime = 0
    interface.logThrottleMs = 1000
    interface.lastPinLogTimes = {}
    interface.pinStates = {}

    # Mock methods
    interface.sendPythonCommand = Mock()
    interface.updatePinState = Mock()
    interface.handlePythonMessage = Mock()

    return interface

@pytest.fixture
def mock_arduino_websocket_handler(mock_arduino_hardware_interface):
    """Mock WebSocket handler for Arduino command logging"""
    handler = Mock()
    handler.hardwareInterface = mock_arduino_hardware_interface
    handler.clients = []
    handler.broadcast = Mock()
    handler.setupEventHandlers = Mock()

    return handler

@pytest.fixture
def mock_websocket_clients():
    """Mock WebSocket clients for testing"""
    clients = []
    for i in range(3):
        client = Mock()
        client.readyState = 1  # WebSocket.OPEN
        client.send = Mock()
        client.received_messages = []

        def capture_send(message, client=client):
            client.received_messages.append(message)

        client.send.side_effect = capture_send
        clients.append(client)

    return clients

# Test markers
pytest.mark.unit = pytest.mark.unit
pytest.mark.integration = pytest.mark.integration
pytest.mark.frontend = pytest.mark.frontend
pytest.mark.backend = pytest.mark.backend
pytest.mark.hardware = pytest.mark.hardware
pytest.mark.websocket = pytest.mark.websocket
pytest.mark.arduino_logging = pytest.mark.arduino_logging
pytest.mark.slow = pytest.mark.slow
