# Behave environment configuration
# Sets a runtime profile: 'simulavr' (default) or 'hil'

def before_all(context):
    userdata = getattr(context.config, "userdata", {}) or {}
    context.profile = userdata.get("profile", "simulavr").lower()
    if context.profile not in ("simulavr", "hil"):
        context.profile = "simulavr"

    # Place to initialize shared resources later (serial ports, PTYs, etc.)
    context.shared = {}


def after_all(context):
    # Cleanup resources if we add any in the future
    pass
