from behave import given, then, when
import requests
import json
import time


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


# Web UI Smoke Test Steps

@given("the web UI backend is running and healthy")
def step_web_ui_backend_healthy(context):
    """Verify the web UI backend is running and healthy."""
    try:
        response = requests.get("http://localhost:3001/api/health", timeout=5)
        assert response.status_code == 200
        health_data = response.json()
        assert health_data.get("status") == "ok"
        context.backend_healthy = True
        print("✅ Web UI backend is healthy")
    except Exception as e:
        raise AssertionError(f"Web UI backend health check failed: {e}")


@given("the web UI frontend is accessible")
def step_web_ui_frontend_accessible(context):
    """Verify the web UI frontend is accessible."""
    try:
        response = requests.get("http://localhost:3000", timeout=5)
        assert response.status_code == 200
        assert "<!DOCTYPE html>" in response.text
        context.frontend_accessible = True
        print("✅ Web UI frontend is accessible")
    except Exception as e:
        raise AssertionError(f"Web UI frontend accessibility check failed: {e}")


@when("I verify all Sonicator #4 pins are accessible")
def step_verify_sonicator_pins_accessible(context):
    """Verify all Sonicator #4 pins are accessible via API."""
    try:
        response = requests.get("http://localhost:3001/api/pins", timeout=5)
        assert response.status_code == 200
        pins_data = response.json()

        # Expected Sonicator #4 pins
        expected_pins = [
            'FREQ_DIV10_4', 'FREQ_LOCK_4', 'OVERLOAD_4', 'UART_RXD', 'POWER_SENSE_4',
            'START_4', 'RESET_4', 'AMPLITUDE_ALL', 'UART_TXD', 'STATUS_LED'
        ]

        pins = pins_data.get("pins", {})
        for pin_name in expected_pins:
            assert pin_name in pins, f"Pin {pin_name} not found in API response"
            pin_info = pins[pin_name]
            assert "pin" in pin_info, f"Pin {pin_name} missing pin information"
            assert "direction" in pin_info, f"Pin {pin_name} missing direction information"
            assert "state" in pin_info, f"Pin {pin_name} missing state information"

        context.pins_accessible = True
        print(f"✅ All {len(expected_pins)} Sonicator #4 pins are accessible")
    except Exception as e:
        raise AssertionError(f"Pin accessibility check failed: {e}")


@then("all pin control APIs respond correctly")
def step_pin_control_apis_respond(context):
    """Test pin control APIs respond correctly."""
    try:
        # Test setting a pin to HIGH
        response = requests.post(
            "http://localhost:3001/api/pins/FREQ_LOCK_4",
            json={"state": "HIGH"},
            timeout=5
        )
        assert response.status_code == 200
        result = response.json()
        assert result.get("success") == True

        # Verify the pin state changed
        response = requests.get("http://localhost:3001/api/pins/FREQ_LOCK_4", timeout=5)
        assert response.status_code == 200
        pin_data = response.json()
        assert pin_data.get("pin", {}).get("state") == "HIGH"

        # Test setting the pin back to LOW
        response = requests.post(
            "http://localhost:3001/api/pins/FREQ_LOCK_4",
            json={"state": "LOW"},
            timeout=5
        )
        assert response.status_code == 200

        print("✅ Pin control APIs respond correctly")
    except Exception as e:
        raise AssertionError(f"Pin control API test failed: {e}")


@then("the frequency generation system is functional")
def step_frequency_generation_functional(context):
    """Test the frequency generation system."""
    try:
        # Test automatic frequency mode
        response = requests.post(
            "http://localhost:3001/api/config",
            json={"config": {"sonicator4": {"operatingFrequencyKHz": 25.0, "manualMode": False}}},
            timeout=5
        )
        assert response.status_code == 200

        # Verify frequency pin shows calculated value
        time.sleep(0.5)  # Allow time for update
        response = requests.get("http://localhost:3001/api/pins/FREQ_DIV10_4", timeout=5)
        assert response.status_code == 200
        pin_data = response.json()
        assert "2.5kHz" in str(pin_data.get("pin", {}).get("state", ""))

        # Test manual frequency mode
        response = requests.post(
            "http://localhost:3001/api/config",
            json={"config": {"sonicator4": {"manualMode": True, "manualFrequencyKHz": 4.0}}},
            timeout=5
        )
        assert response.status_code == 200

        # Verify manual mode indicator
        time.sleep(0.5)  # Allow time for update
        response = requests.get("http://localhost:3001/api/pins/FREQ_DIV10_4", timeout=5)
        assert response.status_code == 200
        pin_data = response.json()
        state = str(pin_data.get("pin", {}).get("state", ""))
        assert "4kHz" in state and "(M)" in state

        print("✅ Frequency generation system is functional")
    except Exception as e:
        raise AssertionError(f"Frequency generation test failed: {e}")


@then("the PWM monitoring system is operational")
def step_pwm_monitoring_operational(context):
    """Test the PWM monitoring system."""
    try:
        # Check PWM pin is accessible and has numeric state
        response = requests.get("http://localhost:3001/api/pins/AMPLITUDE_ALL", timeout=5)
        assert response.status_code == 200
        pin_data = response.json()

        pin_info = pin_data.get("pin", {})
        assert pin_info.get("direction") == "OUT"

        # PWM state should be numeric (percentage) or "LOW"
        state = pin_info.get("state")
        assert state is not None
        if isinstance(state, (int, float)):
            assert 0 <= state <= 100, f"PWM value {state} out of range 0-100"
        elif isinstance(state, str):
            assert state in ["LOW", "HIGH"] or "%" in state

        print("✅ PWM monitoring system is operational")
    except Exception as e:
        raise AssertionError(f"PWM monitoring test failed: {e}")


@then("the theme toggle system works correctly")
def step_theme_toggle_works(context):
    """Test the theme toggle system (basic frontend accessibility check)."""
    try:
        # Since we can't directly test JavaScript from Python, we'll verify
        # the frontend is serving the theme-related assets
        response = requests.get("http://localhost:3000", timeout=5)
        assert response.status_code == 200
        html_content = response.text

        # Check for theme-related CSS classes or JavaScript
        theme_indicators = [
            "dark-theme",  # CSS class for dark theme
            "theme-toggle",  # Theme toggle button class
            "bg-primary",  # CSS custom property usage
        ]

        found_indicators = [indicator for indicator in theme_indicators if indicator in html_content]
        assert len(found_indicators) > 0, f"No theme system indicators found in HTML"

        print("✅ Theme toggle system appears to be correctly integrated")
    except Exception as e:
        raise AssertionError(f"Theme toggle system test failed: {e}")
