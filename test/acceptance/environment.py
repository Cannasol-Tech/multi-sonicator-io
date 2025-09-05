# Behave environment configuration
# Sets a runtime profile: 'hil' (Hardware-in-the-Loop only)

import os
import sys
import importlib.util
import importlib

# Add project root to Python path for proper imports
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

# Convenience: unify HIL logger on context
class _NullLogger:
    def hardware_event(self, *_args, **_kwargs):
        pass
    def measurement(self, *_args, **_kwargs):
        pass

# Behave hooks for HIL automation
def before_all(context):
    """
    Set up HIL environment, auto-detect Arduino port, verify/load ISP sketch, etc.
    """
    context.hil_config = None  # Load from hil_config.yaml
    context.arduino_port = None  # Auto-detect
    context.isp_sketch_loaded = False
    # TODO: Implement auto-detection and setup logic
    print("[HIL] Initializing hardware-in-the-loop environment...")

def before_feature(context, feature):
    """
    Tag scenarios per feature, setup feature-specific HIL state.
    """
    print(f"[HIL] Starting feature: {feature.name}")

def before_scenario(context, scenario):
    """
    Map scenario to PRD, skip if unimplemented, setup scenario state.
    """
    print(f"[HIL] Starting scenario: {scenario.name}")

def after_scenario(context, scenario):
    """
    Teardown scenario state, collect logs, etc.
    """
    print(f"[HIL] Finished scenario: {scenario.name}")


def import_hil_modules():
    """Import HIL modules as a package to support intra-package relative imports"""
    # Ensure project root on sys.path (already added above)
    # Import using package path so relative imports in modules work
    hw_mod = importlib.import_module("test.acceptance.hil_framework.hardware_interface")
    ctrl_mod = importlib.import_module("test.acceptance.hil_framework.hil_controller")
    return hw_mod.HardwareInterface, ctrl_mod.HILController

def before_all(context):

    import yaml
    config_path = os.path.join(os.path.dirname(__file__), 'hil_framework', 'hil_config.yaml')
    with open(config_path, 'r') as f:
        hil_config = yaml.safe_load(f)
    context.config = hil_config

    userdata = getattr(context.config, "userdata", {}) or {}
    context.profile = hil_config.get('behave', {}).get('profile', 'hil').lower()
    if context.profile not in ("hil", "hil"):
        context.profile = "hil"

    context.shared = {}

    if context.profile == "hil":
        context.serial_port = "/tmp/tty-msio"
    else:
        try:
            _HardwareInterface, HILController = import_hil_modules()
            context.hil_controller = HILController(config_file=config_path)
            context.hil_logger = getattr(context.hil_controller, 'logger', _NullLogger())
            if context.hil_controller.setup_hardware():
                context.hardware_ready = True
                context.hardware_interface = context.hil_controller.hardware_interface
                context.serial_port = context.hardware_interface.serial_port if context.hardware_interface else None
                print("✅ HIL framework initialized successfully")
            else:
                context.hardware_ready = False
                context.hardware_interface = None
                context.serial_port = None
                print("❌ HIL hardware connection failed")
        except Exception as e:
            print(f"❌ HIL framework error: {e}")
            import traceback
            traceback.print_exc()
            context.hardware_ready = False
            context.serial_port = None
            context.hardware_interface = None


def after_all(context):
    # Cleanup resources if we add any in the future (e.g., stop emulator)
    if hasattr(context, 'hil_controller') and context.hil_controller:
        context.hil_controller.cleanup_hardware()


def before_scenario(context, scenario):
    # Enforce skipping of @pending scenarios regardless of CLI tag filters
    if "pending" in getattr(scenario, "effective_tags", set()):
        scenario.skip("pending: scenario not implemented yet")

    # HIL-specific scenario setup
    if context.profile == "hil" and hasattr(context, 'hil_controller'):
        if 'hil' in scenario.tags:
            # Program test firmware for HIL scenarios
            if context.hardware_ready:
                # Default test firmware - prefer PlatformIO build if available
                # Order: .pio/build/atmega32a/firmware.hex -> test_firmware.hex
                try:
                    import os
                    pio_hex = os.path.join(".pio", "build", "atmega32a", "firmware.hex")
                    candidates = [pio_hex, "test_firmware.hex"]
                    firmware_path = next((p for p in candidates if os.path.exists(p)), None)
                    if firmware_path:
                        context.hil_controller.program_firmware(firmware_path)
                    else:
                        context.hil_controller.logger.info(
                            "No firmware artifact found (.pio/.../firmware.hex or test_firmware.hex); skipping auto-program for scenario"
                        )
                except Exception as _e:
                    # Do not hard-fail the scenario at setup time; steps that require programmed DUT will fail
                    context.hil_controller.logger.warning(
                        f"Auto-program encountered an error and was skipped: {_e}"
                    )
                print(f"🔧 HIL scenario setup: {scenario.name}")
            else:
                scenario.skip("HIL hardware not available")


def after_scenario(context, scenario):
    """Cleanup after each scenario"""
    if context.profile == "hil" and hasattr(context, 'hil_controller'):
        if context.hardware_ready and 'hil' in scenario.tags:
            # Scenario-specific cleanup if needed
            pass


def after_scenario(context, scenario):
    # Ensure any Modbus client opened by steps is closed to release serial lock
    m = getattr(context, "modbus", None)
    try:
        if m is not None:
            m.close()
    finally:
        if hasattr(context, "modbus"):
            delattr(context, "modbus")
