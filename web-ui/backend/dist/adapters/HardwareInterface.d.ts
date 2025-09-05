import { EventEmitter } from 'events';
export interface PinState {
    pin: string;
    signal: string;
    direction: 'IN' | 'OUT' | 'ANALOG';
    state: 'HIGH' | 'LOW' | number;
    timestamp: number;
}
export interface HardwareCommand {
    command: string;
    args?: string[];
    expectResponse?: boolean;
}
export interface HardwareResponse {
    success: boolean;
    data?: any;
    error?: string;
    timestamp: number;
}
/**
 * Hardware Interface Adapter
 * Integrates with existing Python HIL framework for Arduino Test Wrapper communication
 * Based on scripts/hil_serial.py and test/acceptance/hil_framework/hardware_interface.py
 */
export declare class HardwareInterface extends EventEmitter {
    private pythonProcess;
    private connected;
    private serialPort;
    private pinStates;
    private commandQueue;
    private processingCommand;
    constructor();
    private initializePinStates;
    initialize(): Promise<boolean>;
    private handlePythonMessage;
    private updatePinState;
    private startPinMonitoring;
    private sendPythonCommand;
    sendCommand(command: HardwareCommand): Promise<HardwareResponse>;
    getPinStates(): Map<string, PinState>;
    isConnected(): boolean;
    getSerialPort(): string | null;
    private detectHardware;
    reconnect(): Promise<boolean>;
    retryConnection(maxAttempts?: number, delayMs?: number): Promise<boolean>;
    disconnect(): void;
}
