@web-ui @tag-filtering @behave-tags @critical
Feature: Behave Tag Filtering and Selection
  As a test engineer
  I want to filter and select tests by Behave tags
  So that I can run specific test categories and combinations

  Background:
    Given the web UI Test Automation panel is loaded
    And test scenarios with various tags are available
    And the tag filtering interface is visible

  @smoke @tag-display
  Scenario: TGF-001 Display available Behave tags
    When I navigate to the Tags section
    Then I should see all available tags from feature files
    And tags should be categorized by type
    And each tag should show the count of associated scenarios
    And common tags should be highlighted (smoke, critical, hil, etc.)

  @tag-filtering @single-tag
  Scenario: TGF-002 Filter scenarios by single tag
    Given multiple scenarios with different tags exist
    When I select the "@smoke" tag
    Then only scenarios tagged with "smoke" should be displayed
    And the scenario count should update accordingly
    And the selection should be visually indicated

  @tag-filtering @multiple-tags
  Scenario: TGF-003 Filter scenarios by multiple tags (AND logic)
    Given scenarios with overlapping tags exist
    When I select both "@hil" and "@gpio" tags
    Then only scenarios tagged with both "hil" AND "gpio" should be displayed
    And the filtered count should reflect the intersection
    And both tags should show as active

  @tag-filtering @or-logic
  Scenario: TGF-004 Filter scenarios by multiple tags (OR logic)
    Given scenarios with various tags exist
    When I enable OR mode and select "@smoke" and "@critical" tags
    Then scenarios tagged with either "smoke" OR "critical" should be displayed
    And the filtered count should reflect the union
    And the OR mode should be visually indicated

  @tag-search @dynamic-filtering
  Scenario: TGF-005 Search and filter tags dynamically
    Given many tags are available
    When I type "hil" in the tag search box
    Then only tags containing "hil" should be displayed
    And I should see "hil", "hil-gpio", "hil-modbus", etc.
    When I clear the search
    Then all tags should be visible again

  @tag-categories @organization
  Scenario: TGF-006 Browse tags by category
    Given tags are organized by categories
    When I expand the "Hardware" tag category
    Then I should see hardware-related tags like "gpio", "pwm", "adc"
    When I expand the "Communication" category
    Then I should see "modbus", "uart", "serial" tags
    When I expand the "Test Type" category
    Then I should see "smoke", "regression", "critical" tags

  @tag-execution @filtered-run
  Scenario: TGF-007 Execute tests based on tag selection
    Given I have filtered scenarios by "@gpio" tag
    And 5 scenarios are shown in the filtered view
    When I click "Run Filtered Tests"
    Then only the 5 GPIO-related scenarios should execute
    And the progress should show "5 total scenarios"
    And the execution should complete all GPIO tests

  @tag-presets @quick-selection
  Scenario: TGF-008 Use predefined tag preset combinations
    Given tag presets are available
    When I select the "Smoke Test Suite" preset
    Then scenarios with "@smoke" tag should be selected
    When I select the "Hardware Verification" preset
    Then scenarios with "@hil @gpio @pwm @adc" tags should be selected
    When I select the "Communication Tests" preset
    Then scenarios with "@modbus @uart @communication" tags should be selected

  @tag-management @custom-combinations
  Scenario: TGF-009 Save custom tag combinations
    Given I have selected tags "@hil", "@gpio", and "@critical"
    When I click "Save Tag Combination"
    And I name it "Critical Hardware Tests"
    Then the combination should be saved as a preset
    And I should be able to select it from the presets list
    And it should restore the same tag selection

  @tag-validation @edge-cases
  Scenario: TGF-010 Handle edge cases in tag filtering
    Given various edge case scenarios exist
    When I select a tag with no matching scenarios
    Then I should see "No scenarios found" message
    When I select contradictory tags that can't coexist
    Then I should see appropriate warning messages
    When I select all tags
    Then all scenarios should be displayed (same as no filter)

  @tag-performance @large-datasets
  Scenario: TGF-011 Handle large numbers of tags efficiently
    Given more than 100 different tags exist
    When I open the tag selection interface
    Then the interface should load within 2 seconds
    When I search for tags
    Then search results should appear instantly
    When I select multiple tags
    Then filtering should complete within 1 second
