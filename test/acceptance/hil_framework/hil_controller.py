#!/usr/bin/env python3
"""
HIL Controller - Main controller for Hardware-in-the-Loop testing integration with Behave

This module provides the core HIL controller that integrates with the Behave BDD framework
for acceptance testing with real ATmega32A hardware.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import yaml
import time
import logging
import platform
from pathlib import Path
from typing import Dict, Optional, Any

from .hardware_interface import HardwareInterface
from .programmer import ArduinoISPProgrammer
from .logger import HILLogger


class HILController:
    """HIL Controller integrated with Behave acceptance testing framework"""

    def __init__(self, config_file='hil_config.yaml'):
        """Initialize HIL test framework with hardware configuration for Behave"""
        self.config_file = config_file
        self.config = self._load_config()
        self.logger = HILLogger()
        self.hardware_interface = None
        self.programmer = None
        self.hardware_ready = False

        self.logger.info("HIL Controller initialized")

    def _load_config(self) -> Dict[str, Any]:
        """Load HIL configuration from YAML file"""
        config_path = Path(__file__).parent / self.config_file

        # Default configuration if file doesn't exist
        config = {
            'hardware': {
                'target_mcu': 'ATmega32A',
                'programmer': 'arduino_as_isp',
                'programmer_port': '/dev/ttyUSB0',
                'target_serial_port': '/dev/ttyUSB1'
            },
            'power_supply': {
                'input_voltage': 24.0,
                'regulated_5v': 5.0,
                'tolerance': 0.1
            },
            'timing': {
                'crystal_frequency': 16000000,
                'serial_baud_rate': 115200
            },
            'modbus': {
                'slave_id': 2,
                'baud_rate': 115200,
                'timeout': 1.0
            },
            'behave': {
                'profile': 'hil',
                'junit_output': True,
                'junit_directory': 'acceptance-junit',
                'hardware_required': True
            }
        }

        if config_path.exists():
            try:
                with open(config_path, 'r') as f:
                    loaded_config = yaml.safe_load(f)
                    # Merge with defaults
                    config.update(loaded_config)
            except Exception as e:
                self.logger.warning(f"Failed to load config file {config_path}: {e}")

            return config

    def _auto_detect_programmer_port(self, configured_port: Optional[str]) -> str:
        """Resolve the ArduinoISP programmer serial port with platform-aware auto-detect.
        Preference order:
        1) If configured_port looks usable and exists, use it.
        2) On macOS, prefer /dev/cu.usbmodem* then /dev/cu.usbserial*.
        3) On Linux, prefer /dev/ttyUSB* then /dev/ttyACM*.
        4) Fallback to HardwareInterface auto-detection (same Arduino) when reasonable.
        5) Otherwise, return a platform default.
        """
        try:
            # 1) Use configured if present and exists
            if configured_port and Path(configured_port).exists():
                self.logger.info(f"Using configured programmer port: {configured_port}")
                return configured_port

            system = platform.system()
            candidates = []

            if system == "Darwin":
                import glob
                candidates.extend(sorted(glob.glob("/dev/cu.usbmodem*")))
                candidates.extend(sorted(glob.glob("/dev/cu.usbserial*")))
            elif system == "Linux":
                import glob
                candidates.extend(sorted(glob.glob("/dev/ttyUSB*")))
                candidates.extend(sorted(glob.glob("/dev/ttyACM*")))
            elif system == "Windows":
                # On Windows we can't easily test existence beyond naming; default to COM3 if not specified
                pass

            # 2) If any candidates found, take the first (stable enough for lab)
            for port in candidates:
                if Path(port).exists():
                    self.logger.info(f"Auto-detected programmer port: {port}")
                    return port

            # 3) As a heuristic, if hardware_interface already detected a serial port, reuse it
            if self.hardware_interface and getattr(self.hardware_interface, 'serial_port', None):
                port = self.hardware_interface.serial_port
                if system != "Windows" and Path(port).exists():
                    self.logger.info(f"Reusing hardware interface port for programmer: {port}")
                    return port

            # 4) Platform default fallback
            fallbacks = {
                "Darwin": "/dev/cu.usbmodem2101",
                "Linux": "/dev/ttyUSB0",
                "Windows": "COM3",
            }
            fallback = fallbacks.get(system, "/dev/ttyUSB0")
            self.logger.warning(f"No programmer port auto-detected, using fallback: {fallback}")
            return fallback
        except Exception as e:
            self.logger.error(f"Programmer port auto-detect failed: {e}")
            # Safe fallback
            return "/dev/ttyUSB0"

    def setup_hardware(self) -> bool:
        """Initialize hardware connections and verify basic connectivity with improved timing"""
        import time

        try:
            self.logger.info("Setting up HIL hardware connections...")

            # Initialize hardware interface
            serial_port = self.config['hardware']['target_serial_port']
            baud_rate = self.config['timing']['serial_baud_rate']

            self.hardware_interface = HardwareInterface(serial_port, baud_rate)

            # Initialize programmer with auto-detected port when needed
            programmer_port = self._auto_detect_programmer_port(self.config['hardware'].get('programmer_port'))
            # Persist the resolved port back into config for visibility
            self.config['hardware']['programmer_port'] = programmer_port
            self.programmer = ArduinoISPProgrammer(programmer_port)

            # TIMING FIX: Allow Arduino to fully initialize before attempting connection
            self.logger.info("Waiting for Arduino initialization (3 seconds)...")
            time.sleep(3.0)

            # BUFFER CLEARING FIX: Verify connectivity with retry mechanism
            connection_attempts = 3
            for attempt in range(connection_attempts):
                self.logger.info(f"Connection attempt {attempt + 1}/{connection_attempts}")

                if self.hardware_interface.verify_connection():
                    self.logger.info("✅ Hardware interface connection established")
                    break

                if attempt < connection_attempts - 1:
                    self.logger.info("Connection failed, retrying in 2 seconds...")
                    time.sleep(2.0)
            else:
                self.logger.error("Failed to establish hardware interface connection after all attempts")
                return False

            # STARTUP MESSAGE FIX: Send dummy command to clear any startup messages
            try:
                self.logger.info("Clearing Arduino startup messages...")
                dummy_response = self.hardware_interface.send_command("PING", read_timeout=1.0)
                self.logger.debug(f"Dummy command response: {dummy_response}")

                # Verify connection is stable with a second PING
                verify_response = self.hardware_interface.send_command("PING", read_timeout=2.0)
                if verify_response and "PONG" in verify_response:
                    self.logger.info("✅ Arduino communication verified and stable")
                else:
                    self.logger.warning(f"Arduino communication may be unstable: {verify_response}")

            except Exception as e:
                self.logger.warning(f"Startup message clearing failed: {e}")

            # Mark hardware ready once harness connection is up; programmer may be optional for some tests
            self.hardware_ready = True

            # Try to verify programmer; if it fails, warn but do not fail overall HIL setup
            if not self.programmer.verify_connection():
                self.logger.error("Failed to establish programmer connection")
                self.logger.warning("Proceeding without programmer; scenarios that require programming will be skipped or fail later")

            self.logger.info("HIL hardware setup completed successfully (hardware interface ready)")
            return True

        except Exception as e:
            self.logger.error(f"Hardware setup failed: {e}")
            return False

    def program_firmware(self, firmware_path: str) -> bool:
        """Upload firmware to ATmega32A via Arduino as ISP"""
        try:
            if not self.hardware_ready:
                self.logger.error("Hardware not ready for firmware programming")
                return False

            self.logger.info(f"Programming firmware: {firmware_path}")

            if not Path(firmware_path).exists():
                self.logger.error(f"Firmware file not found: {firmware_path}")
                return False

            # Program using Arduino as ISP
            success = self.programmer.program_firmware(firmware_path)

            if success:
                self.logger.info("Firmware programming completed successfully")
                # Wait for target to boot
                time.sleep(2)
                # Restore Arduino to Test Harness firmware so PING/INFO are available
                try:
                    self.upload_test_harness()
                except Exception as _e:
                    self.logger.warning(f"Failed to restore Test Harness after programming: {_e}")
                return True
            else:
                self.logger.error("Firmware programming failed")
                return False

        except Exception as e:
            self.logger.error(f"Firmware programming error: {e}")
            return False


    def upload_test_harness(self) -> bool:
        """Upload the Arduino Test Harness firmware so PING/INFO work after programming.
        This ensures that after using the Arduino as ISP (which temporarily runs the ISP sketch),
        the Arduino is restored to the Test Harness firmware used by Behave steps.
        """
        try:
            from subprocess import run
            harness_dir = Path(__file__).parent / 'arduino_harness'
            if not harness_dir.exists():
                self.logger.error(f"Arduino Test Harness dir not found: {harness_dir}")
                return False
            self.logger.info("Uploading Arduino Test Harness firmware...")
            result = run(['pio', 'run', '-d', str(harness_dir), '-e', 'arduino_test_harness', '-t', 'upload'],
                         capture_output=True, text=True, timeout=180)
            if result.returncode != 0:
                self.logger.error(f"Test Harness upload failed: {result.stderr}")
                return False
            # Give Arduino time to reboot into harness
            time.sleep(2.0)
            # Re-verify serial connection for harness
            if self.hardware_interface:
                _ = self.hardware_interface.verify_connection()
            self.logger.info("Arduino Test Harness upload completed")
            return True
        except Exception as e:
            self.logger.error(f"Test Harness upload error: {e}")
            return False

    def verify_firmware_version(self) -> Optional[str]:
        """Verify firmware version and build information"""
        try:
            if not self.hardware_interface:
                return None

            response = self.hardware_interface.send_command("VERSION")
            if response:
                self.logger.info(f"Firmware version: {response}")
                return response
            else:
                self.logger.warning("Could not retrieve firmware version")
                return None

        except Exception as e:
            self.logger.error(f"Version verification error: {e}")
            return None

    def run_acceptance_scenario(self, scenario_context: Any) -> bool:
        """Execute Behave scenario with HIL hardware validation"""
        try:
            self.logger.info(f"Running HIL scenario: {scenario_context.scenario.name}")

            if not self.hardware_ready:
                self.logger.error("Hardware not ready for scenario execution")
                return False

            # Scenario execution is handled by step definitions
            # This method provides framework support
            return True

        except Exception as e:
            self.logger.error(f"Scenario execution error: {e}")
            return False

    def cleanup_hardware(self) -> None:
        """Safe hardware cleanup and disconnect"""
        try:
            self.logger.info("Cleaning up HIL hardware connections...")

            if self.hardware_interface:
                self.hardware_interface.cleanup()

            if self.programmer:
                self.programmer.cleanup()

            self.hardware_ready = False
            self.logger.info("HIL hardware cleanup completed")

        except Exception as e:
            self.logger.error(f"Hardware cleanup error: {e}")

    def get_config(self) -> Dict[str, Any]:
        """Get HIL configuration for test scenarios"""
        return self.config.copy()


def main():
    """CLI entry point for HIL controller setup and testing"""
    import argparse

    parser = argparse.ArgumentParser(description="HIL Controller CLI")
    parser.add_argument('--setup', action='store_true', help='Setup HIL hardware')
    parser.add_argument('--cleanup', action='store_true', help='Cleanup HIL hardware')
    parser.add_argument('--config', default='hil_config.yaml', help='Config file path')

    args = parser.parse_args()

    controller = HILController(args.config)

    if args.setup:
        success = controller.setup_hardware()
        sys.exit(0 if success else 1)
    elif args.cleanup:
        controller.cleanup_hardware()
        sys.exit(0)
    else:
        print("HIL Controller - Use --setup or --cleanup")
        sys.exit(1)


if __name__ == '__main__':
    main()
