"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.TestAutomationService = void 0;
const child_process_1 = require("child_process");
const events_1 = require("events");
const path_1 = __importDefault(require("path"));
class TestAutomationService extends events_1.EventEmitter {
    constructor() {
        super();
        this.pythonProcess = null;
        this.currentExecution = null;
        this.pythonServicePath = path_1.default.join(__dirname, 'TestAutomationService.py');
    }
    /**
     * Get available test scenarios by calling Python service
     */
    async getAvailableScenarios() {
        return new Promise((resolve, reject) => {
            const pythonProcess = (0, child_process_1.spawn)('python3', [this.pythonServicePath, 'get_scenarios']);
            let output = '';
            let errorOutput = '';
            pythonProcess.stdout?.on('data', (data) => {
                output += data.toString();
            });
            pythonProcess.stderr?.on('data', (data) => {
                errorOutput += data.toString();
            });
            pythonProcess.on('close', (code) => {
                if (code === 0) {
                    try {
                        const scenarios = JSON.parse(output);
                        resolve(scenarios);
                    }
                    catch (error) {
                        reject(new Error(`Failed to parse scenarios: ${error}`));
                    }
                }
                else {
                    reject(new Error(`Python process failed: ${errorOutput}`));
                }
            });
            pythonProcess.on('error', (error) => {
                reject(new Error(`Failed to start Python process: ${error.message}`));
            });
        });
    }
    /**
     * Execute selected test scenarios
     */
    async executeScenarios(scenarioNames, executionId) {
        if (this.pythonProcess) {
            console.log('Test execution already in progress');
            return false;
        }
        try {
            // Start Python service for test execution
            this.pythonProcess = (0, child_process_1.spawn)('python3', [
                this.pythonServicePath,
                'execute_scenarios',
                JSON.stringify(scenarioNames),
                executionId
            ]);
            let output = '';
            let errorOutput = '';
            this.pythonProcess.stdout?.on('data', (data) => {
                const dataStr = data.toString();
                output += dataStr;
                // Try to parse progress updates
                const lines = dataStr.split('\n');
                for (const line of lines) {
                    if (line.trim().startsWith('{')) {
                        try {
                            const progressUpdate = JSON.parse(line.trim());
                            this.handleProgressUpdate(progressUpdate);
                        }
                        catch (e) {
                            // Not JSON, ignore
                        }
                    }
                }
            });
            this.pythonProcess.stderr?.on('data', (data) => {
                errorOutput += data.toString();
                console.error('Python service error:', data.toString());
            });
            this.pythonProcess.on('close', (code) => {
                console.log(`Test execution completed with code: ${code}`);
                this.pythonProcess = null;
                if (this.currentExecution) {
                    this.currentExecution.status = code === 0 ? 'passed' : 'failed';
                    this.currentExecution.end_time = Date.now();
                    this.emit('execution_complete', this.currentExecution);
                }
            });
            this.pythonProcess.on('error', (error) => {
                console.error('Python process error:', error);
                this.pythonProcess = null;
                this.emit('execution_error', error.message);
            });
            return true;
        }
        catch (error) {
            console.error('Failed to start test execution:', error);
            return false;
        }
    }
    /**
     * Handle progress updates from Python service
     */
    handleProgressUpdate(progressData) {
        try {
            this.currentExecution = progressData;
            this.emit('progress_update', this.currentExecution);
        }
        catch (error) {
            console.error('Error handling progress update:', error);
        }
    }
    /**
     * Stop current test execution
     */
    stopExecution() {
        if (this.pythonProcess) {
            this.pythonProcess.kill('SIGTERM');
            this.pythonProcess = null;
            if (this.currentExecution) {
                this.currentExecution.status = 'error';
                this.currentExecution.error_message = 'Execution stopped by user';
                this.currentExecution.end_time = Date.now();
                this.emit('execution_stopped', this.currentExecution);
            }
            return true;
        }
        return false;
    }
    /**
     * Get current execution status
     */
    getCurrentExecution() {
        return this.currentExecution;
    }
    /**
     * Check if execution is in progress
     */
    isExecutionInProgress() {
        return this.pythonProcess !== null;
    }
    /**
     * Get test scenarios filtered by tags
     */
    async getScenariosByTags(tags) {
        const allScenarios = await this.getAvailableScenarios();
        if (tags.length === 0) {
            return allScenarios;
        }
        return allScenarios.filter(scenario => tags.some(tag => scenario.tags.includes(tag)));
    }
    /**
     * Get test scenarios for specific feature files
     */
    async getScenariosByFeature(featureFiles) {
        const allScenarios = await this.getAvailableScenarios();
        if (featureFiles.length === 0) {
            return allScenarios;
        }
        return allScenarios.filter(scenario => featureFiles.includes(scenario.feature_file));
    }
    /**
     * Get available tags from all scenarios
     */
    async getAvailableTags() {
        const scenarios = await this.getAvailableScenarios();
        const tagSet = new Set();
        scenarios.forEach(scenario => {
            scenario.tags.forEach(tag => tagSet.add(tag));
        });
        return Array.from(tagSet).sort();
    }
    /**
     * Get available feature files
     */
    async getAvailableFeatures() {
        const scenarios = await this.getAvailableScenarios();
        const featureSet = new Set();
        scenarios.forEach(scenario => {
            featureSet.add(scenario.feature_file);
        });
        return Array.from(featureSet).sort();
    }
    /**
     * Validate scenario selection
     */
    async validateScenarios(scenarioNames) {
        const availableScenarios = await this.getAvailableScenarios();
        const availableNames = availableScenarios.map(s => s.name);
        const missing = scenarioNames.filter(name => !availableNames.includes(name));
        return {
            valid: missing.length === 0,
            missing
        };
    }
}
exports.TestAutomationService = TestAutomationService;
