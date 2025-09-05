import { WebSocket } from 'ws';
import { HardwareInterface } from '../adapters/HardwareInterface.js';
export interface WebSocketMessage {
    type: string;
    data: any;
    timestamp: number;
}
export declare class WebSocketHandler {
    private clients;
    private hardwareInterface;
    constructor(hardwareInterface: HardwareInterface);
    private setupHardwareListeners;
    handleConnection(ws: WebSocket): void;
    private sendInitialState;
    private handleMessage;
    private handleHardwareCommand;
    private sendToClient;
    private sendError;
    private broadcast;
    getClientCount(): number;
}
