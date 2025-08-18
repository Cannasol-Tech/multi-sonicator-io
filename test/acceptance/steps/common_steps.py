"""
Common Behave steps with simulavr Modbus support.
Unimplemented functionality in other profiles is skipped.
"""

from behave import given, when, then
import os
import time

try:
    from pymodbus.client import ModbusSerialClient
except Exception:  # pragma: no cover - runtime env inside Docker
    ModbusSerialClient = None


def _profile(context) -> str:
    # Prefer behave userdata: `-D profile=simulavr`
    return getattr(context.config.userdata, "get", lambda k, d=None: d)("profile", None) or getattr(context, "profile", None)


def _ensure_serial(context) -> str:
    """Return serial port path, waiting briefly for emulator to create it.
    Skips the scenario if unavailable.
    """
    port = getattr(context, "serial_port", None)
    if not port:
        context.scenario.skip("serial_port not set in context")
        return None
    # Wait up to 2s for symlink to appear
    deadline = time.time() + 2.0
    while time.time() < deadline and not os.path.exists(port):
        time.sleep(0.05)
    if not os.path.exists(port):
        context.scenario.skip(f"serial device not present: {port}")
        return None
    return port


def _get_client(context) -> ModbusSerialClient:
    if _profile(context) != "simulavr":
        context.scenario.skip("pending: Modbus steps only implemented for simulavr profile")
        return None
    if ModbusSerialClient is None:
        context.scenario.skip("pending: pymodbus not available")
        return None
    port = _ensure_serial(context)
    if not port:
        return None
    shared = getattr(context, "shared", {})
    client = shared.get("modbus_client")
    if client and getattr(client, "connected", False):
        return client
    client = ModbusSerialClient(
        method="rtu",
        port=port,
        baudrate=19200,
        parity="N",
        stopbits=1,
        bytesize=8,
        timeout=0.2,
    )
    if not client.connect():
        context.scenario.skip(f"unable to connect Modbus client on {port}")
        return None
    shared["modbus_client"] = client
    context.shared = shared
    return client


def _hr_index(addr: int) -> int:
    """Convert 4xxxx notation to zero-based holding register index."""
    if addr >= 40001:
        return addr - 40001
    # Fallback: treat as 1-based plain address
    return max(0, addr - 1)


def _normalize_assert_state(word: str) -> int:
    """Map words like 'asserted/deasserted', 'on/off', 'enabled/disabled' to 1/0."""
    w = str(word).strip().lower()
    if w in {"asserted", "on", "enabled", "true", "high", "1"}:
        return 1
    if w in {"deasserted", "off", "disabled", "false", "low", "0"}:
        return 0
    # Fall back to numeric parse
    try:
        return 1 if int(w) != 0 else 0
    except Exception:
        return 0


@given("the system is initialized")
def step_system_initialized(context):
    # Minimal: for simulavr, assume emulator is launched externally by harness.
    if _profile(context) == "simulavr":
        context.system_initialized = True
        return
    # Mark scenario as pending for non-simulavr until implemented
    context.scenario.skip("pending: initialization for this profile not implemented")


@when("I write holding register {address:d} with value {value:d}")
def step_write_register(context, address, value):
    client = _get_client(context)
    if not client:
        return
    idx = _hr_index(address)
    rr = client.write_register(address=idx, value=value, unit=1)
    if getattr(rr, "isError", lambda: False)():
        raise AssertionError(f"Modbus write failed: addr={address} idx={idx} value={value} resp={rr}")


@then("holding register {address:d} should equal {value:d} within {ms:d} ms")
def step_expect_register_value(context, address, value, ms):
    client = _get_client(context)
    if not client:
        return
    idx = _hr_index(address)
    deadline = time.time() + (ms / 1000.0)
    last = None
    while time.time() < deadline:
        rr = client.read_holding_registers(address=idx, count=1, unit=1)
        if not getattr(rr, "isError", lambda: True)() and getattr(rr, "registers", None):
            last = rr.registers[0]
            if last == value:
                return
        time.sleep(0.05)
    raise AssertionError(f"Register 4{idx+1:05d} expected {value} within {ms} ms, last={last}")


@when("I stimulate {signal} to {state}")
def step_stimulate_signal(context, signal, state):
    # TODO: Drive virtual input in simulavr model.
    context.scenario.skip("pending: signal stimulation via simulavr not implemented yet")


@when("I set input {signal} for unit {unit:d} to {state_word}")
def step_set_input_state_unit(context, signal, unit, state_word):
    value = _normalize_assert_state(state_word)
    context.scenario.skip(f"pending: input stimulation for unit={unit} not implemented yet")


@when("I drive input {signal} for unit {unit:d} {level_word}")
def step_drive_input_level_unit(context, signal, unit, level_word):
    value = _normalize_assert_state(level_word)
    context.scenario.skip(f"pending: input drive for unit={unit} not implemented yet")


@then("status flag bit {bit:d} for unit {unit:d} is {state}")
def step_status_flag(context, bit, unit, state):
    # TODO: Inspect simulated status bits.
    context.scenario.skip("pending: status flag check via simulavr not implemented yet")


# Natural-language aliases for clearer test intent
@when("I simulate starting sonicator {unit:d}")
def step_simulate_starting_sonicator(context, unit):
    """Alias for asserting the 'running' input for a unit."""
    # Reuse the unit-scoped input step for single source of truth
    step_set_input_state_unit(context, signal="running", unit=unit, state_word="asserted")


@when("I simulate stopping sonicator {unit:d}")
def step_simulate_stopping_sonicator(context, unit):
    """Alias for deasserting the 'running' input for a unit."""
    step_set_input_state_unit(context, signal="running", unit=unit, state_word="deasserted")


# Additional undefined steps: mark as pending instead of erroring
@then("the CI drift check passes")
def step_ci_drift_check(context):
    context.scenario.skip("pending: CI drift check step not implemented")


@then("corrupted frames are rejected")
def step_crc_rejection(context):
    context.scenario.skip("pending: CRC error handling step not implemented")


# Executive-facing natural language aliases
@when("I trigger emergency stop")
def step_trigger_estop(context):
    step_set_input_state_unit(context, signal="estop", unit=1, state_word="asserted")


@when("I clear emergency stop")
def step_clear_estop(context):
    step_set_input_state_unit(context, signal="estop", unit=1, state_word="deasserted")


@when("I simulate overload on sonicator {unit:d}")
def step_simulate_overload_unit(context, unit):
    step_set_input_state_unit(context, signal="overload", unit=unit, state_word="asserted")


@when("I simulate frequency lock on sonicator {unit:d}")
def step_simulate_freq_lock_unit(context, unit):
    step_set_input_state_unit(context, signal="frequency_lock", unit=unit, state_word="asserted")


@when("I simulate power reading of {value:d}")
def step_simulate_power_reading(context, value):
    # Preserve current behavior (pending) via generic stimulate hook
    step_stimulate_signal(context, signal="power", state=str(value))


@when("I simulate frequency reading of {value:d}")
def step_simulate_frequency_reading(context, value):
    step_stimulate_signal(context, signal="frequency", state=str(value))
