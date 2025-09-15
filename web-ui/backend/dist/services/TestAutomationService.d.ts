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
    private cachedScenarios;
    private availableTags;
    private executionHistory;
    private tagPresets;
    constructor();
    private initializeTagPresets;
    /**
     * Get available test scenarios by calling Python service
     */
    getAvailableScenarios(): Promise<TestScenario[]>;
    /**
     * Extract all available tags from scenarios
     */
    private extractAvailableTags;
    /**
     * Get all available tags from loaded scenarios
     */
    getAvailableTags(): Promise<string[]>;
    /**
     * Get scenarios filtered by tags with AND/OR logic
     */
    getScenariosByTags(tags: string[], useOrLogic?: boolean): Promise<TestScenario[]>;
    /**
     * Search scenarios by text query
     */
    searchScenarios(query: string): Promise<TestScenario[]>;
    /**
     * Get tag presets
     */
    getTagPresets(): Record<string, string[]>;
    /**
     * Save custom tag combination
     */
    saveTagPreset(name: string, tags: string[]): boolean;
    /**
     * Get tags categorized by type
     */
    getCategorizedTags(): Promise<Record<string, string[]>>;
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
     * Get test scenarios for specific feature files
     */
    getScenariosByFeature(featureFiles: string[]): Promise<TestScenario[]>;
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
    /**
     * Get execution history
     */
    getExecutionHistory(): TestExecution[];
    /**
     * Add execution to history
     */
    private addExecutionToHistory;
    /**
     * Clear execution history
     */
    clearExecutionHistory(): void;
    /**
     * Get execution statistics
     */
    getExecutionStats(): {
        totalExecutions: number;
        totalScenarios: number;
        totalPassed: number;
        totalFailed: number;
        averageDuration: number;
        successRate: number;
    };
    /**
     * Export execution results in various formats
     */
    exportExecutionResults(executionId: string, format: 'json' | 'csv' | 'html'): Promise<string>;
    private convertExecutionToCSV;
    private convertExecutionToHTML;
}
