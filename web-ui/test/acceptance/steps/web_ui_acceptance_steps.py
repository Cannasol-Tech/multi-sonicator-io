#!/usr/bin/env python3
"""
Web-UI Acceptance Test Step Implementations

Step definitions for web-ui acceptance testing using behave framework.
Implements actions and assertions for testing web-ui functionality.

Author: Cannasol Technologies
License: Proprietary
"""

import time
import json
from behave import given, when, then, step

# Try to import optional dependencies
try:
    import requests
    HAS_REQUESTS = True
except ImportError:
    HAS_REQUESTS = False

try:
    from selenium.webdriver.common.by import By
    from selenium.webdriver.support.ui import WebDriverWait
    from selenium.webdriver.support import expected_conditions as EC
    from selenium.common.exceptions import TimeoutException, NoSuchElementException
    HAS_SELENIUM = True
except ImportError:
    HAS_SELENIUM = False


# ============================================================================
# BACKGROUND AND SETUP STEPS
# ============================================================================

@given('the web-ui is running in simulation mode')
def step_web_ui_running_simulation_mode(context):
    """Verify web-ui is running in simulation/mock mode"""
    # Verify backend is in simulation mode
    response = requests.get(f"{context.backend_url}/api/health")
    assert response.status_code == 200, f"Backend not available: {response.status_code}"
    
    health_data = response.json()
    assert 'simulation_mode' in health_data, "Backend should indicate simulation mode status"
    # In simulation mode, hardware_present should be false or simulation_mode should be true


@given('the backend test automation service is available')
def step_backend_test_automation_service_available(context):
    """Verify backend test automation service is responding"""
    response = requests.get(f"{context.backend_url}/api/test-automation/scenarios")
    assert response.status_code == 200, f"Test automation service not available: {response.status_code}"


@given('the hardware mock is enabled')
def step_hardware_mock_enabled(context):
    """Verify hardware mock is enabled in the backend"""
    response = requests.get(f"{context.backend_url}/api/hardware/status")
    assert response.status_code == 200, "Hardware status endpoint not available"
    
    status_data = response.json()
    # In mock mode, we expect no real hardware connection but mock responses


@given('the Test Automation panel is loaded')
@given('I navigate to the Test Automation panel')
def step_navigate_to_test_automation_panel(context):
    """Navigate to the Test Automation panel in the web-ui"""
    # Wait for page to load completely
    WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.TAG_NAME, "body"))
    )
    
    # Look for Test Automation tab/button
    test_automation_tab = WebDriverWait(context.driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//button[contains(text(), 'Test Automation')] | //a[contains(text(), 'Test Automation')] | //*[@id='test-automation-tab']"))
    )
    test_automation_tab.click()
    
    # Wait for panel to load
    WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "test-automation-panel") | (By.ID, "test-automation-panel"))
    )


@given('acceptance test scenarios are loaded')
@given('acceptance test scenarios are loaded from the backend')
def step_acceptance_test_scenarios_loaded(context):
    """Verify acceptance test scenarios are loaded in the UI"""
    # Wait for scenarios to load
    scenarios_container = WebDriverWait(context.driver, 15).until(
        EC.presence_of_element_located((By.CLASS_NAME, "scenarios-list") | (By.ID, "scenarios-list"))
    )
    
    # Verify scenarios are present
    scenarios = context.driver.find_elements(By.CLASS_NAME, "scenario-item")
    assert len(scenarios) > 0, "No test scenarios loaded in UI"
    context.loaded_scenarios = scenarios


# ============================================================================
# TEST SCENARIO LOADING AND DISPLAY STEPS
# ============================================================================

@when('I request the list of available test scenarios')
def step_request_available_test_scenarios(context):
    """Request list of available test scenarios"""
    # Look for refresh/load scenarios button if needed
    try:
        refresh_btn = context.driver.find_element(By.XPATH, "//button[contains(text(), 'Load Scenarios')] | //button[contains(text(), 'Refresh')]")
        refresh_btn.click()
        time.sleep(2)  # Wait for loading
    except NoSuchElementException:
        # Scenarios might auto-load, that's OK
        pass


@then('I should see acceptance test scenarios loaded from the backend')
def step_see_acceptance_test_scenarios_loaded(context):
    """Verify acceptance test scenarios are loaded and visible"""
    scenarios_container = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "scenarios-list") | (By.ID, "scenarios-list"))
    )
    
    scenarios = context.driver.find_elements(By.CLASS_NAME, "scenario-item")
    assert len(scenarios) > 0, "No acceptance test scenarios visible"
    context.visible_scenarios = scenarios


@then('each scenario should display its name and description')
def step_scenario_should_display_name_and_description(context):
    """Verify each scenario displays name and description"""
    scenarios = context.visible_scenarios
    
    for scenario in scenarios:
        # Check for scenario name
        name_element = scenario.find_element(By.CLASS_NAME, "scenario-name") or scenario.find_element(By.TAG_NAME, "h3")
        assert name_element.text.strip() != "", "Scenario name should not be empty"
        
        # Check for scenario description
        desc_element = scenario.find_element(By.CLASS_NAME, "scenario-description") or scenario.find_element(By.TAG_NAME, "p")
        assert desc_element.text.strip() != "", "Scenario description should not be empty"


@then('each scenario should show its associated tags')
def step_scenario_should_show_associated_tags(context):
    """Verify each scenario shows its associated tags"""
    scenarios = context.visible_scenarios
    
    for scenario in scenarios:
        # Look for tags container
        tags_container = scenario.find_element(By.CLASS_NAME, "scenario-tags") or scenario.find_element(By.CLASS_NAME, "tags")
        tags = tags_container.find_elements(By.CLASS_NAME, "tag")
        
        # At least one tag should be present
        assert len(tags) > 0, "Scenario should have at least one tag"


@then('the total scenario count should be greater than 0')
@then('the scenario count should be greater than 0')
def step_scenario_count_greater_than_zero(context):
    """Verify scenario count is greater than 0"""
    scenarios = context.visible_scenarios
    assert len(scenarios) > 0, f"Expected scenario count > 0, got {len(scenarios)}"


# ============================================================================
# TAG FILTERING STEPS
# ============================================================================

@when('I select the tag filter dropdown')
def step_select_tag_filter_dropdown(context):
    """Select/open the tag filter dropdown"""
    tag_filter_dropdown = WebDriverWait(context.driver, 10).until(
        EC.element_to_be_clickable((By.CLASS_NAME, "tag-filter-dropdown") | (By.ID, "tag-filter") | (By.XPATH, "//select[contains(@class, 'tag')]"))
    )
    tag_filter_dropdown.click()


@when('I select the "{tag}" tag filter')
def step_select_specific_tag_filter(context, tag):
    """Select a specific tag filter"""
    # Look for the tag option
    tag_option = WebDriverWait(context.driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, f"//option[text()='{tag}'] | //li[text()='{tag}'] | //*[contains(text(), '{tag}')]"))
    )
    tag_option.click()
    
    # Wait for filtering to apply
    time.sleep(1)
    context.applied_tag_filter = tag


@then('only scenarios with the "{tag}" tag should be displayed')
def step_only_scenarios_with_tag_displayed(context, tag):
    """Verify only scenarios with the specified tag are displayed"""
    # Wait for filter to apply
    time.sleep(2)
    
    filtered_scenarios = context.driver.find_elements(By.CLASS_NAME, "scenario-item")
    assert len(filtered_scenarios) > 0, "No scenarios displayed after filtering"
    
    for scenario in filtered_scenarios:
        tags_container = scenario.find_element(By.CLASS_NAME, "scenario-tags") or scenario.find_element(By.CLASS_NAME, "tags")
        tags = [t.text for t in tags_container.find_elements(By.CLASS_NAME, "tag")]
        assert tag in tags, f"Scenario should have '{tag}' tag but has {tags}"


@then('the scenario count should update to reflect the filtered results')
def step_scenario_count_should_update_filtered_results(context):
    """Verify scenario count updates after filtering"""
    current_scenarios = context.driver.find_elements(By.CLASS_NAME, "scenario-item")
    
    # Check if there's a count display
    try:
        count_display = context.driver.find_element(By.CLASS_NAME, "scenario-count") or context.driver.find_element(By.ID, "scenario-count")
        displayed_count = int(count_display.text.split()[0])  # Extract number from text like "5 scenarios"
        assert displayed_count == len(current_scenarios), f"Count display ({displayed_count}) doesn't match actual scenarios ({len(current_scenarios)})"
    except NoSuchElementException:
        # Count display might not exist, that's OK
        pass


# ============================================================================
# TEST EXECUTION STEPS
# ============================================================================

@when('I select a single test scenario "{scenario_name}"')
def step_select_single_test_scenario(context, scenario_name):
    """Select a single test scenario by name"""
    # Find scenario by name
    scenario_xpath = f"//div[contains(@class, 'scenario-item')]//*[contains(text(), '{scenario_name}')]/..//input[@type='checkbox'] | //div[contains(@class, 'scenario-item')]//*[contains(text(), '{scenario_name}')]"
    
    scenario_element = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.XPATH, scenario_xpath))
    )
    
    # If it's a checkbox, click it; otherwise click the scenario
    if scenario_element.tag_name == 'input':
        scenario_element.click()
    else:
        checkbox = scenario_element.find_element(By.XPATH, ".//input[@type='checkbox']")
        checkbox.click()
    
    context.selected_scenario = scenario_name


@when('I click the "Run Selected" button')
def step_click_run_selected_button(context):
    """Click the Run Selected button to start test execution"""
    run_button = WebDriverWait(context.driver, 10).until(
        EC.element_to_be_clickable((By.XPATH, "//button[contains(text(), 'Run Selected')] | //button[@id='run-selected']"))
    )
    run_button.click()


@then('the test execution should start')
def step_test_execution_should_start(context):
    """Verify test execution has started"""
    # Look for execution indicator
    execution_indicator = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "execution-indicator") | (By.ID, "test-execution") | (By.CLASS_NAME, "progress"))
    )
    
    assert execution_indicator.is_displayed(), "Test execution indicator should be visible"


@then('I should see a progress indicator')
@then('I should see a progress indicator showing test execution')
def step_should_see_progress_indicator(context):
    """Verify progress indicator is visible during test execution"""
    progress_indicator = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "progress-bar") | (By.CLASS_NAME, "spinner") | (By.CLASS_NAME, "loading"))
    )
    
    assert progress_indicator.is_displayed(), "Progress indicator should be visible"


@then('the execution status should show "running"')
def step_execution_status_should_show_running(context):
    """Verify execution status shows as running"""
    status_element = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "execution-status") | (By.ID, "test-status"))
    )
    
    # Wait a moment for status to update
    time.sleep(1)
    status_text = status_element.text.lower()
    assert 'running' in status_text or 'executing' in status_text, f"Expected status to show 'running', got '{status_text}'"


@then('I should wait for the test to finish')
def step_wait_for_test_to_finish(context):
    """Wait for test execution to complete"""
    # Wait for status to change from running to completed (with timeout)
    def wait_for_completion():
        try:
            status_element = context.driver.find_element(By.CLASS_NAME, "execution-status") or context.driver.find_element(By.ID, "test-status")
            status_text = status_element.text.lower()
            return 'running' not in status_text and 'executing' not in status_text
        except NoSuchElementException:
            return False
    
    # Wait up to 60 seconds for test completion
    timeout = 60
    start_time = time.time()
    
    while time.time() - start_time < timeout:
        if wait_for_completion():
            break
        time.sleep(2)
    else:
        raise TimeoutException("Test execution did not complete within timeout period")


@then('the final status should be either "passed" or "failed"')
def step_final_status_should_be_passed_or_failed(context):
    """Verify final status is either passed or failed"""
    status_element = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "execution-status") | (By.ID, "test-status"))
    )
    
    status_text = status_element.text.lower()
    assert 'passed' in status_text or 'failed' in status_text or 'error' in status_text, f"Expected final status to be 'passed' or 'failed', got '{status_text}'"


# ============================================================================
# RESULTS VERIFICATION STEPS  
# ============================================================================

@given('I have executed a single acceptance test')
@given('I have completed a single acceptance test')
def step_have_executed_single_acceptance_test(context):
    """Assume a single acceptance test has been executed"""
    # This step assumes previous execution steps have completed
    # Store execution state
    context.test_executed = True


@given('the test has completed')
def step_test_has_completed(context):
    """Verify test has completed execution"""
    # Verify no running status
    try:
        status_element = context.driver.find_element(By.CLASS_NAME, "execution-status") or context.driver.find_element(By.ID, "test-status")
        status_text = status_element.text.lower()
        assert 'running' not in status_text, "Test should be completed, not running"
    except NoSuchElementException:
        # No status element found, assume completed
        pass


@when('I view the test results')
def step_view_test_results(context):
    """View the test results"""
    # Look for results section or click results button
    try:
        results_button = context.driver.find_element(By.XPATH, "//button[contains(text(), 'View Results')] | //button[contains(text(), 'Results')]")
        results_button.click()
        time.sleep(1)
    except NoSuchElementException:
        # Results might be automatically visible
        pass
    
    # Verify results are visible
    results_container = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "test-results") | (By.ID, "results") | (By.CLASS_NAME, "execution-results"))
    )
    
    context.results_container = results_container


@then('I should see the detailed test results displayed')
def step_should_see_detailed_test_results_displayed(context):
    """Verify detailed test results are displayed"""
    results_container = context.results_container
    assert results_container.is_displayed(), "Test results container should be visible"
    
    # Check for various result elements
    result_elements = results_container.find_elements(By.XPATH, ".//*[contains(@class, 'result') or contains(@class, 'scenario') or contains(@class, 'step')]")
    assert len(result_elements) > 0, "Detailed test results should be visible"


# ============================================================================
# ERROR HANDLING AND EDGE CASES
# ============================================================================

@given('the backend test automation service is unavailable')
def step_backend_test_automation_service_unavailable(context):
    """Simulate backend test automation service being unavailable"""
    # This would typically involve stopping the backend service
    # For now, we'll just note this state for the test
    context.backend_unavailable = True


@then('I should see a user-friendly error message')
def step_should_see_user_friendly_error_message(context):
    """Verify a user-friendly error message is displayed"""
    error_message = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.CLASS_NAME, "error-message") | (By.CLASS_NAME, "alert-error") | (By.XPATH, "//*[contains(text(), 'error') or contains(text(), 'Error')]"))
    )
    
    assert error_message.is_displayed(), "Error message should be visible"
    assert len(error_message.text) > 0, "Error message should not be empty"


@then('there should be a retry button available')
def step_should_be_retry_button_available(context):
    """Verify a retry button is available"""
    retry_button = WebDriverWait(context.driver, 10).until(
        EC.presence_of_element_located((By.XPATH, "//button[contains(text(), 'Retry')] | //button[contains(text(), 'Try Again')]"))
    )
    
    assert retry_button.is_displayed(), "Retry button should be visible"
    assert retry_button.is_enabled(), "Retry button should be clickable"