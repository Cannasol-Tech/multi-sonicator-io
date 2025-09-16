#!/usr/bin/env python3
"""
HIL Pin-Level Step Definitions

Provides generic steps to toggle and read named signals defined in
config/hardware-config.yaml via the Arduino Test Wrapper interface.

The wrapper is expected to support textual commands:
- PIN_READ <SIGNAL>            -> returns: "PIN <SIGNAL> <0|1>"
- PIN_SET <SIGNAL> <0|1>      -> returns: "OK" on success
- ADC_READ <SIGNAL>           -> returns: "ADC <SIGNAL> <value> <unit>" (e.g., mV or V)
- SIM_OVERLOAD <ch> <ON|OFF>  -> returns: "OK"
- SIM_FREQ_LOCK <ch> <ON|OFF> -> returns: "OK"

These steps are written defensively: on unexpected responses, they
fail with clear messages to guide harness implementation.
"""
from __future__ import annotations
from behave import when, then


def _require_hw(context):
    assert hasattr(context, 'hardware_interface') and context.hardware_interface, \
        "HIL hardware interface not available"


@then('pin "{signal}" should be {level:w}')
def step_pin_should_be_level(context, signal, level):
    _require_hw(context)
    resp = context.hardware_interface.send_command(f"PIN_READ {signal}")
    assert resp and resp.startswith("PIN "), f"Unexpected PIN_READ response: {resp!r}"
    parts = resp.strip().split()
    assert len(parts) >= 3, f"Malformed PIN response: {resp!r}"
    name = parts[1]
    val = parts[2]
    assert name == signal, f"PIN_READ returned {name}, expected {signal}"
    want = '1' if str(level).upper() in ("HIGH", "1", "ON") else '0'
    assert val == want, f"Pin {signal} = {val}, expected {want}"


@when('I read ADC "{signal}"')
def step_read_adc(context, signal):
    _require_hw(context)
    resp = context.hardware_interface.send_command(f"ADC_READ {signal}")
    assert resp and resp.startswith("ADC "), f"Unexpected ADC_READ response: {resp!r}"
    parts = resp.strip().split()
    assert len(parts) >= 4, f"Malformed ADC response: {resp!r}"
    # ADC <signal> <value> <unit>
    name = parts[1]
    val = float(parts[2])
    unit = parts[3]
    assert name == signal, f"ADC_READ returned {name}, expected {signal}"
    context.last_adc = {"signal": signal, "value": val, "unit": unit}


@then('ADC "{signal}" should be between {vmin:f} and {vmax:f} {unit:w}')
def step_adc_between(context, signal, vmin, vmax, unit):
    assert hasattr(context, 'last_adc'), "No ADC reading on context"
    rec = context.last_adc
    assert rec["signal"] == signal, f"Last ADC signal is {rec['signal']}, expected {signal}"
    assert rec["unit"].lower() == unit.lower(), f"ADC unit {rec['unit']} != expected {unit}"
    assert vmin <= rec["value"] <= vmax, \
        f"ADC {signal}={rec['value']} {rec['unit']} not in range [{vmin}, {vmax}] {unit}"


@when('I simulate OVERLOAD for channel {ch:d} {state:w}')
def step_sim_overload(context, ch, state):
    _require_hw(context)
    mode = "ON" if str(state).upper() in ("ON", "HIGH", "1") else "OFF"
    resp = context.hardware_interface.send_command(f"SIM_OVERLOAD {ch} {mode}")
    assert resp and "OK" in resp, f"SIM_OVERLOAD failed: {resp!r}"


@when('I simulate FREQ_LOCK for channel {ch:d} {state:w}')
def step_sim_freq_lock(context, ch, state):
    _require_hw(context)
    mode = "ON" if str(state).upper() in ("ON", "HIGH", "1") else "OFF"
    resp = context.hardware_interface.send_command(f"SIM_FREQ_LOCK {ch} {mode}")
    assert resp and "OK" in resp, f"SIM_FREQ_LOCK failed: {resp!r}"
