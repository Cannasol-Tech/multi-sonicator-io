"""
Simulation step definitions for pysimulavr.

Assumes the emulator is running and has created the UART PTY symlink at /tmp/tty-msio
via scripts/emulation/cli.py. Uses a minimal MODBUS RTU helper to talk to the DUT.
"""

from behave import given, when, then
from behave.runner import Context
from pathlib import Path
import time

try:
    # Preferred when Behave sets package context
    from .lib.modbus_rtu import ModbusRTU, ModbusConfig  # type: ignore
except Exception:
    # Fallback when executed without package context
    from steps.lib.modbus_rtu import ModbusRTU, ModbusConfig  # type: ignore


def _ensure_modbus(context: Context) -> ModbusRTU:
    if getattr(context, "modbus", None) is None:
        port = getattr(context, "serial_port", None) or "/tmp/tty-msio"
        context.modbus = ModbusRTU(ModbusConfig(port=port))
    return context.modbus  # type: ignore


@given("the emulator is running with UART PTY available")
def step_emulator_running(context: Context):
    p = Path("/tmp/tty-msio")
    deadline = time.time() + 10.0
    while time.time() < deadline and not p.exists():
        time.sleep(0.1)
    if not p.exists():
        raise AssertionError("/tmp/tty-msio not found. Start emulator: scripts/emulation/cli.py")
    # Initialize tracking of expected running units
    context.expected_running = set()


# Compatibility with HIL phrasing used in feature Backgrounds
@given("the HIL wrapper is connected and ready")
def step_wrapper_ready_alias(context: Context):
    # In simulavr profile, treat this as ensuring the emulator PTY exists
    if getattr(context, "profile", "simulavr") == "simulavr":
        return step_emulator_running(context)


@given("the DUT is powered and at safe defaults")
def step_safe_defaults_alias(context: Context):
    if getattr(context, "profile", "simulavr") != "simulavr":
        return
    # Optionally nudge DUT into a known state (no-ops if firmware ignores)
    m = _ensure_modbus(context)
    # Clear starts for all units 1..4 if supported
    for addr in (40005, 40006, 40007, 40008):
        try:
            m.write_holding(addr, 0)
        except Exception:
            pass


# Match the same phrasing as features so we can reuse scenarios
@when("I write {value:d} to holding register {addr:d}")
@when("I write {cmd:d} to holding register {addr:d}")
def step_write_holding_sim(context: Context, value: int = None, cmd: int = None, addr: int = 0):
    if context.profile != "simulavr":
        # Allow fallback to HIL steps if running HIL profile
        return
    m = _ensure_modbus(context)
    val = value if value is not None else cmd
    assert val is not None
    m.write_holding(addr, val)
    # Track expected running set for system summary assertions
    if 40005 <= addr <= 40008:
        unit = addr - 40004  # 40005->1, ... 40008->4
        if val == 1:
            context.expected_running.add(unit)
        else:
            context.expected_running.discard(unit)


@then("within 100 ms the status flag bit0 for unit {unit:d} equals {expected:d}")
def step_check_status_bit0_sim(context: Context, unit: int, expected: int):
    if context.profile != "simulavr":
        return
    m = _ensure_modbus(context)
    # Status flags 40021–40024 map 1:1 to units 1..4
    addr = 40020 + unit
    def ok() -> bool:
        val = m.read_holding(addr)[0]
        return ((val & 0b1) != 0) == (expected == 1)
    if not m.poll_until(ok, timeout_s=0.1, interval_s=0.01):
        raise AssertionError(f"bit0 for unit {unit} did not become {expected} within 100 ms")


@then("Active Sonicator Count and Mask reflect the running units")
def step_check_system_summary(context: Context):
    if context.profile != "simulavr":
        return
    m = _ensure_modbus(context)
    count = m.read_holding(40035)[0]
    mask = m.read_holding(40036)[0]
    expected_set = getattr(context, "expected_running", set())
    expected_count = len(expected_set)
    expected_mask = 0
    for u in expected_set:
        expected_mask |= (1 << (u - 1))
    if count != expected_count or mask != expected_mask:
        raise AssertionError(f"Expected count/mask {expected_count}/{expected_mask:#04x} got {count}/{mask:#04x}")
