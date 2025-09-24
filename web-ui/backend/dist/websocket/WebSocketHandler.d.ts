import { WebSocket } from 'ws';
import { HardwareInterface } from '../adapters/HardwareInterface.js';
import { TestAutomationService } from '../services/TestAutomationService.js';
export interface WebSocketMessage {
    type: string;
    data: any;
    timestamp: number;
}
export declare class WebSocketHandler {
    private clients;
    private hardwareInterface;
    private testAutomationService?;
    private pendingCommands;
    constructor(hardwareInterface: HardwareInterface, testAutomationService?: TestAutomationService);
    private setupHardwareListeners;
    private setupTestAutomationListeners;
    private setupConfigurationListeners;
    handleConnection(ws: WebSocket): void;
    private sendInitialState;
    private handleMessage;
    private handleHardwareCommand;
    private sendToClient;
    private sendError;
    private broadcast;
    getClientCount(): number;
}
