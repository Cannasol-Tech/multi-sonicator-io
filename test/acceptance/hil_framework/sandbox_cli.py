#!/usr/bin/env python3
"""
Sandbox CLI - Interactive sandbox CLI for HIL testing and debugging

This module provides an interactive command-line interface for manual HIL testing,
hardware debugging, and integration with acceptance testing framework.

Author: Cannasol Technologies  
License: Proprietary
"""

import time
import sys
import subprocess
import select
import threading
from typing import Optional, List, Dict, Any

# Handle both direct execution and module imports
try:
    from .hardware_interface import HardwareInterface
    from .logger import HILLogger
except ImportError:
    # Direct execution - add parent directory to path
    sys.path.insert(0, '.')
    from hardware_interface import HardwareInterface
    from logger import HILLogger


class SandboxCLI:
    """Interactive sandbox CLI integrated with acceptance testing framework"""
    
    def __init__(self, arduino_interface: Optional[HardwareInterface] = None):
        """Initialize sandbox CLI with Arduino interface"""
        self.arduino = arduino_interface or HardwareInterface()
        self.logger = HILLogger()
        self.monitoring_enabled = False
        self.running = True
        self.last_monitor_suggestion = 0
        self.monitor_thread = None
        
    def run_interactive_loop(self):
        """Main interactive command loop for manual acceptance testing"""
        print("🔧 HIL Sandbox Mode - Type 'help' for commands")
        print("🔍 Auto-detecting Arduino hardware...")
        
        # Show detected port information
        if hasattr(self.arduino, 'serial_port') and self.arduino.serial_port:
            print(f"📡 Detected port: {self.arduino.serial_port}")
        
        print("Connecting to hardware...")
        
        if not self.arduino.verify_connection():
            print("❌ Failed to connect to Arduino Test Harness")
            print("   Check serial port and hardware connections")
            print(f"   Attempted port: {self.arduino.serial_port}")
            print("\n🔧 Troubleshooting:")
            print("   1. Ensure Arduino is connected via USB")
            print("   2. Check Arduino Test Harness firmware is loaded")
            print("   3. Verify port permissions (ls -la /dev/tty.*)")
            print("   4. Try running with --port <specific_port>")
            return
            
        print(f"✅ Connected to Arduino Test Harness on {self.arduino.serial_port}")
        print()
        
        try:
            while self.running:
                if not self.monitoring_enabled:
                    self.suggest_monitoring_every_15_seconds()
                    
                # Get user input
                try:
                    command = input("hil> ").strip()
                    if command:
                        self.process_command(command)
                except (EOFError, KeyboardInterrupt):
                    print("\nExiting sandbox mode...")
                    break
                    
        finally:
            self.cleanup()
    
    def process_command(self, command: str):
        """Process sandbox commands for manual hardware interaction"""
        cmd_parts = command.strip().lower().split()
        if not cmd_parts:
            return
            
        cmd = cmd_parts[0]
        
        try:
            if cmd == 'monitor_on':
                self.enable_monitoring()
            elif cmd == 'monitor_off':
                self.disable_monitoring()
            elif cmd == 'help':
                self.show_help()
            elif cmd == 'status':
                self.show_status()
            elif cmd == 'set':
                self.set_pin_state(cmd_parts[1:])
            elif cmd == 'read':
                self.read_pin_state(cmd_parts[1:])
            elif cmd == 'adc':
                self.read_adc(cmd_parts[1:])
            elif cmd == 'reset':
                self.reset_atmega()
            elif cmd == 'upload':
                self.upload_firmware(cmd_parts[1:])
            elif cmd == 'build':
                self.build_firmware()
            elif cmd == 'clean':
                self.clean_build()
            elif cmd == 'run_acceptance':
                self.run_acceptance_tests(cmd_parts[1:])
            elif cmd == 'modbus':
                self.modbus_command(cmd_parts[1:])
            elif cmd == 'ping':
                self.ping_hardware()
            elif cmd == 'quit' or cmd == 'exit':
                self.quit_sandbox()
            else:
                print(f"❌ Unknown command: {cmd}. Type 'help' for available commands.")
                
        except Exception as e:
            print(f"❌ Command error: {e}")
            self.logger.error(f"Command '{command}' failed: {e}")
    
    def enable_monitoring(self):
        """Enable real-time pin monitoring"""
        if self.monitoring_enabled:
            print("📊 Monitoring already enabled")
            return
            
        self.monitoring_enabled = True
        self.monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self.monitor_thread.start()
        print("📊 Real-time pin monitoring enabled")
        
    def disable_monitoring(self):
        """Disable real-time pin monitoring"""
        if not self.monitoring_enabled:
            print("📊 Monitoring already disabled")
            return
            
        self.monitoring_enabled = False
        if self.monitor_thread:
            self.monitor_thread.join(timeout=2)
        print("📊 Real-time pin monitoring disabled")
        
    def _monitor_loop(self):
        """Background thread for pin monitoring"""
        while self.monitoring_enabled:
            try:
                status = self._get_pin_status_safe()
                if status:
                    self._display_pin_status(status)
                else:
                    # Fallback: create mock status for display
                    status = self._create_fallback_status()
                    self._display_pin_status(status)
                time.sleep(1)
            except Exception as e:
                self.logger.error(f"Monitor loop error: {e}")
                # Don't break - keep trying
                time.sleep(2)
                
    def _display_pin_status(self, status: Dict[str, Any]):
        """Display beautiful real-time pin status"""
        import os
        from datetime import datetime

        # Clear screen for clean display
        os.system('clear' if os.name == 'posix' else 'cls')

        # Header with timestamp
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print("=" * 80)
        print(f"🔧 HIL HARDWARE SANDBOX - REAL-TIME DUT MONITOR")
        print(f"📅 {timestamp}")
        print("=" * 80)
        print()

        # DUT Status Overview
        print("🎯 DEVICE UNDER TEST (ATmega32A) STATUS:")
        print("-" * 50)

        # Group pins by function for better organization
        sonicator_pins = {
            'FREQ_DIV10_4': 'Frequency Divider /10',
            'FREQ_LOCK_4': 'Frequency Lock Signal',
            'OVERLOAD_4': 'Overload Detection',
            'START_4': 'Start Control',
            'RESET_4': 'Reset Control',
            'POWER_SENSE_4': 'Power Sense (ADC)',
            'AMPLITUDE_ALL': 'Amplitude Control (PWM)'
        }

        system_pins = {
            'UART_RXD': 'UART Receive',
            'UART_TXD': 'UART Transmit',
            'STATUS_LED': 'Status LED'
        }

        # Display Sonicator Control Pins
        print("🔊 SONICATOR CONTROL PINS:")
        for pin, description in sonicator_pins.items():
            self._display_pin_line(pin, description, status)

        print()

        # Display System Pins
        print("⚙️  SYSTEM PINS:")
        for pin, description in system_pins.items():
            self._display_pin_line(pin, description, status)

        print()
        print("-" * 80)
        print("💡 Commands: monitor_off | status | set <pin> <state> | read <pin> | quit")
        print("=" * 80)

    def _display_pin_line(self, pin: str, description: str, status: Dict[str, Any]):
        """Display a single pin status line with formatting"""
        # Get pin state from status
        pin_state = status.get(pin, 'UNKNOWN')

        # Format pin name (left-aligned, 15 chars)
        pin_name = f"{pin:<15}"

        # Format description (left-aligned, 25 chars)
        desc = f"{description:<25}"

        # Format state with color coding
        if isinstance(pin_state, bool):
            if pin_state:
                state_display = "🟢 HIGH"
                state_color = "\033[92m"  # Green
            else:
                state_display = "🔴 LOW "
                state_color = "\033[91m"  # Red
        elif isinstance(pin_state, (int, float)):
            # ADC value
            voltage = pin_state * 5.0 / 1023.0  # Convert to voltage
            state_display = f"📊 {pin_state:4d} ({voltage:.2f}V)"
            state_color = "\033[94m"  # Blue
        else:
            state_display = f"❓ {str(pin_state):<8}"
            state_color = "\033[93m"  # Yellow

        reset_color = "\033[0m"  # Reset

        print(f"  {pin_name} | {desc} | {state_color}{state_display}{reset_color}")

    def _get_pin_status_safe(self):
        """Safely get pin status with error handling"""
        from datetime import datetime
        try:
            return self.arduino.get_pin_status()
        except Exception as e:
            self.logger.error(f"Failed to get pin status: {e}")
            return {
                'connection_error': True,
                'error_message': str(e),
                'timestamp': datetime.now().isoformat()
            }

    def _create_fallback_status(self):
        """Create fallback status when hardware is not responding"""
        from datetime import datetime
        return {
            'FREQ_DIV10_4': False,
            'FREQ_LOCK_4': False,
            'OVERLOAD_4': False,
            'START_4': False,
            'RESET_4': False,
            'POWER_SENSE_4': 0,
            'AMPLITUDE_ALL': 0,
            'UART_RXD': False,
            'UART_TXD': False,
            'STATUS_LED': False,
            'connection_status': 'DISCONNECTED',
            'last_update': datetime.now().strftime("%H:%M:%S")
        }
        
    def show_help(self):
        """Display available sandbox commands including acceptance testing"""
        help_text = """
🔧 HIL Sandbox Commands:
  monitor_on              - Enable real-time pin monitoring
  monitor_off             - Disable real-time pin monitoring  
  status                  - Show current pin states
  set <pin> <state>       - Set output pin state (e.g., 'set PC0 1')
  read <pin>              - Read pin state (e.g., 'read PD3')
  adc <channel>           - Read ADC channel (e.g., 'adc A1')
  reset                   - Reset ATmega32A chip
  upload [firmware.hex]   - Upload firmware to ATmega32A
  build                   - Build ATmega32A firmware
  clean                   - Clean build (wipe previous builds)
  modbus <read|write> <addr> [value] - MODBUS operations
  run_acceptance [args]   - Run Behave acceptance tests with HIL
  ping                    - Test hardware connection
  help                    - Show this help message
  quit                    - Exit sandbox mode

📋 Pin Names (from pin matrix):
  Sonicator 4: FREQ_DIV10_4, FREQ_LOCK_4, OVERLOAD_4, START_4, RESET_4
  Analog: POWER_SENSE_4, AMPLITUDE_ALL
  System: UART_RXD, UART_TXD, STATUS_LED
        """
        print(help_text)
        
    def show_status(self):
        """Show current hardware status"""
        print("📊 Hardware Status:")
        
        try:
            status = self.arduino.get_pin_status()
            if status:
                for pin, state in status.items():
                    print(f"  {pin}: {state}")
            else:
                print("  ❌ Could not retrieve pin status")
                
        except Exception as e:
            print(f"  ❌ Status read error: {e}")
            
    def set_pin_state(self, args: List[str]):
        """Set pin state command"""
        if len(args) < 2:
            print("❌ Usage: set <pin> <state>")
            print("   Example: set START_4 1")
            return
            
        pin = args[0].upper()
        state_str = args[1].upper()
        
        if state_str in ['1', 'HIGH', 'ON']:
            state = True
        elif state_str in ['0', 'LOW', 'OFF']:
            state = False
        else:
            print("❌ State must be: 1/HIGH/ON or 0/LOW/OFF")
            return
            
        success = self.arduino.write_gpio_pin(pin, state)
        if success:
            print(f"✅ Set {pin} to {'HIGH' if state else 'LOW'}")
        else:
            print(f"❌ Failed to set {pin}")
            
    def read_pin_state(self, args: List[str]):
        """Read pin state command"""
        if len(args) < 1:
            print("❌ Usage: read <pin>")
            print("   Example: read OVERLOAD_4")
            return
            
        pin = args[0].upper()
        state = self.arduino.read_gpio_pin(pin)
        
        if state is not None:
            print(f"📊 {pin}: {'HIGH' if state else 'LOW'}")
        else:
            print(f"❌ Failed to read {pin}")
            
    def read_adc(self, args: List[str]):
        """Read ADC channel command"""
        if len(args) < 1:
            print("❌ Usage: adc <channel>")
            print("   Example: adc POWER_SENSE_4")
            return
            
        channel = args[0].upper()
        adc_value = self.arduino.read_adc_channel(channel)
        
        if adc_value is not None:
            voltage = self.arduino.adc_to_voltage(adc_value)
            print(f"📊 {channel}: {adc_value} ({voltage:.3f}V)")
        else:
            print(f"❌ Failed to read ADC {channel}")
            
    def reset_atmega(self):
        """Reset ATmega32A chip"""
        print("🔄 Resetting ATmega32A...")
        success = self.arduino.reset_target()
        
        if success:
            print("✅ ATmega32A reset successful")
        else:
            print("❌ ATmega32A reset failed")
            
    def upload_firmware(self, args: List[str]):
        """Upload firmware to ATmega32A"""
        if args:
            firmware_path = args[0]
        else:
            # Default firmware path
            firmware_path = "firmware.hex"
            
        print(f"⬆️ Uploading firmware: {firmware_path}")
        
        # This would integrate with the programmer module
        # For now, show command that would be executed
        print(f"   avrdude -c arduino -p atmega32 -U flash:w:{firmware_path}:i")
        print("   (Programmer integration not yet implemented)")
        
    def build_firmware(self):
        """Build ATmega32A firmware"""
        print("🔨 Building firmware...")
        
        try:
            # Run platformio build
            result = subprocess.run(
                ['pio', 'run'],
                cwd='../../..',  # Go to project root
                capture_output=True,
                text=True,
                timeout=60
            )
            
            if result.returncode == 0:
                print("✅ Build successful")
            else:
                print("❌ Build failed:")
                print(result.stderr)
                
        except subprocess.TimeoutExpired:
            print("❌ Build timed out")
        except Exception as e:
            print(f"❌ Build error: {e}")
            
    def clean_build(self):
        """Clean build artifacts"""
        print("🧹 Cleaning build...")
        
        try:
            result = subprocess.run(
                ['pio', 'run', '--target', 'clean'],
                cwd='../../..',
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode == 0:
                print("✅ Clean successful")
            else:
                print("❌ Clean failed:")
                print(result.stderr)
                
        except Exception as e:
            print(f"❌ Clean error: {e}")
            
    def modbus_command(self, args: List[str]):
        """Execute MODBUS commands"""
        if len(args) < 2:
            print("❌ Usage: modbus <read|write> <address> [value]")
            print("   Example: modbus read 0000")
            print("   Example: modbus write 0100 0001")
            return
            
        operation = args[0].lower()
        address = int(args[1], 16)
        
        if operation == 'read':
            value = self.arduino.modbus_read_register(address)
            if value is not None:
                print(f"📊 MODBUS Read 0x{address:04X}: 0x{value:04X} ({value})")
            else:
                print(f"❌ MODBUS read failed for 0x{address:04X}")
                
        elif operation == 'write':
            if len(args) < 3:
                print("❌ Write requires value: modbus write <address> <value>")
                return
                
            value = int(args[2], 16)
            success = self.arduino.modbus_write_register(address, value)
            
            if success:
                print(f"✅ MODBUS Write 0x{address:04X}: 0x{value:04X}")
            else:
                print(f"❌ MODBUS write failed for 0x{address:04X}")
        else:
            print("❌ Operation must be 'read' or 'write'")
            
    def ping_hardware(self):
        """Test hardware connection"""
        print("🏓 Pinging hardware...")
        
        if self.arduino.verify_connection():
            print("✅ Hardware ping successful")
        else:
            print("❌ Hardware ping failed")
            
    def run_acceptance_tests(self, test_args: List[str]):
        """Run Behave acceptance tests from sandbox mode"""
        print(f"🧪 Running acceptance tests...")
        
        behave_cmd = ['behave', 'test/acceptance']
        if test_args:
            behave_cmd.extend(test_args)
            
        print(f"   Command: {' '.join(behave_cmd)}")
        
        try:
            result = subprocess.run(
                behave_cmd,
                cwd='../../..',
                timeout=300  # 5 minute timeout
            )
            
            if result.returncode == 0:
                print("✅ Acceptance tests passed")
            else:
                print("❌ Acceptance tests failed")
                
        except subprocess.TimeoutExpired:
            print("❌ Acceptance tests timed out")
        except Exception as e:
            print(f"❌ Test execution error: {e}")
    
    def suggest_monitoring_every_15_seconds(self):
        """Suggest enabling monitoring periodically"""
        current_time = time.time()
        if current_time - self.last_monitor_suggestion > 15:
            print("💡 Tip: Type 'monitor_on' to enable real-time pin monitoring")
            self.last_monitor_suggestion = current_time
            
    def quit_sandbox(self):
        """Exit sandbox mode"""
        print("👋 Exiting HIL sandbox mode...")
        self.running = False
        
    def cleanup(self):
        """Cleanup resources"""
        self.disable_monitoring()
        if self.arduino:
            self.arduino.cleanup()


def main():
    """CLI entry point for sandbox mode"""
    import argparse
    
    parser = argparse.ArgumentParser(description="HIL Sandbox CLI")
    parser.add_argument('--port', default='/dev/ttyUSB0', help='Serial port for Arduino Test Harness')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate')
    
    args = parser.parse_args()
    
    # Initialize hardware interface
    arduino = HardwareInterface(args.port, args.baud)
    
    # Start sandbox CLI
    sandbox = SandboxCLI(arduino)
    sandbox.run_interactive_loop()


if __name__ == '__main__':
    main()
