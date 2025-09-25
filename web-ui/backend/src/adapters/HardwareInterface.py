#!/usr/bin/env python3
"""
Hardware Interface Python Script
Executed by Node.js HardwareInterface.ts
"""

import os
import sys
import json

# Add paths for HIL framework
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../test/acceptance'))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../scripts'))

# Check if hardware mode is required
hardware_present = os.environ.get('HARDWARE_PRESENT', 'false').lower() == 'true'

try:
    from test.acceptance.hil_framework.hil_controller import HILController
    from test.acceptance.hil_framework.hardware_interface import HardwareInterface as HILHardwareInterface
except ImportError as e:
    if hardware_present:
        raise ImportError(f"HIL framework not available but hardware mode required: {e}")
    else:
        # Fallback to mock for development
        print("HIL framework not available, using mock interface", file=sys.stderr)
        HILController = None
        HILHardwareInterface = None

class HardwareInterface:
    def __init__(self):
        self.connected = False
        self.serial_port = None
        # Mock pin states for development
        self.pin_states = {
            'FREQ_DIV10_4': 'LOW',
            'FREQ_LOCK_4': 'LOW',
            'OVERLOAD_4': 'LOW',
            'START_4': 'LOW',
            'RESET_4': 'LOW',
            'POWER_SENSE_4': 512,  # ADC value
            'AMPLITUDE_ALL': '0.0%',
            'STATUS_LED': 'LOW',
            'UART_TXD': 'LOW',
            'UART_RXD': 'HIGH'
        }
        
        if HILController:
            self.hil_controller = HILController()
            self.connected = True
            self.serial_port = 'hil'
        else:
            # Mock mode
            self.hil_controller = None

    def update_pin(self, signal, state):
        if self.hil_controller:
            # Use real HIL
            pass
        else:
            # Mock update
            self.pin_states[signal] = state
            print(f"Mock: Updated {signal} to {state}")

    def get_pin_state(self, signal):
        if self.hil_controller:
            # Use real HIL
            return 'LOW'  # Placeholder
        else:
            return self.pin_states.get(signal, 'LOW')

# Main interface
interface = HardwareInterface()

# Command processing loop
while True:
    try:
        line = input().strip()
        if not line:
            continue
            
        cmd = json.loads(line)
        
        if cmd['command'] == 'update_pin':
            interface.update_pin(cmd['signal'], cmd['state'])
            print(json.dumps({'status': 'ok'}))
        elif cmd['command'] == 'get_pin':
            state = interface.get_pin_state(cmd['signal'])
            print(json.dumps({'status': 'ok', 'state': state}))
        elif cmd['command'] == 'get_all_pins':
            print(json.dumps({'status': 'ok', 'pins': interface.pin_states}))
        else:
            print(json.dumps({'error': f'Unknown command: {cmd["command"]]}'}))
            
    except EOFError:
        break
    except Exception as e:
        print(json.dumps({'error': str(e)}))
