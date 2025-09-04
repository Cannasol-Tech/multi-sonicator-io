Feature: End-to-End Production Workflows
  As a production operator
  I want to execute complete production workflows using the Multi-Sonicator I/O Controller
  So that I can efficiently process materials with multiple sonicators

  Background:
    Given the Multi-Sonicator I/O Controller is installed and operational
    And all 4 CT2000 sonicators are connected and calibrated
    And the HMI system is configured for multi-sonicator operation
    And the production recipe database is loaded

  @workflow @production @critical
  Scenario: Standard batch processing workflow
    Given I have a batch of material ready for processing
    And I have selected the appropriate processing recipe
    When I load the recipe parameters into the system
    And I start the batch processing sequence
    Then sonicator 1 should start at the specified amplitude and duration
    And sonicator 2 should start with a 30-second delay as programmed
    And sonicator 3 should start when sonicator 1 reaches target power
    And sonicator 4 should remain on standby until manually activated
    And all process parameters should be logged continuously
    And the batch should complete within the expected timeframe
    And the system should generate a batch completion report

  @workflow @production @efficiency
  Scenario: Parallel processing workflow
    Given I have multiple batches ready for simultaneous processing
    When I configure each sonicator for a different batch
    And I start all sonicators with their respective parameters
    Then each sonicator should process its assigned batch independently
    And cross-contamination prevention protocols should be active
    And each batch should be tracked separately in the system
    And completion times should be optimized for maximum throughput
    And quality parameters should be maintained for each batch

  @workflow @production @quality
  Scenario: Quality control workflow with real-time monitoring
    Given I am processing a critical batch requiring strict quality control
    When I start the processing with enhanced monitoring enabled
    Then all process parameters should be monitored in real-time
    And any deviations from specifications should trigger alerts
    And automatic adjustments should be made within acceptable ranges
    And out-of-specification conditions should pause processing
    And quality data should be recorded with full traceability
    And the batch should only complete if all quality criteria are met

  @workflow @production @maintenance
  Scenario: Preventive maintenance workflow
    Given the system indicates maintenance is due for sonicator 2
    When I initiate the maintenance workflow
    Then sonicator 2 should be safely shut down and isolated
    And other sonicators should continue operating normally
    And the maintenance checklist should be displayed
    And maintenance actions should be logged as completed
    And system performance should be verified after maintenance
    And sonicator 2 should be returned to service only after validation

  @workflow @production @troubleshooting
  Scenario: Fault diagnosis and recovery workflow
    Given sonicator 3 has developed a fault during operation
    When the fault is detected by the system
    Then sonicator 3 should be automatically shut down safely
    And the fault should be logged with timestamp and conditions
    And diagnostic information should be displayed to the operator
    And other sonicators should continue operating if safe to do so
    And the system should guide the operator through troubleshooting steps
    And recovery procedures should be clearly documented and accessible

  @workflow @production @recipe-management
  Scenario: Recipe development and optimization workflow
    Given I need to develop a new processing recipe
    When I access the recipe development interface
    Then I should be able to create a new recipe template
    And I should be able to define parameters for each sonicator
    And I should be able to set timing and sequencing rules
    And I should be able to test the recipe in simulation mode
    And I should be able to run small-scale validation batches
    And I should be able to optimize parameters based on results
    And the validated recipe should be saved to the database

  @workflow @production @shift-handover
  Scenario: Shift change and handover workflow
    Given a shift change is occurring during active processing
    When the incoming operator logs into the system
    Then the current system status should be clearly displayed
    And all active batches should be identified with their progress
    And any pending alarms or issues should be highlighted
    And the outgoing operator should complete a handover checklist
    And critical information should be communicated and acknowledged
    And the incoming operator should confirm readiness to take control

  @workflow @production @emergency-response
  Scenario: Emergency shutdown and recovery workflow
    Given an emergency situation requires immediate system shutdown
    When the emergency stop is activated
    Then all sonicators should stop immediately (within 50ms)
    And all process materials should be secured safely
    And the emergency condition should be logged with full details
    And emergency response procedures should be displayed
    And the system should prevent restart until safety is confirmed
    When the emergency is resolved and safety is verified
    Then the system should guide through restart procedures
    And all systems should be verified before resuming operation

  @workflow @production @data-management
  Scenario: Production data management and reporting workflow
    Given I need to generate production reports for the day
    When I access the reporting interface
    Then I should be able to view production statistics by sonicator
    And I should be able to generate batch history reports
    And I should be able to export data in various formats
    And I should be able to create custom reports based on criteria
    And all data should be traceable to specific batches and operators
    And reports should be automatically archived for compliance

  @pending @workflow @production @advanced-control
  Scenario: Advanced process control workflow
    Given the system supports advanced process control algorithms
    When I enable adaptive control for a complex process
    Then the system should automatically adjust parameters based on feedback
    And machine learning algorithms should optimize processing over time
    And predictive models should anticipate process variations
    And the system should maintain product quality while maximizing efficiency
    And all control actions should be logged and explainable

  @pending @workflow @production @integration
  Scenario: ERP system integration workflow
    Given the system is integrated with the company ERP system
    When a production order is created in the ERP system
    Then the order should automatically appear in the sonicator system
    And material requirements should be validated against inventory
    And production scheduling should be optimized across all resources
    And progress should be reported back to the ERP system in real-time
    And completion should trigger automatic inventory updates

  @pending @workflow @production @mobile-monitoring
  Scenario: Mobile monitoring and control workflow
    Given I have the mobile app installed and configured
    When I am away from the production floor
    Then I should be able to monitor all system status remotely
    And I should receive push notifications for critical alerts
    And I should be able to perform basic control operations remotely
    And I should be able to view real-time production dashboards
    And all remote actions should be logged and auditable

  @pending @workflow @production @energy-optimization
  Scenario: Energy optimization workflow
    Given the system supports energy monitoring and optimization
    When I enable energy optimization mode
    Then the system should monitor power consumption for all sonicators
    And it should optimize operating schedules to minimize energy costs
    And it should recommend energy-saving operational changes
    And it should track and report energy efficiency metrics
    And it should integrate with facility energy management systems
