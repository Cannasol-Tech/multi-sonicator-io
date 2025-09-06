"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.WebSocketHandler = void 0;
const ws_1 = require("ws");
class WebSocketHandler {
    constructor(hardwareInterface, testAutomationService) {
        this.clients = new Set();
        this.hardwareInterface = hardwareInterface;
        this.testAutomationService = testAutomationService;
        this.setupHardwareListeners();
        this.setupTestAutomationListeners();
    }
    setupHardwareListeners() {
        // Listen for hardware events and broadcast to all connected clients
        this.hardwareInterface.on('connected', (data) => {
            this.broadcast({
                type: 'connection_status',
                data: { connected: true, ...data },
                timestamp: Date.now()
            });
        });
        this.hardwareInterface.on('disconnected', () => {
            this.broadcast({
                type: 'connection_status',
                data: { connected: false },
                timestamp: Date.now()
            });
        });
        this.hardwareInterface.on('pin_update', (signal, pinState) => {
            this.broadcast({
                type: 'pin_update',
                data: { signal, pinState },
                timestamp: Date.now()
            });
        });
        this.hardwareInterface.on('error', (error) => {
            this.broadcast({
                type: 'error',
                data: { error },
                timestamp: Date.now()
            });
        });
    }
    setupTestAutomationListeners() {
        if (!this.testAutomationService) {
            return;
        }
        // Listen for test execution progress updates
        this.testAutomationService.on('progress_update', (execution) => {
            this.broadcast({
                type: 'test_progress',
                data: execution,
                timestamp: Date.now()
            });
        });
        // Listen for test execution completion
        this.testAutomationService.on('execution_complete', (execution) => {
            this.broadcast({
                type: 'test_complete',
                data: execution,
                timestamp: Date.now()
            });
        });
        // Listen for test execution errors
        this.testAutomationService.on('execution_error', (error) => {
            this.broadcast({
                type: 'test_error',
                data: { error },
                timestamp: Date.now()
            });
        });
        // Listen for test execution stopped
        this.testAutomationService.on('execution_stopped', (execution) => {
            this.broadcast({
                type: 'test_stopped',
                data: execution,
                timestamp: Date.now()
            });
        });
    }
    handleConnection(ws) {
        this.clients.add(ws);
        console.log(`WebSocket client connected. Total clients: ${this.clients.size}`);
        // Send initial state to new client
        this.sendInitialState(ws);
        ws.on('message', (data) => {
            try {
                const message = JSON.parse(data.toString());
                this.handleMessage(ws, message);
            }
            catch (error) {
                console.error('Failed to parse WebSocket message:', error);
                this.sendError(ws, 'Invalid message format');
            }
        });
        ws.on('close', () => {
            this.clients.delete(ws);
            console.log(`WebSocket client disconnected. Total clients: ${this.clients.size}`);
        });
        ws.on('error', (error) => {
            console.error('WebSocket error:', error);
            this.clients.delete(ws);
        });
    }
    sendInitialState(ws) {
        // Send current connection status
        const message = {
            type: 'initial_state',
            data: {
                connected: this.hardwareInterface.isConnected(),
                port: this.hardwareInterface.getSerialPort(),
                pins: Object.fromEntries(this.hardwareInterface.getPinStates()),
                test_execution: this.testAutomationService?.getCurrentExecution() || null,
                test_in_progress: this.testAutomationService?.isExecutionInProgress() || false
            },
            timestamp: Date.now()
        };
        this.sendToClient(ws, message);
    }
    async handleMessage(ws, message) {
        try {
            switch (message.type) {
                case 'hardware_command':
                    await this.handleHardwareCommand(ws, message.data);
                    break;
                case 'ping':
                    this.sendToClient(ws, {
                        type: 'pong',
                        data: { timestamp: message.timestamp },
                        timestamp: Date.now()
                    });
                    break;
                case 'get_pin_states':
                    this.sendToClient(ws, {
                        type: 'pin_states',
                        data: Object.fromEntries(this.hardwareInterface.getPinStates()),
                        timestamp: Date.now()
                    });
                    break;
                default:
                    console.warn('Unknown WebSocket message type:', message.type);
                    this.sendError(ws, `Unknown message type: ${message.type}`);
            }
        }
        catch (error) {
            console.error('Error handling WebSocket message:', error);
            this.sendError(ws, 'Failed to process message');
        }
    }
    async handleHardwareCommand(ws, commandData) {
        const { command, pin, value } = commandData;
        try {
            let hardwareCommand;
            switch (command) {
                case 'write_pin':
                    hardwareCommand = {
                        command: `WRITE_PIN ${pin} ${value}`,
                        expectResponse: true
                    };
                    break;
                case 'read_pin':
                    hardwareCommand = {
                        command: `READ_PIN ${pin}`,
                        expectResponse: true
                    };
                    break;
                case 'read_adc':
                    hardwareCommand = {
                        command: `READ_ADC ${pin}`,
                        expectResponse: true
                    };
                    break;
                case 'read_pwm':
                    hardwareCommand = {
                        command: `READ_PWM ${pin}`,
                        expectResponse: true
                    };
                    break;
                case 'set_pwm':
                    hardwareCommand = {
                        command: `SET_PWM ${pin} ${value.frequency} ${value.dutyCycle}`,
                        expectResponse: true
                    };
                    break;
                case 'ping':
                    hardwareCommand = {
                        command: 'PING',
                        expectResponse: true
                    };
                    break;
                default:
                    throw new Error(`Unknown hardware command: ${command}`);
            }
            const response = await this.hardwareInterface.sendCommand(hardwareCommand);
            this.sendToClient(ws, {
                type: 'command_response',
                data: {
                    command,
                    pin,
                    value,
                    response,
                    success: response.success
                },
                timestamp: Date.now()
            });
        }
        catch (error) {
            this.sendError(ws, `Hardware command failed: ${error instanceof Error ? error.message : 'Unknown error'}`);
        }
    }
    sendToClient(ws, message) {
        if (ws.readyState === ws_1.WebSocket.OPEN) {
            try {
                ws.send(JSON.stringify(message));
            }
            catch (error) {
                console.error('Failed to send message to WebSocket client:', error);
            }
        }
    }
    sendError(ws, error) {
        this.sendToClient(ws, {
            type: 'error',
            data: { error },
            timestamp: Date.now()
        });
    }
    broadcast(message) {
        const messageStr = JSON.stringify(message);
        this.clients.forEach(client => {
            if (client.readyState === ws_1.WebSocket.OPEN) {
                try {
                    client.send(messageStr);
                }
                catch (error) {
                    console.error('Failed to broadcast to WebSocket client:', error);
                    this.clients.delete(client);
                }
            }
            else {
                this.clients.delete(client);
            }
        });
    }
    getClientCount() {
        return this.clients.size;
    }
}
exports.WebSocketHandler = WebSocketHandler;
