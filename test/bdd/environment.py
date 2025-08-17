# Behave environment configuration
# Sets a runtime profile: 'simulavr' (default) or 'hil'

def before_all(context):
    userdata = getattr(context.config, "userdata", {}) or {}
    context.profile = userdata.get("profile", "simulavr").lower()
    if context.profile not in ("simulavr", "hil"):
        context.profile = "simulavr"

    # Initialize shared resources (populated later by emulator runner)
    context.shared = {}

    # Stable serial symlink path that emulator runner will create, e.g. /dev/pts/N -> /tmp/tty-msio
    if context.profile == "simulavr":
        context.serial_port = "/tmp/tty-msio"
    else:
        # In HIL, the steps should set this to the detected hardware port
        context.serial_port = None


def after_all(context):
    # Cleanup resources if we add any in the future (e.g., stop emulator)
    pass
