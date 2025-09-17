#!/usr/bin/env python3
"""
MODBUS Frequency Monitor

Standalone utility for monitoring frequency registers on ATmega32A hardware.
Continuously reads and displays MODBUS register 40370 (REG_SON4_FREQ_DIV10_HZ)
to verify frequency measurement and reporting functionality.

Usage:
    python3 frequency_modbus_monitor.py [options]

Options:
    --port PORT          Serial port (default: auto-detect)
    --baud RATE         Baud rate (default: 115200)  
    --interval SECONDS  Reading interval (default: 0.5)
    --register REG      MODBUS register to monitor (default: 40370)
    --samples COUNT     Number of samples to take (default: infinite)
    --log-file FILE     Log readings to file
    --tolerance HZ      Highlight readings outside tolerance
    --help              Show this help message
"""

import sys
import time
import argparse
import logging
from datetime import datetime
from pathlib import Path

# Add test framework to path for MODBUS client
sys.path.append(str(Path(__file__).parent.parent / 'test' / 'acceptance'))

try:
    from pymodbus.client import ModbusSerialClient
    from pymodbus.framer.rtu_framer import ModbusRtuFramer
except ImportError:
    print("❌ Error: pymodbus not available. Install with: pip install pymodbus")
    sys.exit(1)

class FrequencyModbusMonitor:
    def __init__(self, port=None, baud_rate=115200, register=40370):
        self.port = port
        self.baud_rate = baud_rate
        self.register = register
        self.client = None
        self.readings = []
        self.start_time = None
        
    def find_serial_port(self):
        """Auto-detect serial port for ATmega32A"""
        try:
            import serial.tools.list_ports
            
            # Look for common Arduino/ATmega32A USB serial adapters
            ports = list(serial.tools.list_ports.comports())
            
            for port in ports:
                if any(keyword in port.description.lower() for keyword in 
                      ['arduino', 'usb serial', 'ch340', 'cp210', 'ftdi']):
                    return port.device
                    
            # Fallback to first available port
            if ports:
                return ports[0].device
                
        except Exception as e:
            logging.debug(f"Port detection failed: {e}")
            
        return None
        
    def connect(self):
        """Connect to MODBUS RTU device"""
        if not self.port:
            self.port = self.find_serial_port()
            
        if not self.port:
            raise Exception("No serial port found. Please specify with --port")
            
        print(f"🔌 Connecting to ATmega32A via {self.port} at {self.baud_rate} baud...")
        
        # Set up MODBUS RTU client
        self.client = ModbusSerialClient(
            port=self.port,
            baudrate=self.baud_rate,
            parity='N',
            stopbits=1,
            bytesize=8,
            timeout=1.0,
            framer=ModbusRtuFramer
        )
        
        # Set slave/unit ID for ATmega32A (typically 2 based on config)
        self.client.unit_id = 2
        
        if not self.client.connect():
            raise Exception(f"Failed to connect to MODBUS device on {self.port}")
            
        print(f"✅ Connected to ATmega32A MODBUS RTU device")
        self.start_time = time.time()
        
    def disconnect(self):
        """Disconnect from MODBUS device"""
        if self.client and self.client.connected:
            self.client.close()
            print("🔌 Disconnected from MODBUS device")
            
    def read_frequency_register(self):
        """Read frequency register and return value with metadata"""
        if not self.client or not self.client.connected:
            raise Exception("MODBUS client not connected")
            
        # Convert register address (40370 -> 369)
        address = self.register - 40001 if self.register >= 40001 else self.register
        
        try:
            # Read holding register
            result = self.client.read_holding_registers(address=address, count=1)
            
            if result.isError():
                return {
                    'timestamp': time.time(),
                    'register': self.register,
                    'value': None,
                    'error': str(result),
                    'success': False
                }
                
            frequency_hz = result.registers[0]
            
            return {
                'timestamp': time.time(),
                'register': self.register,
                'value': frequency_hz,
                'error': None,
                'success': True
            }
            
        except Exception as e:
            return {
                'timestamp': time.time(),
                'register': self.register,
                'value': None,
                'error': str(e),
                'success': False
            }
            
    def format_reading(self, reading, tolerance=None):
        """Format reading for display"""
        timestamp = datetime.fromtimestamp(reading['timestamp'])
        elapsed = reading['timestamp'] - self.start_time if self.start_time else 0
        
        if not reading['success']:
            return f"[{timestamp.strftime('%H:%M:%S')}] +{elapsed:6.1f}s | ❌ ERROR: {reading['error']}"
            
        freq = reading['value']
        status = "📊"
        
        # Check tolerance if specified
        if tolerance and len(self.readings) > 1:
            prev_reading = self.readings[-2]
            if prev_reading['success']:
                diff = abs(freq - prev_reading['value'])
                if diff > tolerance:
                    status = "⚠️"
                    
        return f"[{timestamp.strftime('%H:%M:%S')}] +{elapsed:6.1f}s | {status} Register {self.register}: {freq:5d} Hz"
        
    def calculate_statistics(self):
        """Calculate statistics from collected readings"""
        successful_readings = [r for r in self.readings if r['success']]
        
        if not successful_readings:
            return None
            
        values = [r['value'] for r in successful_readings]
        
        return {
            'count': len(successful_readings),
            'min': min(values),
            'max': max(values),
            'avg': sum(values) / len(values),
            'range': max(values) - min(values),
            'last': values[-1] if values else None,
            'success_rate': len(successful_readings) / len(self.readings) * 100
        }
        
    def log_to_file(self, reading, log_file):
        """Log reading to file"""
        timestamp = datetime.fromtimestamp(reading['timestamp'])
        
        with open(log_file, 'a') as f:
            if reading['success']:
                f.write(f"{timestamp.isoformat()},{self.register},{reading['value']},SUCCESS,\n")
            else:
                f.write(f"{timestamp.isoformat()},{self.register},,ERROR,\"{reading['error']}\"\n")
                
    def monitor(self, interval=0.5, max_samples=None, tolerance=None, log_file=None):
        """Monitor frequency register continuously"""
        sample_count = 0
        
        print(f"📊 Monitoring MODBUS register {self.register} (REG_SON4_FREQ_DIV10_HZ)")
        print(f"⏱️  Reading interval: {interval}s")
        if max_samples:
            print(f"🔢 Maximum samples: {max_samples}")
        if tolerance:
            print(f"⚠️  Tolerance threshold: ±{tolerance} Hz")
        if log_file:
            print(f"📄 Logging to: {log_file}")
            # Write CSV header
            with open(log_file, 'w') as f:
                f.write("timestamp,register,frequency_hz,status,error\n")
        print("-" * 80)
        
        try:
            while True:
                # Read frequency register
                reading = self.read_frequency_register()
                self.readings.append(reading)
                
                # Display reading
                print(self.format_reading(reading, tolerance))
                
                # Log to file if requested
                if log_file:
                    self.log_to_file(reading, log_file)
                    
                sample_count += 1
                
                # Check if we've reached max samples
                if max_samples and sample_count >= max_samples:
                    break
                    
                # Wait for next reading
                time.sleep(interval)
                
        except KeyboardInterrupt:
            print("\n\n⏹️  Monitoring stopped by user")
            
        # Display final statistics
        self.show_statistics()
        
    def show_statistics(self):
        """Show monitoring statistics"""
        stats = self.calculate_statistics()
        
        print("\n" + "=" * 80)
        print("📈 MONITORING STATISTICS")
        print("=" * 80)
        
        if not stats:
            print("❌ No successful readings collected")
            return
            
        duration = time.time() - self.start_time if self.start_time else 0
        
        print(f"⏱️  Total Duration: {duration:.1f} seconds")
        print(f"📊 Total Readings: {len(self.readings)} ({stats['success_rate']:.1f}% successful)")
        print(f"📈 Frequency Statistics:")
        print(f"   • Current: {stats['last']} Hz")
        print(f"   • Average: {stats['avg']:.1f} Hz")
        print(f"   • Minimum: {stats['min']} Hz") 
        print(f"   • Maximum: {stats['max']} Hz")
        print(f"   • Range: {stats['range']} Hz")
        
        if stats['range'] > 0:
            stability = (1.0 - stats['range'] / max(stats['avg'], 1)) * 100
            print(f"📐 Stability: {stability:.1f}%")

def main():
    """Main function"""
    parser = argparse.ArgumentParser(
        description="Monitor MODBUS frequency register on ATmega32A hardware",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 frequency_modbus_monitor.py
  python3 frequency_modbus_monitor.py --port /dev/ttyUSB0 --interval 1.0
  python3 frequency_modbus_monitor.py --samples 100 --log-file frequency_log.csv
  python3 frequency_modbus_monitor.py --register 40370 --tolerance 100
        """
    )
    
    parser.add_argument('--port', type=str, help='Serial port (auto-detect if not specified)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--interval', type=float, default=0.5, help='Reading interval in seconds (default: 0.5)')
    parser.add_argument('--register', type=int, default=40370, help='MODBUS register to monitor (default: 40370)')
    parser.add_argument('--samples', type=int, help='Number of samples to take (default: infinite)')
    parser.add_argument('--log-file', type=str, help='Log readings to CSV file')
    parser.add_argument('--tolerance', type=int, help='Highlight readings outside tolerance (Hz)')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logging')
    
    args = parser.parse_args()
    
    # Set up logging
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.WARNING)
        
    print("🎯 MODBUS Frequency Monitor")
    print("ATmega32A Frequency Register Monitoring Tool")
    print("=" * 80)
    
    monitor = FrequencyModbusMonitor(
        port=args.port,
        baud_rate=args.baud,
        register=args.register
    )
    
    try:
        monitor.connect()
        monitor.monitor(
            interval=args.interval,
            max_samples=args.samples,
            tolerance=args.tolerance,
            log_file=args.log_file
        )
    except Exception as e:
        print(f"❌ Error: {e}")
        return 1
    finally:
        monitor.disconnect()
        
    return 0

if __name__ == "__main__":
    sys.exit(main())