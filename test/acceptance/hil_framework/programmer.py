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
import os
import glob
import signal
import shutil
from pathlib import Path
from typing import Optional, List, Iterable


class ArduinoISPProgrammer:
    """Arduino as ISP programmer interface for ATmega32A"""

    def __init__(self, programmer_port: str = '/dev/ttyUSB0', avrdude_conf: Optional[str] = None, bitclock: str = '125kHz'):
        """Initialize Arduino ISP programmer"""
        self.programmer_port = programmer_port
        self.logger = logging.getLogger(__name__)
        self.avrdude_cmd = self._resolve_avrdude()
        self.avrdude_conf = avrdude_conf or self._resolve_avrdude_conf()
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
                # Attempt to fallback to PlatformIO-bundled avrdude
                self.avrdude_cmd = self._resolve_avrdude(force_fallback=True)
                self.avrdude_conf = self.avrdude_conf or self._resolve_avrdude_conf()
                result2 = subprocess.run([self.avrdude_cmd, '-v'], capture_output=True, text=True, timeout=10)
                if result2.returncode != 0:
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
                # Try freeing port and retry once
                self._free_port_users()
                time.sleep(1.0)
                return self.verify_connection()
                
        except subprocess.TimeoutExpired:
            self.logger.error("Programmer connection test timed out")
            return False
        except Exception as e:
            self.logger.error(f"Programmer verification error: {e}")
            self._free_port_users()
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
                '-v',
                '-D',  # do not erase chip before programming
                '-V'   # disable verification to speed up; verification can be separate
            ]
            
            self.logger.debug(f"Programming command: {' '.join(cmd)}")
            
            # Execute programming with up to 3 attempts, freeing port on failure
            attempts = 3
            for attempt in range(1, attempts + 1):
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    timeout=120
                )
                if result.returncode == 0:
                    self.logger.info("Firmware programming completed successfully")
                    return True
                else:
                    self.logger.error(f"Programming failed (attempt {attempt}/{attempts}): {result.stderr}")
                    # Free the port users and retry after short delay
                    self._free_port_users()
                    time.sleep(1.5)
            return False
                
        except subprocess.TimeoutExpired:
            self.logger.error("Firmware programming timed out")
            self._free_port_users()
            return False
        except Exception as e:
            self.logger.error(f"Programming error: {e}")
            self._free_port_users()
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

    # -------------------------- Internal helpers --------------------------- #

    def _resolve_avrdude(self, force_fallback: bool = False) -> str:
        """Resolve path to avrdude, checking system, local .pio, then ~/.platformio."""
        if not force_fallback:
            found = shutil.which('avrdude')
            if found:
                return found
        # Try project-local PlatformIO packages dir (.pio/packages)
        try:
            repo_root = Path(__file__).resolve().parents[3]
        except Exception:
            repo_root = Path.cwd()
        local_pio = repo_root / '.pio' / 'packages' / 'tool-avrdude' / 'avrdude'
        if local_pio.exists():
            return str(local_pio)
        # Fallback to user PlatformIO packages dir (~/.platformio/packages)
        home = os.path.expanduser('~')
        user_pio = os.path.join(home, '.platformio', 'packages', 'tool-avrdude', 'avrdude')
        return user_pio if os.path.exists(user_pio) else 'avrdude'

    def _resolve_avrdude_conf(self) -> Optional[str]:
        """Resolve path to avrdude.conf from local .pio or ~/.platformio packages."""
        try:
            repo_root = Path(__file__).resolve().parents[3]
        except Exception:
            repo_root = Path.cwd()
        local_conf = repo_root / '.pio' / 'packages' / 'tool-avrdude' / 'avrdude.conf'
        if local_conf.exists():
            return str(local_conf)
        home = os.path.expanduser('~')
        conf_path = os.path.join(home, '.platformio', 'packages', 'tool-avrdude', 'avrdude.conf')
        return conf_path if os.path.exists(conf_path) else None

    def _candidate_ports(self) -> Iterable[str]:
        """Generate candidate serial device paths to free if locked."""
        ports = set()
        if self.programmer_port:
            ports.add(self.programmer_port)
            # On macOS, tty.* is the dialin counterpart to cu.*
            if '/cu.' in self.programmer_port:
                ports.add(self.programmer_port.replace('/cu.', '/tty.'))
        # Also consider all usbmodem variants if pattern used
        for pat in ('/dev/cu.usbmodem*', '/dev/tty.usbmodem*'):
            for p in glob.glob(pat):
                ports.add(p)
        return ports

    def _pids_holding_device(self, device_path: str) -> List[int]:
        """Return list of PIDs that have the given device open using lsof."""
        try:
            result = subprocess.run(
                ['lsof', '-t', device_path],
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode != 0:
                return []
            pids = []
            for line in result.stdout.strip().splitlines():
                try:
                    pid = int(line.strip())
                    if pid != os.getpid():
                        pids.append(pid)
                except ValueError:
                    continue
            return pids
        except Exception:
            return []

    def _free_port_users(self) -> None:
        """Attempt to free the programmer serial port by terminating blocking processes."""
        try:
            victims = set()
            for dev in self._candidate_ports():
                for pid in self._pids_holding_device(dev):
                    victims.add(pid)
            if not victims:
                return
            self.logger.warning(f"Freeing serial port by terminating processes holding it: {sorted(victims)}")
            # First try gentle SIGTERM, then SIGKILL if necessary
            for pid in victims:
                try:
                    os.kill(pid, signal.SIGTERM)
                except Exception:
                    pass
            time.sleep(0.5)
            # Force kill remaining
            for pid in victims:
                try:
                    os.kill(pid, 0)
                    # still alive
                    os.kill(pid, signal.SIGKILL)
                except Exception:
                    pass
        except Exception as e:
            self.logger.warning(f"Failed to free serial port users: {e}")
