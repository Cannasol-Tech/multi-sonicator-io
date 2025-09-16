#!/usr/bin/env python3
"""
BDD steps for validating DB9 mapping consistency against config/hardware-config.yaml
"""
from __future__ import annotations
from behave import given, then
from pathlib import Path
import yaml


CFG_PATH = Path(__file__).resolve().parents[3] / "config" / "hardware-config.yaml"


@given('the hardware configuration is loaded')
def step_load_hardware_config(context):
    with open(CFG_PATH, 'r', encoding='utf-8') as f:
        context.hw_cfg = yaml.safe_load(f) or {}
    assert 'dut' in context.hw_cfg and 'pins' in context.hw_cfg['dut'], "DUT pin matrix missing in config"
    assert 'db9_connectors' in context.hw_cfg, "DB9 connectors missing in config"


def _required_for_channel(ch: int):
    return {
        1: f"OVERLOAD_{ch}",
        2: f"RESET_{ch}",
        3: f"FREQ_LOCK_{ch}",
        4: f"FREQ_DIV10_{ch}",
        5: f"POWER_SENSE_{ch}",
        7: f"START_{ch}",
        8: "AMPLITUDE_ALL",
    }


@then('DB9 connectors for sonicators 1..4 shall map required signals per channel')
def step_verify_db9_channels(context):
    db9 = context.hw_cfg.get('db9_connectors', {}) or {}
    for ch in (1, 2, 3, 4):
        name = f"DB9-{ch}"
        assert name in db9, f"Missing connector {name}"
        pins = (db9[name].get('pins') or {})
        # Normalize keys to int
        pins_norm = {int(k): v for k, v in pins.items()}
        required = _required_for_channel(ch)
        for pin_num, sig in required.items():
            entry = pins_norm.get(pin_num)
            assert entry is not None, f"{name}: missing pin {pin_num} ({sig})"
            actual_sig = str(entry.get('signal', '')).strip()
            assert actual_sig == sig, f"{name}: pin {pin_num} expected '{sig}' got '{actual_sig or '<empty>'}'"
            dut_pin = str(entry.get('dut_pin', '')).strip()
            assert dut_pin, f"{name}: pin {pin_num} has no dut_pin reference"


@then('a MODBUS connector (DB9-0 or DB9-5) shall be present with RX/TX pins defined')
def step_verify_modbus_connectors(context):
    db9 = context.hw_cfg.get('db9_connectors', {}) or {}
    present = []
    for name in ('DB9-0', 'DB9-5'):
        if name in db9:
            present.append(name)
            pins = {int(k): v for k, v in (db9[name].get('pins') or {}).items()}
            rx = pins.get(8)
            tx = pins.get(9)
            assert rx and str(rx.get('signal', '')).strip() == 'UART_RXD', f"{name}: pin 8 must be UART_RXD"
            assert tx and str(tx.get('signal', '')).strip() == 'UART_TXD', f"{name}: pin 9 must be UART_TXD"
    assert present, "Neither DB9-0 nor DB9-5 is present in config"


@then('all referenced DUT pins shall exist in the DUT pin matrix')
def step_all_dut_pins_exist(context):
    dut_pins = context.hw_cfg['dut']['pins'] or {}
    db9 = context.hw_cfg['db9_connectors'] or {}
    for name, conn in db9.items():
        pins = conn.get('pins') or {}
        for pin_num, entry in pins.items():
            ref = str(entry.get('dut_pin', '')).strip()
            assert ref in dut_pins, f"{name} pin {pin_num}: dut_pin '{ref}' not defined in dut.pins"

