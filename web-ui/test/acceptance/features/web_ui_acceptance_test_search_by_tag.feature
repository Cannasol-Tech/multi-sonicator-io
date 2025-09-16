@web-ui @acceptance-testing @tag-search
Feature: Web-UI Acceptance Test Search by Tag
  As a test engineer
  I want to search and filter acceptance tests by tags in the web-ui
  So that I can efficiently find and run specific test categories

  Background:
    Given the web-ui is running in simulation mode
    And the Test Automation panel is loaded
    And acceptance test scenarios are loaded from the backend
    And the hardware mock is enabled

  @smoke @tag-filtering @search
  Scenario: Filter acceptance tests by single tag
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I select the tag filter dropdown
    And I select the "smoke" tag filter
    Then only scenarios with the "smoke" tag should be displayed
    And the scenario count should update to reflect the filtered results
    And each displayed scenario should have the "smoke" tag

  @tag-filtering @multiple-tags
  Scenario: Filter acceptance tests by multiple tags
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I select multiple tags: "hil" and "gpio"
    And I apply the tag filter
    Then only scenarios with both "hil" and "gpio" tags should be displayed
    And the scenario count should reflect the combined filter
    And each displayed scenario should have both selected tags

  @tag-filtering @OR-logic
  Scenario: Search acceptance tests by tag with OR logic
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I enable OR logic for tag filtering
    And I select multiple tags: "smoke" or "critical"
    Then scenarios with either "smoke" OR "critical" tags should be displayed
    And the result set should be larger than AND logic filtering
    And scenarios with only one of the selected tags should be included

  @tag-filtering @clear-filters
  Scenario: Clear tag filters to show all scenarios
    Given I have applied tag filters to the scenario list
    And only filtered scenarios are displayed
    When I click the "Clear Filters" button
    Then all acceptance test scenarios should be displayed again
    And the scenario count should return to the original total
    And no tag filters should be active

  @tag-filtering @tag-count
  Scenario: Display tag counts for filtering
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I open the tag filter dropdown
    Then each available tag should display its occurrence count
    And the counts should add up correctly
    And tags should be sorted by name or frequency
    And only tags that exist in scenarios should be displayed

  @tag-filtering @no-results
  Scenario: Handle no matching results for tag search
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I search for a tag combination that has no matches
    Then I should see a "No matching scenarios found" message
    And the scenario list should be empty
    And there should be an option to clear the filters
    And a suggestion to try different tags should be displayed

  @tag-filtering @real-time-search
  Scenario: Real-time tag search as user types
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I start typing in the tag search box
    And I type "sm" for "smoke" tag
    Then tag suggestions should appear as I type
    And matching scenarios should be filtered in real-time
    And the scenario count should update with each keystroke
    And I should be able to select from the suggested tags