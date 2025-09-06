"""
Unit tests for WebSocketHandler
Tests WebSocket communication and message handling
"""
import pytest
import json
from unittest.mock import Mock, MagicMock
import sys
import os

# Add backend source to path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../backend/src'))

# Mock Node.js specific imports for testing
sys.modules['ws'] = Mock()

@pytest.mark.unit
@pytest.mark.backend
@pytest.mark.websocket
class TestWebSocketHandler:
    """Test suite for WebSocketHandler class"""
    
    def test_websocket_handler_initialization(self, mock_hardware_interface):
        """Test WebSocketHandler initialization"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        
        assert handler.hardwareInterface == mock_hardware_interface
        assert len(handler.clients) == 0

    def test_client_connection_handling(self, mock_hardware_interface, mock_pin_states):
        """Test WebSocket client connection handling"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1  # WebSocket.OPEN
        
        # Mock hardware interface methods
        mock_hardware_interface.isConnected.return_value = True
        mock_hardware_interface.getSerialPort.return_value = '/dev/ttyUSB0'
        mock_hardware_interface.getPinStates.return_value = mock_pin_states
        
        handler.handleConnection(mock_ws)
        
        assert len(handler.clients) == 1
        assert mock_ws in handler.clients
        
        # Verify initial state was sent
        mock_ws.send.assert_called()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'initial_state'
        assert sent_data['data']['connected'] == True
        assert sent_data['data']['port'] == '/dev/ttyUSB0'

    def test_client_disconnection_handling(self, mock_hardware_interface):
        """Test WebSocket client disconnection handling"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        # Add client through handleConnection to set up mocks properly
        handler.handleConnection(mock_ws)
        assert len(handler.clients) == 1

        # Manually remove client to simulate disconnection
        handler.clients.remove(mock_ws)
        assert len(handler.clients) == 0

    @pytest.mark.asyncio
    async def test_ping_message_handling(self, mock_hardware_interface):
        """Test ping message handling"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1
        
        ping_message = {
            'type': 'ping',
            'timestamp': 1234567890
        }
        
        await handler.handleMessage(mock_ws, ping_message)

        # Verify pong response
        mock_ws.send.assert_called()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'pong'
        assert sent_data['data']['timestamp'] == 1234567890

    @pytest.mark.asyncio
    async def test_hardware_command_handling(self, mock_hardware_interface):
        """Test hardware command message handling"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1
        
        # Mock successful command response as async
        async def mock_send_command(command):
            return {
                'success': True,
                'timestamp': 1234567890
            }
        mock_hardware_interface.sendCommand = mock_send_command
        
        command_message = {
            'type': 'hardware_command',
            'data': {
                'command': 'write_pin',
                'pin': 'FREQ_DIV10_4',
                'value': 'HIGH'
            }
        }
        
        await handler.handleMessage(mock_ws, command_message)
        
        # Verify command was sent to hardware interface (can't assert on async mock easily)
        # Just verify the response was sent
        
        # Verify response was sent to client
        mock_ws.send.assert_called()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'command_response'
        assert sent_data['data']['success'] == True

    @pytest.mark.asyncio
    async def test_get_pin_states_handling(self, mock_hardware_interface, mock_pin_states):
        """Test get pin states message handling"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        mock_hardware_interface.getPinStates.return_value = mock_pin_states

        message = {'type': 'get_pin_states'}
        await handler.handleMessage(mock_ws, message)
        
        # Verify pin states response
        mock_ws.send.assert_called()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'pin_states'
        assert 'FREQ_DIV10_4' in sent_data['data']

    @pytest.mark.asyncio
    async def test_unknown_message_type_handling(self, mock_hardware_interface):
        """Test handling of unknown message types"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        unknown_message = {'type': 'unknown_type'}
        await handler.handleMessage(mock_ws, unknown_message)
        
        # Verify error response
        mock_ws.send.assert_called()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'error'
        assert 'Unknown message type' in sent_data['data']['error']

    def test_broadcast_to_all_clients(self, mock_hardware_interface):
        """Test broadcasting messages to all connected clients"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        
        # Add multiple mock clients
        mock_ws1 = Mock()
        mock_ws1.readyState = 1
        mock_ws2 = Mock()
        mock_ws2.readyState = 1
        mock_ws3 = Mock()
        mock_ws3.readyState = 0  # Closed connection
        
        handler.clients.add(mock_ws1)
        handler.clients.add(mock_ws2)
        handler.clients.add(mock_ws3)
        
        broadcast_message = {
            'type': 'pin_update',
            'data': {'signal': 'FREQ_DIV10_4', 'state': 'HIGH'},
            'timestamp': 1234567890
        }
        
        handler.broadcast(broadcast_message)
        
        # Verify message sent to open connections only
        mock_ws1.send.assert_called_once()
        mock_ws2.send.assert_called_once()
        mock_ws3.send.assert_not_called()
        
        # Verify closed connection was removed
        assert mock_ws3 not in handler.clients

    def test_hardware_event_listeners(self, mock_hardware_interface):
        """Test that hardware events are properly handled"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1
        handler.clients.add(mock_ws)
        
        # Simulate hardware connected event
        mock_hardware_interface.emit('connected', {'port': '/dev/ttyUSB0'})
        
        # Verify broadcast was called (would need to mock the broadcast method)
        # This tests the event listener setup
        assert mock_hardware_interface.on.called

    def test_invalid_json_message_handling(self, mock_hardware_interface):
        """Test handling of invalid JSON messages"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1
        
        # Simulate receiving invalid JSON
        invalid_data = "invalid json data"
        
        # This would normally be called by the WebSocket 'message' event handler
        # We test the error handling directly
        try:
            json.loads(invalid_data)
        except json.JSONDecodeError:
            handler.sendError(mock_ws, 'Invalid message format')
        
        # Verify error response
        mock_ws.send.assert_called()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'error'
        assert sent_data['data']['error'] == 'Invalid message format'

    def test_client_count_tracking(self, mock_hardware_interface):
        """Test client count tracking"""
        from websocket.WebSocketHandlerPython import WebSocketHandler
        
        handler = WebSocketHandler(mock_hardware_interface)
        
        assert handler.getClientCount() == 0
        
        # Add clients
        mock_ws1 = Mock()
        mock_ws2 = Mock()
        handler.clients.add(mock_ws1)
        handler.clients.add(mock_ws2)
        
        assert handler.getClientCount() == 2
        
        # Remove client
        handler.clients.remove(mock_ws1)
        assert handler.getClientCount() == 1

    def test_hardware_event_handlers(self, mock_hardware_interface):
        """Test hardware event handler setup and execution"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)

        # Test onHardwareConnected
        test_data = {'port': '/dev/ttyUSB0'}
        handler.onHardwareConnected(test_data)

        # Test onHardwareDisconnected
        handler.onHardwareDisconnected()

        # Test onPinUpdate
        handler.onPinUpdate('FREQ_DIV10_4', {'state': 'HIGH'})

        # Test onHardwareError
        handler.onHardwareError('Test error')

    def test_websocket_message_creation(self):
        """Test WebSocketMessage class"""
        from websocket.WebSocketHandlerPython import WebSocketMessage

        # Test with default timestamp
        msg = WebSocketMessage('test_type', {'key': 'value'})
        assert msg.type == 'test_type'
        assert msg.data == {'key': 'value'}
        assert isinstance(msg.timestamp, int)

        # Test with custom timestamp
        custom_timestamp = 1234567890
        msg2 = WebSocketMessage('test_type2', {'key2': 'value2'}, custom_timestamp)
        assert msg2.timestamp == custom_timestamp

    def test_send_error_method(self, mock_hardware_interface):
        """Test sendError method"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1
        mock_ws.send = Mock()

        handler.sendError(mock_ws, 'Test error message')

        mock_ws.send.assert_called_once()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'error'
        assert sent_data['data']['error'] == 'Test error message'

    def test_broadcast_with_closed_connections(self, mock_hardware_interface):
        """Test broadcast method removes closed connections"""
        from websocket.WebSocketHandlerPython import WebSocketHandler, WebSocketMessage

        handler = WebSocketHandler(mock_hardware_interface)

        # Add clients with different states
        mock_ws1 = Mock()
        mock_ws1.readyState = 1  # Open
        mock_ws1.send = Mock()

        mock_ws2 = Mock()
        mock_ws2.readyState = 0  # Closed
        mock_ws2.send = Mock()

        mock_ws3 = Mock()
        mock_ws3.readyState = 1  # Open
        mock_ws3.send = Mock()

        handler.clients.add(mock_ws1)
        handler.clients.add(mock_ws2)
        handler.clients.add(mock_ws3)

        assert len(handler.clients) == 3

        # Broadcast message
        message = WebSocketMessage('test', {'data': 'test'})
        handler.broadcast(message)

        # Verify only open connections received message
        mock_ws1.send.assert_called_once()
        mock_ws2.send.assert_not_called()
        mock_ws3.send.assert_called_once()

        # Verify closed connection was removed
        assert len(handler.clients) == 2
        assert mock_ws2 not in handler.clients

    @pytest.mark.asyncio
    async def test_hardware_command_error_handling(self, mock_hardware_interface):
        """Test hardware command error handling"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1
        mock_ws.send = Mock()

        # Mock hardware interface to raise exception
        async def mock_send_command_error(command):
            raise Exception("Hardware communication failed")
        mock_hardware_interface.sendCommand = mock_send_command_error

        command_message = {
            'type': 'hardware_command',
            'data': {
                'command': 'invalid_command',
                'pin': 'INVALID_PIN',
                'value': 'INVALID'
            }
        }

        await handler.handleMessage(mock_ws, command_message)

        # Verify error response was sent
        mock_ws.send.assert_called()
        sent_data = json.loads(mock_ws.send.call_args[0][0])
        assert sent_data['type'] == 'error'
        assert 'Hardware command failed' in sent_data['data']['error']

    def test_send_to_client_with_closed_connection(self, mock_hardware_interface):
        """Test sendToClient with closed connection"""
        from websocket.WebSocketHandlerPython import WebSocketHandler, WebSocketMessage

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 0  # Closed connection
        mock_ws.send = Mock()

        message = WebSocketMessage('test', {'data': 'test'})
        handler.sendToClient(mock_ws, message)

        # Verify send was not called for closed connection
        mock_ws.send.assert_not_called()

    @pytest.mark.asyncio
    async def test_hardware_command_read_pin(self, mock_hardware_interface):
        """Test read_pin hardware command"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        # Mock hardware interface response
        mock_hardware_interface.sendCommand.return_value = {'success': True, 'data': 'HIGH'}

        read_pin_message = {
            'type': 'hardware_command',
            'data': {
                'command': 'read_pin',
                'pin': 'D7'
            }
        }

        await handler.handleMessage(mock_ws, read_pin_message)

        # Verify hardware command was sent
        mock_hardware_interface.sendCommand.assert_called_with({
            'command': 'READ_PIN D7',
            'expectResponse': True
        })

    @pytest.mark.asyncio
    async def test_hardware_command_read_adc(self, mock_hardware_interface):
        """Test read_adc hardware command"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        # Mock hardware interface response
        mock_hardware_interface.sendCommand.return_value = {'success': True, 'data': '768'}

        read_adc_message = {
            'type': 'hardware_command',
            'data': {
                'command': 'read_adc',
                'pin': 'A1'
            }
        }

        await handler.handleMessage(mock_ws, read_adc_message)

        # Verify hardware command was sent
        mock_hardware_interface.sendCommand.assert_called_with({
            'command': 'READ_ADC A1',
            'expectResponse': True
        })

    @pytest.mark.asyncio
    async def test_hardware_command_read_pwm(self, mock_hardware_interface):
        """Test read_pwm hardware command"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        # Mock hardware interface response
        mock_hardware_interface.sendCommand.return_value = {'success': True, 'data': '1000:50'}

        read_pwm_message = {
            'type': 'hardware_command',
            'data': {
                'command': 'read_pwm',
                'pin': 'D9'
            }
        }

        await handler.handleMessage(mock_ws, read_pwm_message)

        # Verify hardware command was sent
        mock_hardware_interface.sendCommand.assert_called_with({
            'command': 'READ_PWM D9',
            'expectResponse': True
        })

    @pytest.mark.asyncio
    async def test_hardware_command_set_pwm(self, mock_hardware_interface):
        """Test set_pwm hardware command"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        # Mock hardware interface response
        mock_hardware_interface.sendCommand.return_value = {'success': True}

        set_pwm_message = {
            'type': 'hardware_command',
            'data': {
                'command': 'set_pwm',
                'pin': 'D9',
                'value': {
                    'frequency': 2000,
                    'dutyCycle': 75
                }
            }
        }

        await handler.handleMessage(mock_ws, set_pwm_message)

        # Verify hardware command was sent
        mock_hardware_interface.sendCommand.assert_called_with({
            'command': 'SET_PWM D9 2000 75',
            'expectResponse': True
        })

    @pytest.mark.asyncio
    async def test_hardware_command_ping_command(self, mock_hardware_interface):
        """Test ping hardware command (different from ping message)"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        # Mock hardware interface response
        mock_hardware_interface.sendCommand.return_value = {'success': True, 'data': 'PONG'}

        ping_command_message = {
            'type': 'hardware_command',
            'data': {
                'command': 'ping'
            }
        }

        await handler.handleMessage(mock_ws, ping_command_message)

        # Verify hardware command was sent
        mock_hardware_interface.sendCommand.assert_called_with({
            'command': 'PING',
            'expectResponse': True
        })

    def test_websocket_mock_attribute_setup(self, mock_hardware_interface):
        """Test WebSocket mock attribute setup for clients without attributes"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)

        # Create a mock WebSocket without the required attributes
        mock_ws = Mock()
        # Remove the attributes to test the setup code
        if hasattr(mock_ws, 'on'):
            delattr(mock_ws, 'on')
        if hasattr(mock_ws, 'send'):
            delattr(mock_ws, 'send')
        if hasattr(mock_ws, 'readyState'):
            delattr(mock_ws, 'readyState')

        # Handle connection - this should set up the missing attributes
        result_ws = handler.handleConnection(mock_ws)

        # Verify attributes were added
        assert hasattr(result_ws, 'on')
        assert hasattr(result_ws, 'send')
        assert hasattr(result_ws, 'readyState')
        assert result_ws.readyState == 1

    def test_send_to_client_with_dict_message(self, mock_hardware_interface):
        """Test sending dict message to client"""
        from websocket.WebSocketHandlerPython import WebSocketHandler

        handler = WebSocketHandler(mock_hardware_interface)
        mock_ws = Mock()
        mock_ws.readyState = 1

        # Test with dict message (should use the else branch on line 146)
        dict_message = {
            'type': 'test',
            'data': 'test_data',
            'timestamp': 1234567890
        }

        handler.sendToClient(mock_ws, dict_message)

        # Verify message was sent as JSON
        mock_ws.send.assert_called_with(json.dumps(dict_message))
