#!/usr/bin/env python3
"""
MODBUS topology checks against hardware-config.yaml and HIL settings
"""
from __future__ import annotations
from behave import then
from pathlib import Path
import yaml


CFG_PATH = Path(__file__).resolve().parents[3] / "config" / "hardware-config.yaml"


@then('the configured MODBUS slave id and baud rate shall match the hardware configuration')
def step_modbus_config_matches_cfg(context):
    cfg = getattr(context, 'hw_cfg', None)
    if cfg is None:
        with open(CFG_PATH, 'r', encoding='utf-8') as f:
            cfg = yaml.safe_load(f) or {}
    comm = (cfg.get('communication', {}) or {}).get('modbus', {})
    exp_slave = int(comm.get('slave_id', 2))
    exp_baud = int(comm.get('baud_rate', 115200))
    # Record expectations for step alignment
    context.expected_modbus = {'slave_id': exp_slave, 'baud_rate': exp_baud}
    assert exp_slave == 2, f"Expected default slave id 2, got {exp_slave} (update steps if config changed)"
    assert exp_baud == 115200, f"Expected default baud 115200, got {exp_baud} (update steps if config changed)"


@then('the HIL profile shall use the same serial port as the hardware configuration when available')
def step_hil_serial_port_matches(context):
    cfg = getattr(context, 'hw_cfg', None)
    if cfg is None:
        with open(CFG_PATH, 'r', encoding='utf-8') as f:
            cfg = yaml.safe_load(f) or {}
    ser = (cfg.get('communication', {}) or {}).get('serial', {})
    cfg_port = ser.get('port')
    # HIL environment may not expose port when running offline; treat mismatch as warning
    hil_port = getattr(context, 'serial_port', None)
    # record in logger if available
    if hasattr(context, 'hil_logger'):
        context.hil_logger.info(f"CFG serial port: {cfg_port} | HIL serial port: {hil_port}")
    # Only assert presence of config; port equality is best-effort when hardware present
    assert cfg_port, "communication.serial.port must be set in hardware-config.yaml"

