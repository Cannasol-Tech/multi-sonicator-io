from behave import given, then


@given("the emulation profile is simulavr")
def step_profile_is_simulavr(context):
    # environment.py sets default to simulavr; verify explicitly
    assert getattr(context, "profile", None) == "simulavr", (
        f"Expected profile 'simulavr', got '{getattr(context, 'profile', None)}'"
    )


@then("the harness sanity check passes")
def step_harness_passes(context):
    # Nothing to do: existence of this step and no exceptions = pass
    assert True
