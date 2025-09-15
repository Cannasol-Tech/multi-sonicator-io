"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.WebSocketHandler = void 0;
const ws_1 = require("ws");

/**
 * WebSocket transport for broadcasting hardware and test-automation events
 * and handling incoming UI commands.
 *
 * This module exposes a single class, {@link WebSocketHandler}, that bridges
 * the server-side domain (hardware interface + test automation service) with
 * browser clients connected over WebSockets. It fans out status/events to all
 * clients and routes client requests back to the hardware layer.
 *
 * Design goals:
 * - Minimal overhead: pre-stringify payloads for broadcast and reuse buffers.
 * - Clear message schema: each message has `type`, `data`, and `timestamp`.
 * - Robustness: input validation and graceful error reporting to clients.
 *
 * Message envelope:
 *   {
 *     type: string,
 *     data: any,
 *     timestamp: number // ms since epoch (Date.now())
 *   }
 *
 * Known outbound `type` values include:
 * - `connection_status`, `pin_update`, `error`
 * - `arduino_command_sent`, `arduino_command_response`
 * - `test_progress`, `test_complete`, `test_error`, `test_stopped`
 * - `initial_state`, `pong`, `pin_states`, `command_response`
 *
 * Known inbound `type` values include:
 * - `hardware_command` | `ping` | `get_pin_states`
 *
 * Note: This file is a compiled JS artifact that intentionally carries rich
 * JSDoc so downstream documentation generators can consume it.
 * @module websocket/WebSocketHandler
 */
/**
 * Orchestrates WebSocket client connections and message routing.
 *
 * @example
 * const wss = new WebSocketServer({ server });
 * const handler = new WebSocketHandler(hardwareInterface, testService);
 * wss.on('connection', (ws) => handler.handleConnection(ws));
 */
class WebSocketHandler {
    /**
     * @param {object} hardwareInterface - Hardware adapter that emits events and accepts commands.
     * @param {function} hardwareInterface.on - Event emitter interface.
     * @param {function} hardwareInterface.sendCommand - Sends a low-level command and resolves with a response.
     * @param {function} hardwareInterface.isConnected - Returns boolean connection state.
     * @param {function} hardwareInterface.getSerialPort - Returns current serial port identifier.
     * @param {function} hardwareInterface.getPinStates - Returns iterable of [signal, state].
     * @param {object} [testAutomationService] - Optional test execution service emitting progress/completion events.
     */
    constructor(hardwareInterface, testAutomationService) {
        this.clients = new Set();
        this.pendingCommands = new Map();
        this.hardwareInterface = hardwareInterface;
        this.testAutomationService = testAutomationService;
        this.setupHardwareListeners();
        this.setupTestAutomationListeners();
    }
    /**
     * Subscribes to hardware layer events and broadcasts them to clients.
     * Pairs sent Arduino commands with received responses when possible.
     * @private
     */
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
        // Handle Arduino command logging for proper command/response pairing
        this.hardwareInterface.on('arduino_command', (commandData) => {
            if (commandData.direction === 'sent') {
                // Store sent command for pairing with response
                const commandId = `${commandData.timestamp}-${commandData.type}`;
                this.pendingCommands.set(commandId, {
                    command: commandData.data,
                    timestamp: commandData.timestamp
                });
                // Broadcast arduino_command_sent message
                this.broadcast({
                    type: 'arduino_command_sent',
                    data: {
                        command: commandData.data,
                        timestamp: commandData.timestamp,
                        type: commandData.type
                    },
                    timestamp: Date.now()
                });
            }
            else if (commandData.direction === 'received') {
                // Find matching sent command and create response pair
                let matchedCommand = null;
                let responseTime = 0;
                // Look for most recent matching command (simple approach)
                const recentCommands = Array.from(this.pendingCommands.entries())
                    .sort((a, b) => b[1].timestamp - a[1].timestamp);
                if (recentCommands.length > 0) {
                    const [commandId, commandInfo] = recentCommands[0];
                    matchedCommand = commandInfo.command;
                    responseTime = commandData.timestamp - commandInfo.timestamp;
                    this.pendingCommands.delete(commandId);
                }
                // Broadcast arduino_command_response message
                this.broadcast({
                    type: 'arduino_command_response',
                    data: {
                        command: matchedCommand || 'Unknown command',
                        response: commandData.data,
                        responseTime: Math.max(responseTime, 0),
                        success: !commandData.data.includes('ERROR') && !commandData.data.includes('FAIL'),
                        error: commandData.data.includes('ERROR') || commandData.data.includes('FAIL') ? commandData.data : undefined,
                        timestamp: commandData.timestamp
                    },
                    timestamp: Date.now()
                });
            }
        });
    }
    /**
     * Subscribes to test automation events for real-time UI updates.
     * No-ops when service is not provided.
     * @private
     */
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
    /**
     * Handles a new WebSocket client connection.
     * Sends initial state and wires message/close/error handlers.
     * @param {import('ws').WebSocket} ws - Client socket connection.
     */
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
    /**
     * Sends current connection status, pin states, and test execution snapshot to one client.
     * @param {import('ws').WebSocket} ws - Target client socket.
     */
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
    /**
     * Routes an inbound client message based on its `type`.
     * @param {import('ws').WebSocket} ws - Source client.
     * @param {{type:string,data?:any,timestamp?:number}} message - Parsed message envelope.
     * @returns {Promise<void>}
     */
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
    /**
     * Translates a high-level hardware command from the UI into the
     * low-level protocol understood by the hardware interface.
     * Responds to the caller with a `command_response` message.
     *
     * @param {import('ws').WebSocket} ws - Requesting client.
     * @param {{command:string,pin?:number|string,value?:any}} commandData - Command payload.
     * @returns {Promise<void>}
     */
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
    /**
     * Sends a message to a single WebSocket client if the socket is open.
     * @param {import('ws').WebSocket} ws - Target client.
     * @param {{type:string,data?:any,timestamp:number}} message - Message envelope.
     */
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
    /**
     * Sends a standardized error envelope to a single client.
     * @param {import('ws').WebSocket} ws - Target client.
     * @param {string} error - Human-readable error message.
     */
    sendError(ws, error) {
        this.sendToClient(ws, {
            type: 'error',
            data: { error },
            timestamp: Date.now()
        });
    }
    /**
     * Broadcasts a pre-structured message to all connected clients.
     * Uses pre-serialization for efficiency.
     * @param {{type:string,data?:any,timestamp:number}} message - Message envelope.
     */
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
    /**
     * Returns the number of currently connected WebSocket clients.
     * @returns {number}
     */
    getClientCount() {
        return this.clients.size;
    }
}
exports.WebSocketHandler = WebSocketHandler;
