"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.setupRoutes = setupRoutes;
const config_1 = __importDefault(require("./config"));
const tests_1 = __importDefault(require("./tests"));
function setupRoutes(app, hardwareInterface, testAutomationService) {
    // Hardware configuration management routes - make sure this is the first route
    app.use('/api/config', config_1.default);
    // Hardware test execution routes
    app.use('/api/tests', tests_1.default);
    // Health check endpoint
    app.get('/api/health', (req, res) => {
        res.json({
            status: 'ok',
            timestamp: Date.now(),
            hardware: {
                connected: hardwareInterface.isConnected(),
                port: hardwareInterface.getSerialPort()
            }
        });
    });
    // Get current pin states
    app.get('/api/pins', (req, res) => {
        const pinStates = Object.fromEntries(hardwareInterface.getPinStates());
        res.json({
            pins: pinStates,
            timestamp: Date.now()
        });
    });
    // Get specific pin state
    app.get('/api/pins/:signal', (req, res) => {
        const { signal } = req.params;
        const pinStates = hardwareInterface.getPinStates();
        const pinState = pinStates.get(signal);
        if (!pinState) {
            return res.status(404).json({
                error: `Pin signal '${signal}' not found`,
                availablePins: Array.from(pinStates.keys())
            });
        }
        res.json({
            pin: pinState,
            timestamp: Date.now()
        });
    });
    // Control specific pin state
    app.post('/api/pins/:signal', async (req, res) => {
        try {
            const { signal } = req.params;
            const { state } = req.body;
            console.log(`API: Controlling pin ${signal} to ${state}`);
            const pinStates = hardwareInterface.getPinStates();
            const pinState = pinStates.get(signal);
            if (!pinState) {
                console.log(`API: Pin ${signal} not found`);
                return res.status(404).json({
                    error: `Pin signal '${signal}' not found`,
                    availablePins: Array.from(pinStates.keys())
                });
            }
            // Only allow control of input pins
            if (pinState.direction !== 'IN') {
                console.log(`API: Pin ${signal} is not an input pin (${pinState.direction})`);
                return res.status(400).json({
                    error: `Pin '${signal}' is not an input pin (direction: ${pinState.direction})`,
                    message: 'Only input pins can be controlled'
                });
            }
            // Validate state value
            if (!['HIGH', 'LOW'].includes(state)) {
                console.log(`API: Invalid state value: ${state}`);
                return res.status(400).json({
                    error: 'Invalid state value',
                    message: 'State must be either "HIGH" or "LOW"'
                });
            }
            // Send command to hardware
            const command = {
                command: 'write_pin',
                args: [signal, state],
                expectResponse: true
            };
            console.log(`API: Sending command:`, command);
            const result = await hardwareInterface.sendCommand(command);
            console.log(`API: Command result:`, result);
            res.json({
                success: true,
                signal,
                state,
                result,
                timestamp: Date.now()
            });
        }
        catch (error) {
            console.error('Error controlling pin:', error);
            res.status(500).json({
                error: 'Failed to control pin',
                message: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    });
    // Send hardware command
    app.post('/api/command', async (req, res) => {
        try {
            const { command, args } = req.body;
            if (!command) {
                return res.status(400).json({
                    error: 'Command is required'
                });
            }
            const response = await hardwareInterface.sendCommand({
                command,
                args,
                expectResponse: true
            });
            res.json({
                command,
                args,
                response,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to execute command',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get connection status
    app.get('/api/connection', (req, res) => {
        res.json({
            connected: hardwareInterface.isConnected(),
            port: hardwareInterface.getSerialPort(),
            timestamp: Date.now()
        });
    });
    // PING hardware to test communication
    app.post('/api/ping', async (req, res) => {
        try {
            console.log('API: PING command requested');
            const startTime = Date.now();
            if (!hardwareInterface.isConnected()) {
                return res.status(503).json({
                    success: false,
                    error: 'Hardware not connected',
                    timestamp: Date.now()
                });
            }
            const command = {
                command: 'ping',
                args: [],
                expectResponse: true
            };
            console.log('API: Sending PING command to hardware');
            const result = await hardwareInterface.sendCommand(command);
            const responseTime = Date.now() - startTime;
            console.log(`API: PING response received in ${responseTime}ms:`, result);
            res.json({
                success: result.success,
                responseTime,
                result,
                timestamp: Date.now()
            });
        }
        catch (error) {
            console.error('Error sending PING command:', error);
            res.status(500).json({
                success: false,
                error: 'Failed to send PING command',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get current configuration
    app.get('/api/config', (req, res) => {
        try {
            const config = hardwareInterface.getConfiguration();
            res.json({
                success: true,
                config,
                timestamp: Date.now()
            });
        }
        catch (error) {
            console.error('Error getting configuration:', error);
            res.status(500).json({
                success: false,
                error: 'Failed to get configuration',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Update configuration
    app.post('/api/config', async (req, res) => {
        try {
            const { config } = req.body;
            console.log('API: Configuration update requested:', config);
            if (!hardwareInterface.isConnected()) {
                return res.status(503).json({
                    success: false,
                    error: 'Hardware not connected',
                    timestamp: Date.now()
                });
            }
            const result = await hardwareInterface.updateConfiguration(config);
            console.log('API: Configuration update result:', result);
            res.json({
                success: result.success,
                config: result.config,
                timestamp: Date.now()
            });
        }
        catch (error) {
            console.error('Error updating configuration:', error);
            res.status(500).json({
                success: false,
                error: 'Failed to update configuration',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Reconnect to hardware
    app.post('/api/connection/reconnect', async (req, res) => {
        try {
            console.log('Manual reconnection requested');
            const success = await hardwareInterface.reconnect();
            res.json({
                success,
                connected: hardwareInterface.isConnected(),
                port: hardwareInterface.getSerialPort(),
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                error: 'Reconnection failed',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Retry connection with multiple attempts
    app.post('/api/connection/retry', async (req, res) => {
        try {
            const { maxAttempts = 3, delayMs = 2000 } = req.body;
            console.log(`Connection retry requested: ${maxAttempts} attempts`);
            const success = await hardwareInterface.retryConnection(maxAttempts, delayMs);
            res.json({
                success,
                connected: hardwareInterface.isConnected(),
                port: hardwareInterface.getSerialPort(),
                attempts: maxAttempts,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                error: 'Connection retry failed',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Test Automation Endpoints
    // Get available test scenarios
    app.get('/api/test/scenarios', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const scenarios = await testAutomationService.getAvailableScenarios();
            res.json({
                scenarios,
                count: scenarios.length,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get test scenarios',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get scenarios by tags with advanced filtering
    app.get('/api/test/scenarios/tags/:tags', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const tags = req.params.tags.split(',').map(tag => tag.trim());
            const useOrLogic = req.query.logic === 'or';
            const scenarios = await testAutomationService.getScenariosByTags(tags, useOrLogic);
            res.json({
                scenarios,
                tags,
                logic: useOrLogic ? 'OR' : 'AND',
                count: scenarios.length,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get scenarios by tags',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Search scenarios by text query
    app.get('/api/test/scenarios/search', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const query = req.query.q;
            if (!query || query.trim().length === 0) {
                return res.status(400).json({
                    error: 'Search query parameter "q" is required'
                });
            }
            const scenarios = await testAutomationService.searchScenarios(query.trim());
            res.json({
                scenarios,
                query: query.trim(),
                count: scenarios.length,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to search scenarios',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get available tags
    app.get('/api/test/tags', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const tags = await testAutomationService.getAvailableTags();
            res.json({
                tags,
                count: tags.length,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get available tags',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get categorized tags
    app.get('/api/test/tags/categorized', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const categorizedTags = await testAutomationService.getCategorizedTags();
            res.json({
                categories: categorizedTags,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get categorized tags',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get tag presets
    app.get('/api/test/tags/presets', (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const presets = testAutomationService.getTagPresets();
            res.json({
                presets,
                count: Object.keys(presets).length,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get tag presets',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Save custom tag preset
    app.post('/api/test/tags/presets', (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const { name, tags } = req.body;
            if (!name || typeof name !== 'string' || name.trim().length === 0) {
                return res.status(400).json({
                    error: 'Preset name is required and must be a non-empty string'
                });
            }
            if (!tags || !Array.isArray(tags)) {
                return res.status(400).json({
                    error: 'Tags must be provided as an array'
                });
            }
            const success = testAutomationService.saveTagPreset(name.trim(), tags);
            res.json({
                success,
                name: name.trim(),
                tags,
                message: success ? 'Tag preset saved successfully' : 'Failed to save tag preset',
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to save tag preset',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get available feature files
    app.get('/api/test/features', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const features = await testAutomationService.getAvailableFeatures();
            res.json({
                features,
                count: features.length,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get available features',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Execute test scenarios
    app.post('/api/test/execute', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const { scenarios, execution_id } = req.body;
            if (!scenarios || !Array.isArray(scenarios)) {
                return res.status(400).json({
                    error: 'Scenarios array is required'
                });
            }
            if (!execution_id) {
                return res.status(400).json({
                    error: 'Execution ID is required'
                });
            }
            // Validate scenarios exist
            const validation = await testAutomationService.validateScenarios(scenarios);
            if (!validation.valid) {
                return res.status(400).json({
                    error: 'Invalid scenarios',
                    missing_scenarios: validation.missing
                });
            }
            const success = await testAutomationService.executeScenarios(scenarios, execution_id);
            if (success) {
                res.json({
                    success: true,
                    execution_id,
                    scenarios,
                    message: 'Test execution started',
                    timestamp: Date.now()
                });
            }
            else {
                res.status(409).json({
                    success: false,
                    error: 'Test execution already in progress',
                    timestamp: Date.now()
                });
            }
        }
        catch (error) {
            res.status(500).json({
                success: false,
                error: 'Failed to start test execution',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get current test execution status
    app.get('/api/test/execution', (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        const execution = testAutomationService.getCurrentExecution();
        res.json({
            execution,
            in_progress: testAutomationService.isExecutionInProgress(),
            timestamp: Date.now()
        });
    });
    // Stop current test execution
    app.post('/api/test/stop', (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        const success = testAutomationService.stopExecution();
        res.json({
            success,
            message: success ? 'Test execution stopped' : 'No test execution in progress',
            timestamp: Date.now()
        });
    });
    // Get execution history
    app.get('/api/test/history', (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const history = testAutomationService.getExecutionHistory();
            res.json({
                history,
                count: history.length,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get execution history',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Get execution statistics
    app.get('/api/test/statistics', (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const stats = testAutomationService.getExecutionStats();
            res.json({
                statistics: stats,
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to get execution statistics',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Export execution results
    app.get('/api/test/export/:executionId', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const { executionId } = req.params;
            const format = req.query.format || 'json';
            if (!['json', 'csv', 'html'].includes(format)) {
                return res.status(400).json({
                    error: 'Invalid format. Supported formats: json, csv, html'
                });
            }
            const exportData = await testAutomationService.exportExecutionResults(executionId, format);
            // Set appropriate content type and headers
            switch (format) {
                case 'json':
                    res.setHeader('Content-Type', 'application/json');
                    res.setHeader('Content-Disposition', `attachment; filename="execution_${executionId}.json"`);
                    break;
                case 'csv':
                    res.setHeader('Content-Type', 'text/csv');
                    res.setHeader('Content-Disposition', `attachment; filename="execution_${executionId}.csv"`);
                    break;
                case 'html':
                    res.setHeader('Content-Type', 'text/html');
                    res.setHeader('Content-Disposition', `attachment; filename="execution_${executionId}.html"`);
                    break;
            }
            res.send(exportData);
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to export execution results',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // Clear execution history
    app.delete('/api/test/history', (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            testAutomationService.clearExecutionHistory();
            res.json({
                success: true,
                message: 'Execution history cleared',
                timestamp: Date.now()
            });
        }
        catch (error) {
            res.status(500).json({
                error: 'Failed to clear execution history',
                message: error instanceof Error ? error.message : 'Unknown error',
                timestamp: Date.now()
            });
        }
    });
    // API documentation endpoint
    app.get('/api', (req, res) => {
        res.json({
            name: 'Multi-Sonicator-IO Web UI API',
            version: '1.1.0',
            description: 'REST API for Arduino Test Harness - ATmega32A DUT control with advanced BDD test automation',
            endpoints: {
                // Hardware Control
                'GET /api/health': 'Health check and system status',
                'GET /api/pins': 'Get all pin states',
                'GET /api/pins/:signal': 'Get specific pin state by signal name',
                'POST /api/pins/:signal': 'Control specific pin state',
                'POST /api/command': 'Send hardware command',
                'GET /api/connection': 'Get hardware connection status',
                'POST /api/connection/reconnect': 'Reconnect to hardware',
                'POST /api/connection/retry': 'Retry connection with multiple attempts',
                'POST /api/ping': 'Ping hardware to test communication',
                'GET /api/config': 'Get current hardware configuration',
                'POST /api/config': 'Update hardware configuration',
                // Hardware Tests
                'POST /api/tests/configuration': 'Run hardware configuration tests',
                'POST /api/tests/sonicator/:id': 'Run tests for specific sonicator channel',
                'POST /api/tests/modbus': 'Run MODBUS communication tests',
                'POST /api/tests/all': 'Run all hardware tests',
                'GET /api/tests/status': 'Get test capabilities and status',
                // BDD Test Scenarios
                'GET /api/test/scenarios': 'Get available BDD test scenarios',
                'GET /api/test/scenarios/search?q=query': 'Search scenarios by text query',
                'GET /api/test/scenarios/tags/:tags?logic=and|or': 'Get scenarios filtered by tags with AND/OR logic',
                'POST /api/test/execute': 'Execute selected test scenarios',
                'GET /api/test/execution': 'Get current test execution status',
                'POST /api/test/stop': 'Stop current test execution',
                // Tags and Categorization
                'GET /api/test/tags': 'Get all available test tags',
                'GET /api/test/tags/categorized': 'Get tags organized by category',
                'GET /api/test/tags/presets': 'Get predefined tag combinations',
                'POST /api/test/tags/presets': 'Save custom tag combination',
                // Features and Files
                'GET /api/test/features': 'Get available feature files',
                // Execution History and Analytics
                'GET /api/test/history': 'Get execution history',
                'DELETE /api/test/history': 'Clear execution history',
                'GET /api/test/statistics': 'Get execution statistics and analytics',
                'GET /api/test/export/:executionId?format=json|csv|html': 'Export execution results in various formats'
            },
            queryParameters: {
                '/api/test/scenarios/tags/:tags': {
                    'logic': 'and|or - Tag filtering logic (default: and)'
                },
                '/api/test/scenarios/search': {
                    'q': 'string - Search query for scenario name, description, or tags'
                },
                '/api/test/export/:executionId': {
                    'format': 'json|csv|html - Export format (default: json)'
                }
            },
            websocket: 'ws://localhost:3001/ws',
            pinMapping: 'Based on config/hardware-config.yaml (SOLE SOURCE OF TRUTH)'
        });
    });
}
