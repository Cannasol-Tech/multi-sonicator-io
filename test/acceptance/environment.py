# Behave environment configuration
# Sets a runtime profile: 'simulavr' (default) or 'hil'

import os
import sys
import importlib.util

# Add project root to Python path for proper imports
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

def import_hil_modules():
    """Import HIL modules using direct file imports to avoid path issues"""
    hil_framework_path = os.path.join(os.path.dirname(__file__), "hil_framework")
    
    # Import HardwareInterface
    hardware_interface_file = os.path.join(hil_framework_path, "hardware_interface.py")
    spec = importlib.util.spec_from_file_location("hardware_interface", hardware_interface_file)
    hardware_interface_module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(hardware_interface_module)
    
    # Import HILController
    hil_controller_file = os.path.join(hil_framework_path, "hil_controller.py")
    spec = importlib.util.spec_from_file_location("hil_controller", hil_controller_file)
    hil_controller_module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(hil_controller_module)
    
    return hardware_interface_module.HardwareInterface, hil_controller_module.HILController

def before_all(context):
    userdata = getattr(context.config, "userdata", {}) or {}
    context.profile = userdata.get("profile", "simulavr").lower()
    if context.profile not in ("simulavr", "hil"):
        context.profile = "simulavr"

    # Initialize shared resources (populated later by emulator runner)
    context.shared = {}

    # Profile-specific initialization
    if context.profile == "simulavr":
        # Stable serial symlink path that emulator runner will create, e.g. /dev/pts/N -> /tmp/tty-msio
        context.serial_port = "/tmp/tty-msio"
    else:
        # HIL mode - initialize HIL framework
        try:
            HardwareInterface, HILController = import_hil_modules()
            
            context.hardware_interface = HardwareInterface()
            context.hil_controller = HILController()
            
            # Setup hardware connections
            if context.hardware_interface.verify_connection():
                context.hardware_ready = True
                context.serial_port = context.hardware_interface.serial_port
                print("✅ HIL framework initialized successfully")
            else:
                context.hardware_ready = False
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
                # Default test firmware - can be overridden by scenario
                firmware_path = "test_firmware.hex"
                context.hil_controller.program_firmware(firmware_path)
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
