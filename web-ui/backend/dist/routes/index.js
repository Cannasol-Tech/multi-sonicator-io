"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setupRoutes = setupRoutes;
function setupRoutes(app, hardwareInterface, testAutomationService) {
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
    // Get scenarios by tags
    app.get('/api/test/scenarios/tags/:tags', async (req, res) => {
        if (!testAutomationService) {
            return res.status(503).json({
                error: 'Test automation service not available',
                timestamp: Date.now()
            });
        }
        try {
            const tags = req.params.tags.split(',').map(tag => tag.trim());
            const scenarios = await testAutomationService.getScenariosByTags(tags);
            res.json({
                scenarios,
                tags,
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
    // API documentation endpoint
    app.get('/api', (req, res) => {
        res.json({
            name: 'Multi-Sonicator-IO Web UI API',
            version: '1.0.0',
            description: 'REST API for Arduino Test Harness - ATmega32A DUT control',
            endpoints: {
                'GET /api/health': 'Health check and system status',
                'GET /api/pins': 'Get all pin states',
                'GET /api/pins/:signal': 'Get specific pin state by signal name',
                'POST /api/command': 'Send hardware command',
                'GET /api/connection': 'Get hardware connection status',
                'GET /api/test/scenarios': 'Get available BDD test scenarios',
                'GET /api/test/scenarios/tags/:tags': 'Get scenarios filtered by tags',
                'GET /api/test/tags': 'Get available test tags',
                'GET /api/test/features': 'Get available feature files',
                'POST /api/test/execute': 'Execute selected test scenarios',
                'GET /api/test/execution': 'Get current test execution status',
                'POST /api/test/stop': 'Stop current test execution'
            },
            websocket: 'ws://localhost:3001/ws',
            pinMapping: 'Based on docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)'
        });
    });
}
