"""
Python-compatible version of HardwareInterface for testing
This mirrors the TypeScript HardwareInterface functionality for pytest
"""

import time
from typing import Dict, Any, Optional, List
from unittest.mock import Mock

class PinState:
    def __init__(self, pin: str, signal: str, direction: str, state: Any = 'LOW', description: str = ''):
        self.pin = pin
        self.signal = signal
        self.direction = direction
        self.state = state
        self.timestamp = int(time.time() * 1000)
        self.description = description

class HardwareInterface:
    """Python version of HardwareInterface for testing"""
    
    def __init__(self):
        self.connected = False
        self.serialPort = None
        self.pinStates = {}
        self.pythonProcess = None
        self.event_handlers = {}
        self.initializePinStates()
    
    def initializePinStates(self):
        """Initialize pin states based on pin-matrix.md (SOLE SOURCE OF TRUTH)"""
        initial_pins = [
            {'pin': 'D7', 'signal': 'FREQ_DIV10_4', 'direction': 'IN', 'description': 'Frequency ÷10 input to ATmega32A PB0'},
            {'pin': 'D8', 'signal': 'FREQ_LOCK_4', 'direction': 'IN', 'description': 'Frequency lock input to ATmega32A PB4'},
            {'pin': 'A2', 'signal': 'OVERLOAD_4', 'direction': 'IN', 'description': 'Overload input to ATmega32A PD3'},
            {'pin': 'A3', 'signal': 'START_4', 'direction': 'OUT', 'description': 'Start output from ATmega32A PC0'},
            {'pin': 'A4', 'signal': 'RESET_4', 'direction': 'OUT', 'description': 'Reset output from ATmega32A PC1'},
            {'pin': 'A1', 'signal': 'POWER_SENSE_4', 'direction': 'ANALOG', 'description': 'Power sense analog from ATmega32A PA7'},
            {'pin': 'D9', 'signal': 'AMPLITUDE_ALL', 'direction': 'IN', 'description': 'Amplitude PWM input to ATmega32A PD7'},
            {'pin': 'D10', 'signal': 'UART_RXD', 'direction': 'IN', 'description': 'UART RX to ATmega32A PD0'},
            {'pin': 'D11', 'signal': 'UART_TXD', 'direction': 'OUT', 'description': 'UART TX from ATmega32A PD1'},
            {'pin': 'D12', 'signal': 'STATUS_LED', 'direction': 'OUT', 'description': 'Status LED from ATmega32A PD2'}
        ]
        
        for pin_info in initial_pins:
            pin_state = PinState(
                pin=pin_info['pin'],
                signal=pin_info['signal'],
                direction=pin_info['direction'],
                state=0 if pin_info['direction'] == 'ANALOG' else 'LOW',
                description=pin_info['description']
            )
            self.pinStates[pin_info['signal']] = pin_state
    
    def getPinStates(self) -> Dict[str, Any]:
        """Get all pin states as dictionary"""
        return {signal: {
            'pin': pin_state.pin,
            'signal': pin_state.signal,
            'direction': pin_state.direction,
            'state': pin_state.state,
            'timestamp': pin_state.timestamp,
            'description': pin_state.description
        } for signal, pin_state in self.pinStates.items()}
    
    def isConnected(self) -> bool:
        """Check if hardware is connected"""
        return self.connected
    
    def getSerialPort(self) -> Optional[str]:
        """Get current serial port"""
        return self.serialPort
    
    def updatePinState(self, pin: str, data: str):
        """Update pin state from hardware response"""
        timestamp = int(time.time() * 1000)
        
        # Find pin by Arduino pin name or signal name
        for signal, pin_state in self.pinStates.items():
            if pin_state.pin == pin or signal == pin:
                if data == 'HIGH':
                    pin_state.state = 'HIGH'
                elif data == 'LOW':
                    pin_state.state = 'LOW'
                elif data.startswith('ADC='):
                    try:
                        adc_value = int(data.split('=')[1])
                        pin_state.state = adc_value
                    except (ValueError, IndexError):
                        pass
                
                pin_state.timestamp = timestamp
                break
    
    async def sendCommand(self, command: Dict[str, Any]) -> Dict[str, Any]:
        """Send command to hardware (mock for testing)"""
        if not self.connected:
            return {
                'success': False,
                'error': 'Hardware not connected',
                'timestamp': int(time.time() * 1000)
            }
        
        return {
            'success': True,
            'timestamp': int(time.time() * 1000)
        }
    
    def disconnect(self):
        """Disconnect from hardware"""
        if self.pythonProcess:
            self.pythonProcess.kill()
            self.pythonProcess = None
        self.connected = False
        self.serialPort = None
    
    def handlePythonMessage(self, message: Dict[str, Any]):
        """Handle messages from Python HIL process"""
        if message.get('type') == 'connection':
            if message.get('status') == 'connected':
                self.connected = True
                self.serialPort = message.get('port')
            else:
                self.connected = False
        elif message.get('type') == 'pin_state':
            self.updatePinState(message.get('pin'), message.get('data'))
        elif message.get('type') == 'error':
            self.emit('error', message.get('error'))
    
    # Mock event emitter functionality for testing
    def on(self, event: str, callback):
        """Mock event listener"""
        if event not in self.event_handlers:
            self.event_handlers[event] = []
        self.event_handlers[event].append(callback)

    def emit(self, event: str, *args):
        """Mock event emitter"""
        if event in self.event_handlers:
            for callback in self.event_handlers[event]:
                callback(*args)
