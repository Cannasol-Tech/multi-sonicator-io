import { EventEmitter } from 'events';

/**
 * @file TestAutomationService.d.ts
 * @brief TypeScript declaration file for the Test Automation Service
 * @details This service manages and executes automated test scenarios, integrating with a Python-based test automation framework.
 * @author Cannasol Technologies
 * @copyright Proprietary
 */

/**
 * @brief Status types for test execution
 * @details Represents all possible states a test can be in during execution
 */
export type TestStatus = 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';

/**
 * @brief Represents a single test step within a test scenario
 * @details A test step is the smallest unit of execution in a test scenario, typically corresponding to a Given/When/Then statement in BDD
 */
export interface TestStep {
    /**
     * @brief The type/category of the test step
     * @details Common values include 'Given', 'When', 'Then', 'And', 'But'
     */
    step_type: string;
    
    /**
     * @brief Human-readable description of what this step does
     * @details Full text of the step as written in the feature file
     */
    description: string;
    
    /**
     * @brief Current execution status of this step
     * @details Tracks the progress of the step through its lifecycle
     */
    status: TestStatus;
    
    /**
     * @brief Time taken to execute this step in milliseconds
     * @details Measured duration from start to completion of the step
     */
    duration_ms?: number;
    
    /**
     * @brief Error message if the step failed
     * @details Contains detailed information about what went wrong during execution
     */
    error_message?: string;
    
    /**
     * @brief List of hardware pin interactions performed during this step
     * @details Tracks which hardware pins were accessed or modified during execution
     */
    pin_interactions: string[];
}

/**
 * @brief Represents a complete test scenario containing multiple steps
 * @details A test scenario is a collection of related test steps that validate a specific feature or behavior
 */
export interface TestScenario {
    /**
     * @brief Unique name identifier for the scenario
     * @details Used to reference and execute specific scenarios
     */
    name: string;
    
    /**
     * @brief Human-readable description of what this scenario tests
     * @details More detailed explanation of the scenario's purpose
     */
    description: string;
    
    /**
     * @brief Path to the feature file containing this scenario
     * @details File name where this scenario is defined
     */
    feature_file: string;
    
    /**
     * @brief Name of the feature this scenario belongs to
     * @details Optional feature name from the feature file
     */
    feature_name?: string;
    
    /**
     * @brief List of tags associated with this scenario for filtering
     * @details Used for organizing and selecting scenarios for execution
     */
    tags: string[];
    
    /**
     * @brief Array of test steps that make up this scenario
     * @details Ordered collection of steps to execute for this scenario
     */
    steps: TestStep[];
    
    /**
     * @brief Current execution status of the entire scenario
     * @details Overall status of the scenario execution
     */
    status: TestStatus;
    
    /**
     * @brief Total time taken to execute all steps in milliseconds
     * @details Cumulative duration of all steps in the scenario
     */
    duration_ms?: number;
    
    /**
     * @brief Error message if the scenario failed
     * @details Contains information about what caused the scenario to fail
     */
    error_message?: string;
}

/**
 * @brief Represents a complete test execution session containing multiple scenarios
 * @details A test execution is a single run of one or more test scenarios
 */
export interface TestExecution {
    /**
     * @brief Unique identifier for this execution session
     * @details Used to track and reference specific execution runs
     */
    execution_id: string;
    
    /**
     * @brief Array of test scenarios included in this execution
     * @details Collection of scenarios that were executed in this session
     */
    scenarios: TestScenario[];
    
    /**
     * @brief Overall status of the entire execution
     * @details Aggregate status based on individual scenario results
     */
    status: TestStatus;
    
    /**
     * @brief Timestamp when execution started
     * @details Unix timestamp in milliseconds when the execution began
     */
    start_time?: number;
    
    /**
     * @brief Timestamp when execution completed
     * @details Unix timestamp in milliseconds when the execution finished
     */
    end_time?: number;
    
    /**
     * @brief Total number of scenarios in this execution
     * @details Count of all scenarios scheduled for execution
     */
    total_scenarios: number;
    
    /**
     * @brief Number of scenarios that passed
     * @details Count of scenarios that completed successfully
     */
    passed_scenarios: number;
    
    /**
     * @brief Number of scenarios that failed
     * @details Count of scenarios that encountered failures
     */
    failed_scenarios: number;
    
    /**
     * @brief Index of the currently executing scenario
     * @details Zero-based index indicating which scenario is currently running
     */
    current_scenario_index: number;
    
    /**
     * @brief Error message if the execution failed at a high level
     * @details Contains information about execution-level failures
     */
    error_message?: string;
}

/**
 * @brief Statistics about test execution history
 * @details Aggregated metrics about past test executions
 */
export interface ExecutionStats {
    /**
     * @brief Total number of executions
     * @details Count of all execution sessions recorded
     */
    totalExecutions: number;
    
    /**
     * @brief Total number of scenarios across all executions
     * @details Cumulative count of all scenarios that have been executed
     */
    totalScenarios: number;
    
    /**
     * @brief Total number of passed scenarios
     * @details Count of all scenarios that completed successfully
     */
    totalPassed: number;
    
    /**
     * @brief Total number of failed scenarios
     * @details Count of all scenarios that encountered failures
     */
    totalFailed: number;
    
    /**
     * @brief Average execution duration in seconds
     * @details Mean duration of completed execution sessions
     */
    averageDuration: number;
    
    /**
     * @brief Success rate as a percentage
     * @details Percentage of scenarios that passed successfully
     */
    successRate: number;
}

/**
 * @brief Result of scenario validation
 * @details Outcome of checking whether specified scenarios exist and are valid
 */
export interface ScenarioValidationResult {
    /**
     * @brief Whether all scenarios are valid
     * @details True if all specified scenario names were found
     */
    valid: boolean;
    
    /**
     * @brief List of missing scenario names
     * @details Names of scenarios that could not be found
     */
    missing: string[];
}

/**
 * @brief Categorized tags for easier filtering
 * @details Tags organized by functional categories for better organization
 */
export interface CategorizedTags {
    /**
     * @brief Test type tags
     * @details Tags that indicate the type or priority of tests
     */
    'Test Type': string[];
    
    /**
     * @brief Hardware-related tags
     * @details Tags that relate to specific hardware components or systems
     */
    'Hardware': string[];
    
    /**
     * @brief Communication protocol tags
     * @details Tags that relate to communication interfaces or protocols
     */
    'Communication': string[];
    
    /**
     * @brief Feature-specific tags
     * @details Tags that relate to specific product features
     */
    'Feature': string[];
    
    /**
     * @brief Other tags that don't fit in other categories
     * @details Miscellaneous tags that don't belong to other categories
     */
    'Other': string[];
}

/**
 * @brief Export formats supported by the service
 * @details Available formats for exporting test execution results
 */
export type ExportFormat = 'json' | 'csv' | 'html';

/**
 * @brief Service for managing and executing automated test scenarios
 * @details This service integrates with a Python-based test automation framework to provide
 *          BDD test execution capabilities through a web interface.
 * @extends EventEmitter
 */
export declare class TestAutomationService extends EventEmitter {
    /**
     * @brief Initialize the test automation service
     * @details Sets up internal state, initializes tag presets, and prepares for test execution
     */
    constructor();
    
    /**
     * @brief Get available test scenarios by calling Python service
     * @details Parses feature files and returns all available test scenarios
     * @returns {Promise<TestScenario[]>} Promise resolving to array of available test scenarios
     * @throws {Error} If the Python process fails or scenarios cannot be parsed
     */
    getAvailableScenarios(): Promise<TestScenario[]>;
    
    /**
     * @brief Get all available tags from loaded scenarios
     * @details Extracts and returns all unique tags from the available scenarios
     * @returns {Promise<string[]>} Promise resolving to array of available tags
     */
    getAvailableTags(): Promise<string[]>;
    
    /**
     * @brief Get scenarios filtered by tags with AND/OR logic
     * @details Filters scenarios based on specified tags using either AND or OR logic
     * @param {string[]} tags - Array of tags to filter by
     * @param {boolean} [useOrLogic=false] - Whether to use OR logic (true) or AND logic (false)
     * @returns {Promise<TestScenario[]>} Promise resolving to filtered scenarios
     */
    getScenariosByTags(tags: string[], useOrLogic?: boolean): Promise<TestScenario[]>;
    
    /**
     * @brief Search scenarios by text query
     * @details Searches scenario names, descriptions, feature files, and tags for matching text
     * @param {string} query - Text to search for in scenario metadata
     * @returns {Promise<TestScenario[]>} Promise resolving to matching scenarios
     */
    searchScenarios(query: string): Promise<TestScenario[]>;
    
    /**
     * @brief Get tag presets
     * @details Returns a copy of the currently defined tag preset combinations
     * @returns {Record<string, string[]>} Copy of current tag presets
     */
    getTagPresets(): Record<string, string[]>;
    
    /**
     * @brief Save custom tag combination
     * @details Stores a named combination of tags for quick scenario filtering
     * @param {string} name - Name of the preset
     * @param {string[]} tags - Array of tags for this preset
     * @returns {boolean} Whether the operation was successful
     */
    saveTagPreset(name: string, tags: string[]): boolean;
    
    /**
     * @brief Get tags categorized by type
     * @details Organizes tags into functional categories for easier filtering
     * @returns {Promise<CategorizedTags>} Promise resolving to categorized tags
     */
    getCategorizedTags(): Promise<CategorizedTags>;
    
    /**
     * @brief Execute selected test scenarios
     * @details Starts execution of specified scenarios through the Python test framework
     * @param {string[]} scenarioNames - Names of scenarios to execute
     * @param {string} executionId - Unique identifier for this execution
     * @returns {Promise<boolean>} Promise resolving to whether execution started successfully
     */
    executeScenarios(scenarioNames: string[], executionId: string): Promise<boolean>;
    
    /**
     * @brief Stop current test execution
     * @details Terminates the currently running test execution if one exists
     * @returns {boolean} Whether there was an execution to stop
     */
    stopExecution(): boolean;
    
    /**
     * @brief Get current execution status
     * @details Returns the current execution session if one is in progress
     * @returns {TestExecution | null} Current execution or null if none in progress
     */
    getCurrentExecution(): TestExecution | null;
    
    /**
     * @brief Check if execution is in progress
     * @details Determines whether a test execution is currently running
     * @returns {boolean} Whether an execution is currently in progress
     */
    isExecutionInProgress(): boolean;
    
    /**
     * @brief Get test scenarios for specific feature files
     * @details Filters scenarios to only those defined in specified feature files
     * @param {string[]} featureFiles - Array of feature file names to filter by
     * @returns {Promise<TestScenario[]>} Promise resolving to scenarios from specified feature files
     */
    getScenariosByFeature(featureFiles: string[]): Promise<TestScenario[]>;
    
    /**
     * @brief Get available feature files
     * @details Returns a list of all feature files that contain test scenarios
     * @returns {Promise<string[]>} Promise resolving to array of feature file names
     */
    getAvailableFeatures(): Promise<string[]>;
    
    /**
     * @brief Validate scenario selection
     * @details Checks whether specified scenario names exist in the available scenarios
     * @param {string[]} scenarioNames - Names of scenarios to validate
     * @returns {Promise<ScenarioValidationResult>} Promise resolving to validation result
     */
    validateScenarios(scenarioNames: string[]): Promise<ScenarioValidationResult>;
    
    /**
     * @brief Get execution history
     * @details Returns a copy of the stored execution history
     * @returns {TestExecution[]} Copy of execution history
     */
    getExecutionHistory(): TestExecution[];
    
    /**
     * @brief Clear execution history
     * @details Removes all stored execution history
     */
    clearExecutionHistory(): void;
    
    /**
     * @brief Get execution statistics
     * @details Calculates and returns aggregated statistics about execution history
     * @returns {ExecutionStats} Execution statistics
     */
    getExecutionStats(): ExecutionStats;
    
    /**
     * @brief Export execution results in various formats
     * @details Converts execution results to specified format for external use
     * @param {string} executionId - ID of execution to export
     * @param {ExportFormat} format - Format to export in
     * @returns {Promise<string>} Promise resolving to exported data as string
     * @throws {Error} If execution ID is not found or format is unsupported
     */
    exportExecutionResults(executionId: string, format: ExportFormat): Promise<string>;
}
