#!/usr/bin/env python3
"""
Arduino ISP Programmer - Interface for programming ATmega32A via Arduino as ISP

This module provides the programming interface for uploading firmware to the 
ATmega32A target using Arduino as ISP.

Author: Cannasol Technologies  
License: Proprietary
"""

import subprocess
import logging
import time
from pathlib import Path
from typing import Optional, List


class ArduinoISPProgrammer:
    """Arduino as ISP programmer interface for ATmega32A"""

    def __init__(self, programmer_port: str = '/dev/ttyUSB0', avrdude_conf: Optional[str] = None, bitclock: str = '125kHz'):
        """Initialize Arduino ISP programmer"""
        self.programmer_port = programmer_port
        self.logger = logging.getLogger(__name__)
        self.avrdude_cmd = 'avrdude'
        self.avrdude_conf = avrdude_conf
        self.bitclock = bitclock
        # ATmega32A programming parameters
        self.target_mcu = 'atmega32'
        # Use stk500v1 to match PlatformIO upload settings for Arduino as ISP
        self.programmer_type = 'stk500v1'
        self.baud_rate = 19200
        
    def verify_connection(self) -> bool:
        """Verify Arduino ISP programmer connection"""
        try:
            # Test avrdude availability
            result = subprocess.run(
                [self.avrdude_cmd, '-v'],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode != 0:
                self.logger.error("avrdude not available or not working")
                return False
                
            # Test programmer connection
            cmd = [self.avrdude_cmd]
            if self.avrdude_conf:
                cmd += ['-C', self.avrdude_conf]
            cmd += [
                '-c', self.programmer_type,
                '-p', self.target_mcu,
                '-P', self.programmer_port,
                '-b', str(self.baud_rate),
                '-B', self.bitclock,
                '-v'
            ]
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode == 0:
                self.logger.info("Arduino ISP programmer connection verified")
                return True
            else:
                self.logger.error(f"Programmer connection failed: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            self.logger.error("Programmer connection test timed out")
            return False
        except Exception as e:
            self.logger.error(f"Programmer verification error: {e}")
            return False
            
    def program_firmware(self, firmware_path: str) -> bool:
        """Program firmware to ATmega32A via Arduino ISP"""
        try:
            if not Path(firmware_path).exists():
                self.logger.error(f"Firmware file not found: {firmware_path}")
                return False
                
            self.logger.info(f"Programming firmware: {firmware_path}")
            
            # Construct avrdude command
            cmd = [self.avrdude_cmd]
            if self.avrdude_conf:
                cmd += ['-C', self.avrdude_conf]
            cmd += [
                '-c', self.programmer_type,
                '-p', self.target_mcu,
                '-P', self.programmer_port,
                '-b', str(self.baud_rate),
                '-B', self.bitclock,
                '-U', f'flash:w:{firmware_path}:i',
                '-v'
            ]
            
            self.logger.debug(f"Programming command: {' '.join(cmd)}")
            
            # Execute programming
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=120  # 2 minutes timeout
            )
            
            if result.returncode == 0:
                self.logger.info("Firmware programming completed successfully")
                return True
            else:
                self.logger.error(f"Programming failed: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            self.logger.error("Firmware programming timed out")
            return False
        except Exception as e:
            self.logger.error(f"Programming error: {e}")
            return False
            
    def read_fuses(self) -> Optional[dict]:
        """Read ATmega32A fuse settings"""
        try:
            fuses = {}
            
            # Read low fuse
            cmd = [
                self.avrdude_cmd,
                '-c', self.programmer_type,
                '-p', self.target_mcu,
                '-P', self.programmer_port,
                '-b', str(self.baud_rate),
                '-U', 'lfuse:r:-:h'
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            if result.returncode == 0:
                # Parse fuse value from output
                for line in result.stderr.split('\n'):
                    if 'lfuse' in line and '0x' in line:
                        fuses['lfuse'] = line.split('0x')[1][:2]
                        
            # Similar for high fuse and extended fuse
            # Simplified for now - can be expanded if needed
            
            return fuses if fuses else None
            
        except Exception as e:
            self.logger.error(f"Fuse read error: {e}")
            return None
            
    def verify_firmware(self, firmware_path: str) -> bool:
        """Verify programmed firmware matches file"""
        try:
            if not Path(firmware_path).exists():
                return False
                
            cmd = [self.avrdude_cmd]
            if self.avrdude_conf:
                cmd += ['-C', self.avrdude_conf]
            cmd += [
                '-c', self.programmer_type,
                '-p', self.target_mcu,
                '-P', self.programmer_port,
                '-b', str(self.baud_rate),
                '-B', self.bitclock,
                '-U', f'flash:v:{firmware_path}:i'
            ]
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=60
            )
            
            if result.returncode == 0:
                self.logger.info("Firmware verification successful")
                return True
            else:
                self.logger.error(f"Firmware verification failed: {result.stderr}")
                return False
                
        except Exception as e:
            self.logger.error(f"Verification error: {e}")
            return False
            
    def erase_chip(self) -> bool:
        """Erase ATmega32A chip"""
        try:
            cmd = [self.avrdude_cmd]
            if self.avrdude_conf:
                cmd += ['-C', self.avrdude_conf]
            cmd += [
                '-c', self.programmer_type,
                '-p', self.target_mcu,
                '-P', self.programmer_port,
                '-b', str(self.baud_rate),
                '-B', self.bitclock,
                '-e'
            ]
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode == 0:
                self.logger.info("Chip erase completed")
                return True
            else:
                self.logger.error(f"Chip erase failed: {result.stderr}")
                return False
                
        except Exception as e:
            self.logger.error(f"Erase error: {e}")
            return False
            
    def cleanup(self) -> None:
        """Clean up programmer interface"""
        # No persistent connections to clean up for avrdude
        self.logger.info("Programmer interface cleanup completed")
