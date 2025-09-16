#!/usr/bin/env python3
"""
Steps that iterate over all connected channels from hardware-config.yaml
to validate control and status signals via HIL.
"""
from __future__ import annotations
from behave import then
from pathlib import Path
import yaml
import time


CFG_PATH = Path(__file__).resolve().parents[3] / "config" / "hardware-config.yaml"


def _load_cfg(context):
    cfg = getattr(context, 'hw_cfg', None)
    if cfg is None:
        with open(CFG_PATH, 'r', encoding='utf-8') as f:
            cfg = yaml.safe_load(f) or {}
    return cfg


def _connected_channels(cfg):
    chans = []
    for ch in (cfg.get('sonicators', {}) or {}).get('channels', []) or []:
        try:
            if str(ch.get('status', '')).lower() == 'connected':
                chans.append(int(ch.get('id')))
        except Exception:
            continue
    return sorted(chans)


def _require_hw(context):
    assert hasattr(context, 'hardware_interface') and context.hardware_interface, 
        "HIL hardware interface not available"


def _son_reg_start(ch: int) -> int:
    base = 0x0100
    stride = 0x0020
    offset = 0x00
    return base + (ch - 1) * stride + offset


def _son_reg_reset(ch: int) -> int:
    base = 0x0100
    stride = 0x0020
    offset = 0x02
    return base + (ch - 1) * stride + offset


def _son_reg_amp_sp(ch: int) -> int:
    base = 0x0100
    stride = 0x0020
    offset = 0x01
    return base + (ch - 1) * stride + offset


def _son_reg_status_flags(ch: int) -> int:
    base = 0x0100
    stride = 0x0020
    offset = 0x12
    return base + (ch - 1) * stride + offset


@then('START pins for all connected channels shall toggle via MODBUS')
def step_all_channels_start_toggle(context):
    _require_hw(context)
    cfg = _load_cfg(context)
    chans = _connected_channels(cfg)
    assert chans, "No connected channels in hardware-config.yaml"
    for ch in chans:
        addr = _son_reg_start(ch)
        # Start = 1
        ok = context.hardware_interface.modbus_write_register(addr, 1)
        assert ok, f"MODBUS write failed for start ch{ch} addr=0x{addr:04X}"
        resp = context.hardware_interface.send_command(f"PIN_READ START_{ch}")
        assert resp and resp.strip().endswith('1'), f"START_{ch} expected HIGH, got: {resp!r}"
        # Stop = 0
        ok = context.hardware_interface.modbus_write_register(addr, 0)
        assert ok, f"MODBUS write failed for stop ch{ch} addr=0x{addr:04X}"
        resp = context.hardware_interface.send_command(f"PIN_READ START_{ch}")
        assert resp and resp.strip().endswith('0'), f"START_{ch} expected LOW, got: {resp!r}"


@then('RESET pins for all connected channels shall pulse via MODBUS reset command')
def step_all_channels_reset_pulse(context):
    _require_hw(context)
    cfg = _load_cfg(context)
    chans = _connected_channels(cfg)
    assert chans, "No connected channels in hardware-config.yaml"
    for ch in chans:
        addr = _son_reg_reset(ch)
        ok = context.hardware_interface.modbus_write_register(addr, 1)
        assert ok, f"MODBUS write failed for reset ch{ch} addr=0x{addr:04X}"
        # Allow brief time for line to assert
        time.sleep(0.02)
        resp = context.hardware_interface.send_command(f"PIN_READ RESET_{ch}")
        assert resp and resp.strip().endswith('1'), f"RESET_{ch} expected HIGH pulse, got: {resp!r}"
        # Optional: allow pulse to drop
        time.sleep(0.05)


@then('AMPLITUDE setpoints for all connected channels shall be writable and readable')
def step_all_channels_amplitude_rw(context):
    _require_hw(context)
    cfg = _load_cfg(context)
    chans = _connected_channels(cfg)
    assert chans, "No connected channels in hardware-config.yaml"
    for ch in chans:
        addr = _son_reg_amp_sp(ch)
        sp = 50  # nominal 50%
        ok = context.hardware_interface.modbus_write_register(addr, sp)
        assert ok, f"MODBUS write failed for amplitude ch{ch} addr=0x{addr:04X}"
        val = context.hardware_interface.modbus_read_register(addr)
        assert val == sp, f"Amplitude SP readback mismatch ch{ch}: got {val}, expected {sp}"


@then('OVERLOAD and FREQ_LOCK signals shall reflect for all connected channels')
def step_all_channels_status_reflect(context):
    _require_hw(context)
    cfg = _load_cfg(context)
    chans = _connected_channels(cfg)
    assert chans, "No connected channels in hardware-config.yaml"
    for ch in chans:
        # OVERLOAD
        r = context.hardware_interface.send_command(f"SIM_OVERLOAD {ch} ON")
        assert r and 'OK' in r, f"SIM_OVERLOAD ON failed for ch{ch}: {r!r}"
        resp = context.hardware_interface.send_command(f"PIN_READ OVERLOAD_{ch}")
        assert resp and resp.strip().endswith('1'), f"OVERLOAD_{ch} expected HIGH, got: {resp!r}"
        r = context.hardware_interface.send_command(f"SIM_OVERLOAD {ch} OFF")
        assert r and 'OK' in r, f"SIM_OVERLOAD OFF failed for ch{ch}: {r!r}"
        resp = context.hardware_interface.send_command(f"PIN_READ OVERLOAD_{ch}")
        assert resp and resp.strip().endswith('0'), f"OVERLOAD_{ch} expected LOW, got: {resp!r}"
        # FREQ_LOCK
        r = context.hardware_interface.send_command(f"SIM_FREQ_LOCK {ch} ON")
        assert r and 'OK' in r, f"SIM_FREQ_LOCK ON failed for ch{ch}: {r!r}"
        resp = context.hardware_interface.send_command(f"PIN_READ FREQ_LOCK_{ch}")
        assert resp and resp.strip().endswith('1'), f"FREQ_LOCK_{ch} expected HIGH, got: {resp!r}"
        r = context.hardware_interface.send_command(f"SIM_FREQ_LOCK {ch} OFF")
        assert r and 'OK' in r, f"SIM_FREQ_LOCK OFF failed for ch{ch}: {r!r}"
        resp = context.hardware_interface.send_command(f"PIN_READ FREQ_LOCK_{ch}")
        assert resp and resp.strip().endswith('0'), f"FREQ_LOCK_{ch} expected LOW, got: {resp!r}"


@then('MODBUS status flags shall reflect OVERLOAD and FREQ_LOCK for all connected channels')
def step_all_channels_status_flags(context):
    _require_hw(context)
    cfg = _load_cfg(context)
    chans = _connected_channels(cfg)
    assert chans, "No connected channels in hardware-config.yaml"
    for ch in chans:
        # Turn both on
        assert 'OK' in context.hardware_interface.send_command(f"SIM_OVERLOAD {ch} ON")
        assert 'OK' in context.hardware_interface.send_command(f"SIM_FREQ_LOCK {ch} ON")
        addr = _son_reg_status_flags(ch)
        flags = context.hardware_interface.modbus_read_register(addr)
        assert flags is not None, f"Failed reading status flags ch{ch} addr=0x{addr:04X}"
        # 0x0002 OVERLOAD, 0x0004 FREQ_LOCK
        assert (flags & 0x0002) != 0, f"OVERLOAD flag not set for ch{ch}"
        assert (flags & 0x0004) != 0, f"FREQ_LOCK flag not set for ch{ch}"
        # Turn off and verify cleared
        assert 'OK' in context.hardware_interface.send_command(f"SIM_OVERLOAD {ch} OFF")
        assert 'OK' in context.hardware_interface.send_command(f"SIM_FREQ_LOCK {ch} OFF")
        flags2 = context.hardware_interface.modbus_read_register(addr)
        assert (flags2 & 0x0002) == 0, f"OVERLOAD flag not cleared for ch{ch}"
        assert (flags2 & 0x0004) == 0, f"FREQ_LOCK flag not cleared for ch{ch}"


@then('POWER_SENSE ADC ranges shall be sane for all connected channels')
def step_all_channels_power_sense_range(context):
    _require_hw(context)
    cfg = _load_cfg(context)
    chans = _connected_channels(cfg)
    assert chans, "No connected channels in hardware-config.yaml"
    for ch in chans:
        resp = context.hardware_interface.send_command(f"ADC_READ POWER_SENSE_{ch}")
        assert resp and resp.startswith('ADC '), f"ADC_READ failed for POWER_SENSE_{ch}: {resp!r}"
        parts = resp.strip().split()
        assert len(parts) >= 4, f"Malformed ADC response: {resp!r}"
        val = float(parts[2])
        unit = parts[3]
        # Acceptable envelope for sanity (0..5V or 0..5000 mV)
        if unit.lower() == 'mv':
            assert 0.0 <= val <= 5000.0, f"POWER_SENSE_{ch} {val} mV out of range"
        elif unit.lower() == 'v':
            assert 0.0 <= val <= 5.0, f"POWER_SENSE_{ch} {val} V out of range"
        else:
            assert False, f"Unexpected ADC unit {unit} for POWER_SENSE_{ch}"
