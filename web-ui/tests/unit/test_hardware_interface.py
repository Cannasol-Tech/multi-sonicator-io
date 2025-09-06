"""
Unit tests for HardwareInterface adapter
Tests the Python HIL integration and hardware communication
"""
import pytest
import asyncio
import json
from unittest.mock import Mock, patch, MagicMock
import sys
import os

# Add backend source to path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../backend/src'))

# Mock the Node.js specific imports for testing
sys.modules['child_process'] = Mock()
sys.modules['events'] = Mock()

@pytest.mark.unit
@pytest.mark.backend
@pytest.mark.hardware
class TestHardwareInterface:
    """Test suite for HardwareInterface class"""
    
    def test_pin_state_initialization(self, mock_pin_states):
        """Test that pin states are initialized correctly from pin-matrix.md"""
        # Import here to avoid import issues during collection
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        
        # Verify all expected pins are initialized
        expected_signals = [
            'FREQ_DIV10_4', 'FREQ_LOCK_4', 'OVERLOAD_4', 'START_4', 'RESET_4',
            'POWER_SENSE_4', 'AMPLITUDE_ALL', 'UART_RXD', 'UART_TXD', 'STATUS_LED'
        ]
        
        pin_states = interface.getPinStates()
        assert len(pin_states) == len(expected_signals)
        
        for signal in expected_signals:
            assert signal in pin_states
            pin_state = pin_states[signal]
            assert 'pin' in pin_state
            assert 'signal' in pin_state
            assert 'direction' in pin_state
            assert 'state' in pin_state
            assert 'timestamp' in pin_state

    def test_pin_directions_match_matrix(self):
        """Test that pin directions match the pin matrix specification"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        pin_states = interface.getPinStates()
        
        # Verify specific pin directions based on pin-matrix.md
        assert pin_states['FREQ_DIV10_4']['direction'] == 'IN'
        assert pin_states['FREQ_LOCK_4']['direction'] == 'IN'
        assert pin_states['OVERLOAD_4']['direction'] == 'IN'
        assert pin_states['START_4']['direction'] == 'OUT'
        assert pin_states['RESET_4']['direction'] == 'OUT'
        assert pin_states['POWER_SENSE_4']['direction'] == 'ANALOG'
        assert pin_states['AMPLITUDE_ALL']['direction'] == 'IN'
        assert pin_states['UART_RXD']['direction'] == 'IN'
        assert pin_states['UART_TXD']['direction'] == 'OUT'
        assert pin_states['STATUS_LED']['direction'] == 'OUT'

    def test_arduino_pin_mapping(self):
        """Test that Arduino pin assignments match pin-matrix.md"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        pin_states = interface.getPinStates()
        
        # Verify Arduino pin assignments
        assert pin_states['FREQ_DIV10_4']['pin'] == 'D7'
        assert pin_states['FREQ_LOCK_4']['pin'] == 'D8'
        assert pin_states['OVERLOAD_4']['pin'] == 'A2'
        assert pin_states['START_4']['pin'] == 'A3'
        assert pin_states['RESET_4']['pin'] == 'A4'
        assert pin_states['POWER_SENSE_4']['pin'] == 'A1'
        assert pin_states['AMPLITUDE_ALL']['pin'] == 'D9'
        assert pin_states['UART_RXD']['pin'] == 'D10'
        assert pin_states['UART_TXD']['pin'] == 'D11'
        assert pin_states['STATUS_LED']['pin'] == 'D12'

    def test_hardware_initialization_success(self, mock_python_process):
        """Test successful hardware initialization"""
        from adapters.HardwareInterfacePython import HardwareInterface

        interface = HardwareInterface()
        interface.pythonProcess = mock_python_process

        # Mock successful connection message
        connection_message = {
            "type": "connection",
            "status": "connected",
            "port": "/dev/ttyUSB0"
        }

        # Simulate receiving connection message
        interface.handlePythonMessage(connection_message)

        assert interface.isConnected() == True
        assert interface.getSerialPort() == "/dev/ttyUSB0"

    def test_pin_state_update_digital(self):
        """Test updating digital pin states"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        
        # Test HIGH state update
        interface.updatePinState('D7', 'HIGH')
        pin_states = interface.getPinStates()
        assert pin_states['FREQ_DIV10_4']['state'] == 'HIGH'
        
        # Test LOW state update
        interface.updatePinState('D7', 'LOW')
        pin_states = interface.getPinStates()
        assert pin_states['FREQ_DIV10_4']['state'] == 'LOW'

    def test_pin_state_update_analog(self):
        """Test updating analog pin states"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        
        # Test analog value update
        interface.updatePinState('A1', 'ADC=768')
        pin_states = interface.getPinStates()
        assert pin_states['POWER_SENSE_4']['state'] == 768

    @pytest.mark.asyncio
    async def test_send_command_success(self, mock_python_process):
        """Test successful command sending"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        interface.connected = True
        interface.pythonProcess = mock_python_process
        
        command = {'command': 'PING', 'expectResponse': True}
        response = await interface.sendCommand(command)
        
        assert response['success'] == True
        assert 'timestamp' in response

    @pytest.mark.asyncio
    async def test_send_command_not_connected(self):
        """Test command sending when not connected"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        interface.connected = False
        
        command = {'command': 'PING', 'expectResponse': True}
        response = await interface.sendCommand(command)
        
        assert response['success'] == False
        assert response['error'] == 'Hardware not connected'

    def test_disconnect(self, mock_python_process):
        """Test hardware disconnection"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        interface.pythonProcess = mock_python_process
        interface.connected = True
        
        interface.disconnect()
        
        assert interface.connected == False
        mock_python_process.kill.assert_called_once()

    def test_python_message_handling_error(self):
        """Test handling of error messages from Python process"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        error_emitted = False
        
        def on_error(error):
            nonlocal error_emitted
            error_emitted = True
            assert error == "Test error message"
        
        interface.on('error', on_error)
        
        error_message = {
            "type": "error",
            "error": "Test error message"
        }
        
        interface.handlePythonMessage(error_message)
        assert error_emitted

    def test_get_connection_status(self):
        """Test connection status reporting"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        interface = HardwareInterface()
        
        # Test disconnected state
        assert interface.isConnected() == False
        assert interface.getSerialPort() is None
        
        # Test connected state
        interface.connected = True
        interface.serialPort = '/dev/ttyUSB0'
        
        assert interface.isConnected() == True
        assert interface.getSerialPort() == '/dev/ttyUSB0'

    def test_adc_parsing_error_handling(self):
        """Test ADC parsing with invalid data"""
        from adapters.HardwareInterfacePython import HardwareInterface

        interface = HardwareInterface()

        # Test invalid ADC format - should not crash
        interface.updatePinState('A1', 'ADC=invalid')
        pin_states = interface.getPinStates()
        # State should remain unchanged (default 0)
        assert pin_states['POWER_SENSE_4']['state'] == 0

        # Test malformed ADC data - should not crash
        interface.updatePinState('A1', 'ADC')
        pin_states = interface.getPinStates()
        # State should remain unchanged
        assert pin_states['POWER_SENSE_4']['state'] == 0

    def test_python_message_handling_disconnection(self):
        """Test handling of disconnection messages from Python process"""
        from adapters.HardwareInterfacePython import HardwareInterface

        interface = HardwareInterface()
        interface.connected = True  # Start connected

        # Test disconnection message
        message = {
            'type': 'connection',
            'status': 'disconnected'
        }

        interface.handlePythonMessage(message)

        # Should be disconnected now
        assert interface.connected == False

    def test_python_message_handling_pin_state(self):
        """Test handling of pin state messages from Python process"""
        from adapters.HardwareInterfacePython import HardwareInterface

        interface = HardwareInterface()

        # Test pin state message
        message = {
            'type': 'pin_state',
            'pin': 'D7',
            'data': 'HIGH'
        }

        interface.handlePythonMessage(message)

        # Pin state should be updated
        pin_states = interface.getPinStates()
        assert pin_states['FREQ_DIV10_4']['state'] == 'HIGH'
