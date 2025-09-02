# Behave environment configuration
# Sets a runtime profile: 'simulavr' (default) or 'hil'

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
            from test.acceptance.hil_framework.hil_controller import HILController
            from test.acceptance.hil_framework.hardware_interface import HardwareInterface
            
            context.hil_controller = HILController()
            context.hardware_interface = HardwareInterface()
            
            # Setup hardware connections
            if context.hil_controller.setup_hardware():
                context.hardware_ready = True
                context.serial_port = context.hil_controller.config['hardware']['target_serial_port']
                print("✅ HIL framework initialized successfully")
            else:
                context.hardware_ready = False
                context.serial_port = None
                print("❌ HIL framework initialization failed")
                
        except ImportError as e:
            print(f"❌ HIL framework not available: {e}")
            context.hardware_ready = False
            context.serial_port = None
        except Exception as e:
            print(f"❌ HIL framework error: {e}")
            context.hardware_ready = False
            context.serial_port = None


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
