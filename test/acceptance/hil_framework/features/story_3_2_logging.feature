# language: en
@epic3 @story3_2 @trace
Feature: S-3.2 Real-Time Data Logging, Export, and Full Traceability
  As a hardware test engineer
  I want reliable real-time data logging, export, and end-to-end traceability across HIL operations and the Web UI
  So that I can analyze hardware behavior over time, share results, and maintain audit-quality traceability.

  Background:
    Given the sandbox mode is prepared and hardware simulation for Sonicator 4 is active @trace:S32-CTX

  @TODO @trace:S32-AC1
  Scenario: Enable logging from the Web UI and see active status
    When the user enables logging at 100ms
    Then logging is active and status shows current rate and buffer status

  @TODO @trace:S32-AC2
  Scenario: Configurable profile maintains drop rate <= 0.5%
    Given a logging profile of 100ms interval and 10-minute retention
    When the system runs for 1 minute
    Then at least 600 records per pin exist with <= 0.5% dropped

  @TODO @trace:S32-AC3
  Scenario: Export CSV and JSONL from the UI
    Given active logging
    When the user exports CSV
    Then the downloaded CSV has timestamp, pin, value, and metadata headers
    When the user exports JSONL
    Then the JSONL contains one valid JSON object per line with correlation IDs

  @TODO @trace:S32-AC4
  Scenario: Backend API supports CSV and JSONL export
    Given logging is enabled
    When a client requests "/api/logs/export?format=csv"
    Then the response code is 200 and Content-Type is text/csv with valid schema
    When a client requests "/api/logs/export?format=jsonl"
    Then the response code is 200 and Content-Type is application/json with valid JSONL schema

  @TODO @trace:S32-AC5
  Scenario: Traceability matrix includes S-3.2 links
    When CI generates the traceability matrix
    Then S-3.2 appears with links to scenarios, code modules, tests, and artifacts

  @TODO @trace:S32-AC6
  Scenario: 60-minute continuous logging stability
    Given logging at 100ms sampling
    When the session runs for 60 minutes
    Then there are zero crashes and drop rate remains <= 0.5%
