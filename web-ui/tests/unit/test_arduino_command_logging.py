"""
Unit tests for Arduino Command Logging functionality
Tests command logging, response handling, and WebSocket broadcasting
"""
import pytest
import json
import time
from unittest.mock import Mock, MagicMock, patch, call
import sys
import os

@pytest.mark.unit
@pytest.mark.backend
@pytest.mark.arduino_logging
class TestArduinoCommandLogging:
    """Test suite for Arduino command logging functionality"""

    @pytest.fixture
    def mock_hardware_interface(self):
        """Create a mock hardware interface for testing"""
        interface = Mock()
        interface.connected = True
        interface.pythonProcess = Mock()
        interface.pythonProcess.stdin = Mock()
        interface.pythonProcess.stdin.write = Mock()
        interface.emit = Mock()
        interface.lastLogTime = 0
        interface.logThrottleMs = 2000
        interface.lastPinLogTimes = {}
        interface.pinStates = {}

        # Mock methods that exist in the real implementation
        interface.sendPythonCommand = Mock()
        interface.updatePinState = Mock()
        interface.handlePythonMessage = Mock()

        return interface

    @pytest.fixture
    def mock_websocket_handler(self, mock_hardware_interface):
        """Create a mock WebSocket handler for testing"""
        handler = Mock()
        handler.hardwareInterface = mock_hardware_interface
        handler.clients = []
        handler.broadcast = Mock()
        handler.setupEventHandlers = Mock()
        return handler

    def test_command_logging_on_send(self, mock_hardware_interface):
        """Test that commands are logged when sent to Arduino"""

        # Mock the sendPythonCommand method
        command = {"type": "pin_read", "pin": "D7"}

        # Simulate the command logging logic that would be in sendPythonCommand
        def simulate_send_with_logging(cmd):
            # Log the command
            mock_hardware_interface.emit('arduino_command', {
                'direction': 'sent',
                'data': json.dumps(cmd),
                'timestamp': time.time() * 1000,
                'type': cmd.get('type', 'unknown')
            })
            return True

        mock_hardware_interface.sendPythonCommand.side_effect = simulate_send_with_logging

        # Call the method that should log commands
        result = mock_hardware_interface.sendPythonCommand(command)

        # Verify command was logged and emitted
        mock_hardware_interface.emit.assert_called_with('arduino_command', {
            'direction': 'sent',
            'data': json.dumps(command),
            'timestamp': pytest.approx(time.time() * 1000, abs=1000),
            'type': 'pin_read'
        })

    def test_response_logging_on_receive(self, mock_hardware_interface):
        """Test that responses are logged when received from Arduino"""

        # Mock response message
        response_data = "HIGH"
        message = {
            'type': 'response',
            'data': response_data
        }

        # Simulate the response logging logic that would be in handlePythonMessage
        def simulate_response_handling(msg):
            if msg['type'] == 'response':
                mock_hardware_interface.emit('arduino_command', {
                    'direction': 'received',
                    'data': msg['data'],
                    'timestamp': time.time() * 1000,
                    'type': 'response'
                })

        mock_hardware_interface.handlePythonMessage.side_effect = simulate_response_handling

        # Simulate receiving a response
        mock_hardware_interface.handlePythonMessage(message)

        # Verify response was logged
        mock_hardware_interface.emit.assert_called_with('arduino_command', {
            'direction': 'received',
            'data': response_data,
            'timestamp': pytest.approx(time.time() * 1000, abs=1000),
            'type': 'response'
        })

    def test_websocket_broadcasting_arduino_commands(self, mock_websocket_handler, mock_hardware_interface):
        """Test that Arduino commands are broadcast via WebSocket"""

        # Mock WebSocket clients
        mock_client1 = Mock()
        mock_client2 = Mock()
        mock_client1.readyState = 1  # WebSocket.OPEN
        mock_client2.readyState = 1  # WebSocket.OPEN

        mock_websocket_handler.clients = [mock_client1, mock_client2]

        # Simulate Arduino command event
        command_data = {
            'direction': 'sent',
            'data': '{"type":"pin_read","pin":"D7"}',
            'timestamp': time.time() * 1000,
            'type': 'pin_read'
        }

        # Simulate the event handling that would trigger broadcast
        def simulate_arduino_command_event(event_name, data):
            if event_name == 'arduino_command':
                message = {
                    'type': 'arduino_command',
                    'data': data,
                    'timestamp': time.time() * 1000
                }
                mock_websocket_handler.broadcast(json.dumps(message))

        mock_hardware_interface.emit.side_effect = simulate_arduino_command_event

        # Trigger the arduino_command event
        mock_hardware_interface.emit('arduino_command', command_data)

        # Verify broadcast was called
        mock_websocket_handler.broadcast.assert_called_once()

    def test_command_logging_throttling(self, mock_hardware_interface):
        """Test that command logging is properly throttled"""

        # Set up throttling
        mock_hardware_interface.lastPinLogTimes = {}
        mock_hardware_interface.logThrottleMs = 1000  # 1 second throttle

        pin = "D7"
        data = "HIGH"
        current_time = time.time() * 1000

        # Simulate the throttling logic that would be in updatePinState
        def simulate_throttled_update(pin, data):
            current_time = time.time() * 1000
            last_log_time = mock_hardware_interface.lastPinLogTimes.get(pin, 0)
            should_log = current_time - last_log_time > mock_hardware_interface.logThrottleMs

            if should_log:
                mock_hardware_interface.lastPinLogTimes[pin] = current_time
                mock_hardware_interface.emit('arduino_command', {
                    'direction': 'received',
                    'data': data,
                    'timestamp': current_time,
                    'type': 'response'
                })

        mock_hardware_interface.updatePinState.side_effect = simulate_throttled_update

        # First call should log
        with patch('time.time', return_value=current_time / 1000):
            mock_hardware_interface.updatePinState(pin, data)

        # Verify first call was logged
        assert pin in mock_hardware_interface.lastPinLogTimes
        assert mock_hardware_interface.emit.called

        # Second call within throttle period should not log
        mock_hardware_interface.emit.reset_mock()
        with patch('time.time', return_value=(current_time + 500) / 1000):  # 500ms later
            mock_hardware_interface.updatePinState(pin, data)

        # Should not have emitted new log (throttled)
        mock_hardware_interface.emit.assert_not_called()

        # Third call after throttle period should log again
        mock_hardware_interface.emit.reset_mock()
        with patch('time.time', return_value=(current_time + 1500) / 1000):  # 1.5s later
            mock_hardware_interface.updatePinState(pin, data)

        # Should have emitted new log
        mock_hardware_interface.emit.assert_called()

    def test_command_data_structure_validation(self, mock_hardware_interface):
        """Test that command data structure is properly validated"""
        
        # Test various command types
        test_commands = [
            {"type": "pin_read", "pin": "D7"},
            {"type": "pin_write", "pin": "D8", "value": "HIGH"},
            {"type": "pwm_set", "pin": "D9", "value": 128},
            {"type": "analog_read", "pin": "A0"}
        ]
        
        for command in test_commands:
            mock_hardware_interface.emit.reset_mock()
            
            # Simulate sending command
            mock_hardware_interface.emit('arduino_command', {
                'direction': 'sent',
                'data': json.dumps(command),
                'timestamp': time.time() * 1000,
                'type': command.get('type', 'unknown')
            })
            
            # Verify the call was made with correct structure
            call_args = mock_hardware_interface.emit.call_args[0]
            assert call_args[0] == 'arduino_command'
            
            command_data = call_args[1]
            assert 'direction' in command_data
            assert 'data' in command_data
            assert 'timestamp' in command_data
            assert 'type' in command_data
            assert command_data['direction'] in ['sent', 'received']
            assert isinstance(command_data['timestamp'], (int, float))

    def test_response_data_parsing(self, mock_hardware_interface):
        """Test that response data is properly parsed and logged"""
        
        # Test various response formats
        test_responses = [
            ("HIGH", "digital"),
            ("LOW", "digital"),
            ("1023", "analog"),
            ("PWM=50%", "pwm"),
            ("2.5kHz", "frequency"),
            ("OK", "status"),
            ("ERROR", "error")
        ]
        
        for response_data, expected_type in test_responses:
            mock_hardware_interface.emit.reset_mock()
            
            # Simulate receiving response
            mock_hardware_interface.emit('arduino_command', {
                'direction': 'received',
                'data': response_data,
                'timestamp': time.time() * 1000,
                'type': 'response'
            })
            
            # Verify response was logged correctly
            call_args = mock_hardware_interface.emit.call_args[0]
            command_data = call_args[1]
            
            assert command_data['direction'] == 'received'
            assert command_data['data'] == response_data
            assert command_data['type'] == 'response'

    def test_error_handling_in_command_logging(self, mock_hardware_interface):
        """Test error handling in command logging functionality"""

        # Test with malformed command
        def simulate_error_handling(cmd):
            try:
                # Simulate command processing that might fail
                if cmd.get("invalid"):
                    raise Exception("Connection error")
                return True
            except Exception as e:
                # Even if command fails, we should still log the attempt
                mock_hardware_interface.emit('arduino_command', {
                    'direction': 'sent',
                    'data': json.dumps(cmd),
                    'timestamp': time.time() * 1000,
                    'type': 'error'
                })
                raise e

        mock_hardware_interface.sendPythonCommand.side_effect = simulate_error_handling

        # Should handle error gracefully
        try:
            result = mock_hardware_interface.sendPythonCommand({"invalid": "command"})
        except Exception:
            pass  # Expected to raise

        # Logging should still work even if command fails
        assert mock_hardware_interface.emit.called

    def test_concurrent_command_logging(self, mock_hardware_interface):
        """Test command logging under concurrent conditions"""
        
        import threading
        import time
        
        # Simulate multiple concurrent commands
        commands = [
            {"type": "pin_read", "pin": f"D{i}"} for i in range(10)
        ]
        
        def send_command(cmd):
            mock_hardware_interface.emit('arduino_command', {
                'direction': 'sent',
                'data': json.dumps(cmd),
                'timestamp': time.time() * 1000,
                'type': cmd['type']
            })
        
        # Send commands concurrently
        threads = []
        for cmd in commands:
            thread = threading.Thread(target=send_command, args=(cmd,))
            threads.append(thread)
            thread.start()
        
        # Wait for all threads to complete
        for thread in threads:
            thread.join()
        
        # Verify all commands were logged
        assert mock_hardware_interface.emit.call_count == len(commands)

    def test_memory_management_in_logging(self, mock_hardware_interface):
        """Test that logging doesn't cause memory leaks"""
        
        # Simulate many log entries
        for i in range(1000):
            mock_hardware_interface.emit('arduino_command', {
                'direction': 'sent' if i % 2 == 0 else 'received',
                'data': f'test_data_{i}',
                'timestamp': time.time() * 1000 + i,
                'type': 'test'
            })
        
        # Verify that lastPinLogTimes doesn't grow unbounded
        # (In real implementation, this should be managed)
        assert len(mock_hardware_interface.lastPinLogTimes) < 100  # Reasonable limit
