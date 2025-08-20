# Step definitions for HIL-based acceptance tests
# These are scaffolds and intentionally Pending until the HIL wrapper protocol and MODBUS client are implemented.

import os
from behave import given, when, then
from behave.runner import Context
from typing import Optional


def _pending(msg: str = "Pending step implementation"):
    raise NotImplementedError(msg)


# Only register HIL steps when explicitly enabled to avoid ambiguity with simulavr steps
if os.environ.get("BEHAVE_ENABLE_HIL_STEPS", "0") == "1":
    # Background
    @given("the HIL wrapper is connected and ready")
    def step_hil_ready(context: Context):
        # TODO: detect serial port and issue PING to wrapper
        _pending("Connect to wrapper serial and verify 'OK WRAPPER_READY' and 'OK PONG'")


    @given("the DUT is powered and at safe defaults")
    def step_dut_safe_defaults(context: Context):
        # TODO: ensure DUT reset/safe state prior to scenario
        _pending("Ensure DUT safe defaults via wrapper controls")


# Generic MODBUS write/read actions
    @when("I write {value:d} to holding register {addr:d}")
    @when("I write {cmd:d} to holding register {addr:d}")
    def step_write_holding(context: Context, value: int = None, cmd: int = None, addr: int = 0):
        # TODO: perform MODBUS FC06 write to addr with value/cmd
        _pending("MODBUS write FC06 to holding register")


    @then("within 100 ms the status flag bit0 for unit {unit:d} equals {expected:d}")
    def step_check_status_bit0(context: Context, unit: int, expected: int):
        # TODO: poll 40021–40024 and assert bit0 for unit
        _pending("Poll status flags and assert bit0")


    @then("Active Sonicator Count and Mask reflect the running units")
    def step_check_system_summary(context: Context):
        # TODO: read 40035 and 40036 and assert against started units set in context
        _pending("Assert Active Count/Mask consistency")


# Overload scenarios
    @given("unit {unit:d} overload input is set to {ovl:d}")
    def step_set_overload_input(context: Context, unit: int, ovl: int):
        # TODO: drive overload input via wrapper for given unit
        _pending("Drive overload input via wrapper")


    @then("within 100 ms the overload flag bit1 for unit {unit:d} equals {expected_flag:d}")
    def step_check_overload_flag(context: Context, unit: int, expected_flag: int):
        # TODO: poll 40021–40024 and assert bit1
        _pending("Assert overload flag bit1")


    @then("starting the unit via register {start_addr:d} yields behavior per spec when overload={ovl:d}")
    def step_start_behavior_under_overload(context: Context, start_addr: int, ovl: int):
        # TODO: attempt start and assert allowed/blocked based on overload
        _pending("Assert start behavior under overload condition")


# Amplitude scenarios
    @then("within 100 ms the amplitude output maps to {value:d} percent within tolerance 2%")
    def step_check_amplitude_mapping(context: Context, value: int):
        # TODO: read wrapper ADC and compare to expected mapping
        _pending("Verify amplitude mapping via wrapper ADC")


    @then("the write is rejected or the register value is clamped per spec")
    def step_amplitude_invalid_write_behavior(context: Context):
        # TODO: assert error or clamped value after out-of-range write
        _pending("Invalid amplitude write handling")


# Frequency & lock scenarios
    @given("unit {unit:d} receives a frequency of {hz:d} Hz on the ÷10 input")
    def step_set_frequency_input(context: Context, unit: int, hz: int):
        # TODO: configure wrapper to output frequency
        _pending("Provide ÷10 frequency signal via wrapper")


    @given("unit {unit:d} lock input is set to {lock:d}")
    def step_set_lock_input(context: Context, unit: int, lock: int):
        # TODO: drive lock input via wrapper
        _pending("Drive frequency lock input via wrapper")


    @then("within 100 ms holding register {freq_addr:d} is approximately {hz:d} Hz")
    def step_check_frequency_register(context: Context, freq_addr: int, hz: int):
        # TODO: read freq register and compare within tolerance
        _pending("Assert frequency register value")


    @then("status flag bit2 for unit {unit:d} equals {lock:d}")
    def step_check_lock_flag(context: Context, unit: int, lock: int):
        # TODO: poll 40021–40024 and assert bit2
        _pending("Assert lock flag bit2")
