"""
Python-compatible version of WebSocketHandler for testing
This mirrors the TypeScript WebSocketHandler functionality for pytest
"""

import json
import time
from typing import Set, Dict, Any, Optional
from unittest.mock import Mock

class WebSocketMessage:
    def __init__(self, msg_type: str, data: Any = None, timestamp: Optional[int] = None):
        self.type = msg_type
        self.data = data or {}
        self.timestamp = timestamp or int(time.time() * 1000)

class WebSocketHandler:
    """Python version of WebSocketHandler for testing"""
    
    def __init__(self, hardwareInterface):
        self.clients = set()
        self.hardwareInterface = hardwareInterface
        self.setupHardwareListeners()
    
    def setupHardwareListeners(self):
        """Setup hardware event listeners"""
        # Mock event listener setup
        self.hardwareInterface.on('connected', self.onHardwareConnected)
        self.hardwareInterface.on('disconnected', self.onHardwareDisconnected)
        self.hardwareInterface.on('pin_update', self.onPinUpdate)
        self.hardwareInterface.on('error', self.onHardwareError)
    
    def onHardwareConnected(self, data):
        """Handle hardware connected event"""
        message = WebSocketMessage('connection_status', {'connected': True, **data})
        self.broadcast(message)
    
    def onHardwareDisconnected(self):
        """Handle hardware disconnected event"""
        message = WebSocketMessage('connection_status', {'connected': False})
        self.broadcast(message)
    
    def onPinUpdate(self, signal, pinState):
        """Handle pin update event"""
        message = WebSocketMessage('pin_update', {'signal': signal, 'pinState': pinState})
        self.broadcast(message)
    
    def onHardwareError(self, error):
        """Handle hardware error event"""
        message = WebSocketMessage('error', {'error': error})
        self.broadcast(message)
    
    def handleConnection(self, ws):
        """Handle new WebSocket connection"""
        self.clients.add(ws)

        # Mock WebSocket event handlers if not already set
        if not hasattr(ws, 'on'):
            ws.on = Mock()
        if not hasattr(ws, 'send'):
            ws.send = Mock()
        if not hasattr(ws, 'readyState'):
            ws.readyState = 1  # WebSocket.OPEN

        self.sendInitialState(ws)

        return ws
    
    def sendInitialState(self, ws):
        """Send initial state to new client"""
        message = WebSocketMessage('initial_state', {
            'connected': self.hardwareInterface.isConnected(),
            'port': self.hardwareInterface.getSerialPort(),
            'pins': self.hardwareInterface.getPinStates()
        })
        self.sendToClient(ws, message)
    
    async def handleMessage(self, ws, message: Dict[str, Any]):
        """Handle incoming WebSocket message"""
        msg_type = message.get('type')
        
        if msg_type == 'ping':
            response = WebSocketMessage('pong', {'timestamp': message.get('timestamp')})
            self.sendToClient(ws, response)
        
        elif msg_type == 'hardware_command':
            await self.handleHardwareCommand(ws, message.get('data', {}))
        
        elif msg_type == 'get_pin_states':
            response = WebSocketMessage('pin_states', self.hardwareInterface.getPinStates())
            self.sendToClient(ws, response)
        
        else:
            self.sendError(ws, f'Unknown message type: {msg_type}')
    
    async def handleHardwareCommand(self, ws, commandData: Dict[str, Any]):
        """Handle hardware command message"""
        command = commandData.get('command')
        pin = commandData.get('pin')
        value = commandData.get('value')
        
        try:
            # Create hardware command based on type
            if command == 'write_pin':
                hw_command = {'command': f'WRITE_PIN {pin} {value}', 'expectResponse': True}
            elif command == 'read_pin':
                hw_command = {'command': f'READ_PIN {pin}', 'expectResponse': True}
            elif command == 'read_adc':
                hw_command = {'command': f'READ_ADC {pin}', 'expectResponse': True}
            elif command == 'set_pwm':
                frequency = value.get('frequency', 1000)
                duty_cycle = value.get('dutyCycle', 50)
                hw_command = {'command': f'SET_PWM {pin} {frequency} {duty_cycle}', 'expectResponse': True}
            elif command == 'ping':
                hw_command = {'command': 'PING', 'expectResponse': True}
            else:
                raise ValueError(f'Unknown hardware command: {command}')
            
            response = await self.hardwareInterface.sendCommand(hw_command)

            message_data = {
                'command': command,
                'pin': pin,
                'value': value,
                'response': response,
                'success': response.get('success', False)
            }
            message = WebSocketMessage('command_response', message_data)
            self.sendToClient(ws, message)
            
        except Exception as e:
            self.sendError(ws, f'Hardware command failed: {str(e)}')
    
    def sendToClient(self, ws, message):
        """Send message to specific client"""
        if hasattr(ws, 'send') and getattr(ws, 'readyState', 1) == 1:
            if isinstance(message, WebSocketMessage):
                message_dict = {
                    'type': message.type,
                    'data': message.data,
                    'timestamp': message.timestamp
                }
            else:
                message_dict = message
            ws.send(json.dumps(message_dict))
    
    def sendError(self, ws, error: str):
        """Send error message to client"""
        message = WebSocketMessage('error', {'error': error})
        self.sendToClient(ws, message)
    
    def broadcast(self, message):
        """Broadcast message to all connected clients"""
        if isinstance(message, WebSocketMessage):
            message_dict = {
                'type': message.type,
                'data': message.data,
                'timestamp': message.timestamp
            }
        else:
            # Handle dict messages
            message_dict = message
        message_str = json.dumps(message_dict)
        
        # Remove closed connections while broadcasting
        closed_clients = set()
        for client in self.clients:
            if hasattr(client, 'readyState') and client.readyState == 1:
                if hasattr(client, 'send'):
                    client.send(message_str)
            else:
                closed_clients.add(client)
        
        # Remove closed clients
        self.clients -= closed_clients
    
    def getClientCount(self) -> int:
        """Get number of connected clients"""
        return len(self.clients)
