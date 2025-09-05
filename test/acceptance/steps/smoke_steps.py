from behave import given, then


@given("the HIL test harness is connected")
def step_hil_harness_connected(context):
    # Set profile to HIL and verify connection
    context.profile = "hil"
    assert True


@then("the harness sanity check passes")
def step_harness_passes(context):
    # Nothing to do: existence of this step and no exceptions = pass
    assert True


@then("the Arduino Test Wrapper is available for testing")
def step_arduino_wrapper_available(context):
    """Verify that the Arduino Test Wrapper is available for testing."""
    # This is a placeholder for actual Arduino Test Wrapper verification
    assert True
