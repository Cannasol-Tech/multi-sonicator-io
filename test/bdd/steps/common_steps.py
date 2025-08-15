# Common step placeholders for Behave scenarios.
# Implementations will branch on context.profile (simulavr vs hil)

from behave import given, when, then


def _not_impl():
    raise NotImplementedError("Step not yet implemented. Fill in per profile (simulavr/hil)")


@given("the system is initialized")
def step_system_initialized(context):
    _not_impl()


@when("I write holding register {address:d} with value {value:d}")
def step_write_register(context, address, value):
    _not_impl()


@then("holding register {address:d} should equal {value:d} within {ms:d} ms")
def step_expect_register_value(context, address, value, ms):
    _not_impl()


@when("I stimulate {signal} to {state}")
def step_stimulate_signal(context, signal, state):
    _not_impl()


@then("status flag bit {bit:d} for unit {unit:d} is {state}")
def step_status_flag(context, bit, unit, state):
    _not_impl()
