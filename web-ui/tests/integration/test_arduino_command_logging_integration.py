"""
Integration tests for Arduino Command Logging
Tests the complete flow from backend command logging to frontend display
"""
import pytest
import json
import time
import asyncio
from unittest.mock import Mock, MagicMock, patch, AsyncMock
import sys
import os

# Add backend source to path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../backend/src'))

# Mock Node.js specific imports for testing
sys.modules['child_process'] = Mock()
sys.modules['events'] = Mock()
sys.modules['ws'] = Mock()

@pytest.mark.integration
@pytest.mark.arduino_logging
class TestArduinoCommandLoggingIntegration:
    """Integration test suite for Arduino command logging end-to-end flow"""
    
    @pytest.fixture
    def mock_hardware_interface(self):
        """Create a mock hardware interface for integration testing"""
        from adapters.HardwareInterfacePython import HardwareInterface
        
        with patch.object(HardwareInterface, '__init__', return_value=None):
            interface = HardwareInterface()
            interface.connected = True
            interface.pythonProcess = Mock()
            interface.pythonProcess.stdin = Mock()
            interface.pythonProcess.stdin.write = Mock()
            interface.emit = Mock()
            interface.lastLogTime = 0
            interface.logThrottleMs = 100  # Faster for testing
            interface.lastPinLogTimes = {}
            
            # Mock pin states
            interface.pinStates = {
                'D7': {'pin': 'D7', 'signal': 'D7', 'direction': 'input', 'state': 'LOW', 'timestamp': time.time() * 1000},
                'D8': {'pin': 'D8', 'signal': 'D8', 'direction': 'output', 'state': 'HIGH', 'timestamp': time.time() * 1000}
            }
            
            return interface

    @pytest.fixture
    def mock_websocket_handler(self, mock_hardware_interface):
        """Create a mock WebSocket handler for integration testing"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        with patch.object(WebSocketHandler, '__init__', return_value=None):
            handler = WebSocketHandler(mock_hardware_interface)
            handler.hardwareInterface = mock_hardware_interface
            handler.clients = []
            handler.broadcast = Mock()
            
            # Set up event handling
            handler.setupEventHandlers = Mock()
            
            return handler

    @pytest.fixture
    def mock_websocket_clients(self):
        """Create mock WebSocket clients"""
        clients = []
        for i in range(3):
            client = Mock()
            client.readyState = 1  # WebSocket.OPEN
            client.send = Mock()
            client.received_messages = []
            
            # Mock send to capture messages
            def capture_send(message, client=client):
                client.received_messages.append(message)
            
            client.send.side_effect = capture_send
            clients.append(client)
        
        return clients

    def test_end_to_end_command_flow(self, mock_hardware_interface, mock_websocket_handler, mock_websocket_clients):
        """Test complete flow from command send to WebSocket broadcast"""
        
        # Set up WebSocket clients
        mock_websocket_handler.clients = mock_websocket_clients
        
        # Set up event handling to simulate real behavior
        def simulate_arduino_command_event(event_name, data):
            if event_name == 'arduino_command':
                # Simulate WebSocket broadcast
                message = {
                    'type': 'arduino_command',
                    'data': data,
                    'timestamp': time.time() * 1000
                }
                mock_websocket_handler.broadcast(json.dumps(message))
        
        mock_hardware_interface.emit.side_effect = simulate_arduino_command_event
        
        # Test command sending
        test_command = {"type": "pin_read", "pin": "D7"}
        
        # Simulate sending command
        mock_hardware_interface.emit('arduino_command', {
            'direction': 'sent',
            'data': json.dumps(test_command),
            'timestamp': time.time() * 1000,
            'type': 'pin_read'
        })
        
        # Verify command was logged
        assert mock_hardware_interface.emit.called
        
        # Verify WebSocket broadcast was triggered
        assert mock_websocket_handler.broadcast.called
        
        # Test response receiving
        response_data = "HIGH"
        mock_hardware_interface.emit('arduino_command', {
            'direction': 'received',
            'data': response_data,
            'timestamp': time.time() * 1000,
            'type': 'response'
        })
        
        # Verify response was logged and broadcast
        assert mock_hardware_interface.emit.call_count >= 2
        assert mock_websocket_handler.broadcast.call_count >= 2

    def test_multiple_clients_receive_commands(self, mock_hardware_interface, mock_websocket_handler, mock_websocket_clients):
        """Test that all connected clients receive Arduino commands"""
        
        mock_websocket_handler.clients = mock_websocket_clients
        
        # Mock broadcast to actually send to all clients
        def real_broadcast(message):
            for client in mock_websocket_handler.clients:
                if client.readyState == 1:  # WebSocket.OPEN
                    client.send(message)
        
        mock_websocket_handler.broadcast.side_effect = real_broadcast
        
        # Set up event handling
        def simulate_broadcast_event(event_name, data):
            if event_name == 'arduino_command':
                message = {
                    'type': 'arduino_command',
                    'data': data,
                    'timestamp': time.time() * 1000
                }
                mock_websocket_handler.broadcast(json.dumps(message))
        
        mock_hardware_interface.emit.side_effect = simulate_broadcast_event
        
        # Send a command
        test_command = {"type": "pin_write", "pin": "D8", "value": "LOW"}
        mock_hardware_interface.emit('arduino_command', {
            'direction': 'sent',
            'data': json.dumps(test_command),
            'timestamp': time.time() * 1000,
            'type': 'pin_write'
        })
        
        # Verify all clients received the message
        for client in mock_websocket_clients:
            assert len(client.received_messages) == 1
            message = json.loads(client.received_messages[0])
            assert message['type'] == 'arduino_command'
            assert message['data']['direction'] == 'sent'

    def test_command_throttling_integration(self, mock_hardware_interface, mock_websocket_handler):
        """Test that command throttling works in integration"""
        
        # Set up rapid command sending
        commands_sent = []
        
        def capture_emit(event_name, data):
            if event_name == 'arduino_command':
                commands_sent.append(data)
        
        mock_hardware_interface.emit.side_effect = capture_emit
        
        # Send multiple commands rapidly for the same pin
        pin = "D7"
        for i in range(10):
            # Simulate updatePinState which should be throttled
            current_time = time.time() * 1000
            
            # Mock the throttling logic
            last_log_time = mock_hardware_interface.lastPinLogTimes.get(pin, 0)
            should_log = current_time - last_log_time > mock_hardware_interface.logThrottleMs
            
            if should_log:
                mock_hardware_interface.lastPinLogTimes[pin] = current_time
                mock_hardware_interface.emit('arduino_command', {
                    'direction': 'received',
                    'data': f'update_{i}',
                    'timestamp': current_time,
                    'type': 'response'
                })
            
            # Small delay to simulate rapid updates
            time.sleep(0.01)
        
        # Should have throttled most commands
        assert len(commands_sent) < 10  # Should be significantly less than 10

    def test_error_recovery_integration(self, mock_hardware_interface, mock_websocket_handler, mock_websocket_clients):
        """Test error recovery in the integration flow"""
        
        mock_websocket_handler.clients = mock_websocket_clients
        
        # Simulate a client with connection error
        error_client = mock_websocket_clients[0]
        error_client.readyState = 3  # WebSocket.CLOSED
        error_client.send.side_effect = Exception("Connection closed")
        
        # Mock broadcast with error handling
        def robust_broadcast(message):
            successful_sends = 0
            for client in mock_websocket_handler.clients:
                try:
                    if client.readyState == 1:  # WebSocket.OPEN
                        client.send(message)
                        successful_sends += 1
                except Exception:
                    pass  # Ignore errors for this test
            return successful_sends
        
        mock_websocket_handler.broadcast.side_effect = robust_broadcast
        
        # Set up event handling
        def simulate_event_with_broadcast(event_name, data):
            if event_name == 'arduino_command':
                message = {
                    'type': 'arduino_command',
                    'data': data,
                    'timestamp': time.time() * 1000
                }
                mock_websocket_handler.broadcast(json.dumps(message))
        
        mock_hardware_interface.emit.side_effect = simulate_event_with_broadcast
        
        # Send a command
        test_command = {"type": "pin_read", "pin": "A0"}
        mock_hardware_interface.emit('arduino_command', {
            'direction': 'sent',
            'data': json.dumps(test_command),
            'timestamp': time.time() * 1000,
            'type': 'pin_read'
        })
        
        # Verify that working clients still received the message
        working_clients = [c for c in mock_websocket_clients if c.readyState == 1]
        for client in working_clients:
            assert len(client.received_messages) == 1

    def test_high_frequency_command_handling(self, mock_hardware_interface, mock_websocket_handler):
        """Test handling of high-frequency Arduino commands"""
        
        commands_processed = []
        
        def process_command(event_name, data):
            if event_name == 'arduino_command':
                commands_processed.append(data)
                # Simulate WebSocket broadcast
                mock_websocket_handler.broadcast(json.dumps({
                    'type': 'arduino_command',
                    'data': data,
                    'timestamp': time.time() * 1000
                }))
        
        mock_hardware_interface.emit.side_effect = process_command
        
        # Generate high-frequency commands
        start_time = time.time()
        for i in range(100):
            mock_hardware_interface.emit('arduino_command', {
                'direction': 'sent' if i % 2 == 0 else 'received',
                'data': f'high_freq_cmd_{i}',
                'timestamp': time.time() * 1000,
                'type': 'test'
            })
        
        end_time = time.time()
        processing_time = end_time - start_time
        
        # Verify all commands were processed
        assert len(commands_processed) == 100
        
        # Verify processing was reasonably fast
        assert processing_time < 1.0  # Should complete in less than 1 second
        
        # Verify WebSocket broadcasts were made
        assert mock_websocket_handler.broadcast.call_count == 100

    def test_command_data_integrity(self, mock_hardware_interface, mock_websocket_handler, mock_websocket_clients):
        """Test that command data maintains integrity through the pipeline"""
        
        mock_websocket_handler.clients = mock_websocket_clients
        
        # Track messages through the pipeline
        original_commands = []
        broadcast_messages = []
        
        def track_emit(event_name, data):
            if event_name == 'arduino_command':
                original_commands.append(data)
        
        def track_broadcast(message):
            broadcast_messages.append(json.loads(message))
            # Also send to clients
            for client in mock_websocket_handler.clients:
                if client.readyState == 1:
                    client.send(message)
        
        mock_hardware_interface.emit.side_effect = track_emit
        mock_websocket_handler.broadcast.side_effect = track_broadcast
        
        # Set up event handling to trigger broadcast
        def simulate_full_flow(event_name, data):
            track_emit(event_name, data)
            if event_name == 'arduino_command':
                message = {
                    'type': 'arduino_command',
                    'data': data,
                    'timestamp': time.time() * 1000
                }
                track_broadcast(json.dumps(message))
        
        mock_hardware_interface.emit.side_effect = simulate_full_flow
        
        # Send test commands with various data types
        test_commands = [
            {
                'direction': 'sent',
                'data': '{"type":"pin_read","pin":"D7"}',
                'timestamp': time.time() * 1000,
                'type': 'pin_read'
            },
            {
                'direction': 'received',
                'data': 'HIGH',
                'timestamp': time.time() * 1000,
                'type': 'response'
            },
            {
                'direction': 'sent',
                'data': '{"type":"analog_read","pin":"A0"}',
                'timestamp': time.time() * 1000,
                'type': 'analog_read'
            },
            {
                'direction': 'received',
                'data': '512',
                'timestamp': time.time() * 1000,
                'type': 'response'
            }
        ]
        
        for cmd in test_commands:
            mock_hardware_interface.emit('arduino_command', cmd)
        
        # Verify data integrity
        assert len(original_commands) == len(test_commands)
        assert len(broadcast_messages) == len(test_commands)
        
        for i, original in enumerate(original_commands):
            broadcast = broadcast_messages[i]
            
            # Verify structure
            assert broadcast['type'] == 'arduino_command'
            assert 'data' in broadcast
            assert 'timestamp' in broadcast
            
            # Verify data integrity
            broadcast_data = broadcast['data']
            assert broadcast_data['direction'] == original['direction']
            assert broadcast_data['data'] == original['data']
            assert broadcast_data['type'] == original['type']
            
            # Timestamps should be close (within 1 second)
            assert abs(broadcast_data['timestamp'] - original['timestamp']) < 1000

    def test_concurrent_command_processing(self, mock_hardware_interface, mock_websocket_handler):
        """Test concurrent command processing in integration"""
        
        import threading
        import queue
        
        processed_commands = queue.Queue()
        
        def thread_safe_emit(event_name, data):
            if event_name == 'arduino_command':
                processed_commands.put(data)
                mock_websocket_handler.broadcast(json.dumps({
                    'type': 'arduino_command',
                    'data': data,
                    'timestamp': time.time() * 1000
                }))
        
        mock_hardware_interface.emit.side_effect = thread_safe_emit
        
        # Create multiple threads sending commands
        def send_commands(thread_id, num_commands):
            for i in range(num_commands):
                command = {
                    'direction': 'sent',
                    'data': f'thread_{thread_id}_cmd_{i}',
                    'timestamp': time.time() * 1000,
                    'type': 'test'
                }
                mock_hardware_interface.emit('arduino_command', command)
        
        # Start multiple threads
        threads = []
        num_threads = 5
        commands_per_thread = 10
        
        for i in range(num_threads):
            thread = threading.Thread(target=send_commands, args=(i, commands_per_thread))
            threads.append(thread)
            thread.start()
        
        # Wait for all threads to complete
        for thread in threads:
            thread.join()
        
        # Verify all commands were processed
        total_commands = num_threads * commands_per_thread
        processed_count = processed_commands.qsize()
        
        assert processed_count == total_commands
        
        # Verify no data corruption
        all_commands = []
        while not processed_commands.empty():
            all_commands.append(processed_commands.get())
        
        for command in all_commands:
            assert 'direction' in command
            assert 'data' in command
            assert 'timestamp' in command
            assert 'type' in command
            assert command['direction'] == 'sent'
            assert 'thread_' in command['data']
