import { EventEmitter } from 'events';
export interface TestStep {
    step_type: string;
    description: string;
    status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
    duration_ms?: number;
    error_message?: string;
    pin_interactions: string[];
}
export interface TestScenario {
    name: string;
    description: string;
    feature_file: string;
    feature_name?: string;
    tags: string[];
    steps: TestStep[];
    status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
    duration_ms?: number;
    error_message?: string;
}
export interface TestExecution {
    execution_id: string;
    scenarios: TestScenario[];
    status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
    start_time?: number;
    end_time?: number;
    total_scenarios: number;
    passed_scenarios: number;
    failed_scenarios: number;
    current_scenario_index: number;
    error_message?: string;
}
export declare class TestAutomationService extends EventEmitter {
    private pythonProcess;
    private currentExecution;
    private pythonServicePath;
    constructor();
    /**
     * Get available test scenarios by calling Python service
     */
    getAvailableScenarios(): Promise<TestScenario[]>;
    /**
     * Execute selected test scenarios
     */
    executeScenarios(scenarioNames: string[], executionId: string): Promise<boolean>;
    /**
     * Handle progress updates from Python service
     */
    private handleProgressUpdate;
    /**
     * Stop current test execution
     */
    stopExecution(): boolean;
    /**
     * Get current execution status
     */
    getCurrentExecution(): TestExecution | null;
    /**
     * Check if execution is in progress
     */
    isExecutionInProgress(): boolean;
    /**
     * Get test scenarios filtered by tags
     */
    getScenariosByTags(tags: string[]): Promise<TestScenario[]>;
    /**
     * Get test scenarios for specific feature files
     */
    getScenariosByFeature(featureFiles: string[]): Promise<TestScenario[]>;
    /**
     * Get available tags from all scenarios
     */
    getAvailableTags(): Promise<string[]>;
    /**
     * Get available feature files
     */
    getAvailableFeatures(): Promise<string[]>;
    /**
     * Validate scenario selection
     */
    validateScenarios(scenarioNames: string[]): Promise<{
        valid: boolean;
        missing: string[];
    }>;
}
