import { EventEmitter } from 'events';
export interface PinState {
    pin: string;
    signal: string;
    direction: 'IN' | 'OUT' | 'ANALOG';
    state: 'HIGH' | 'LOW' | number | string;
    timestamp: number;
    frequency?: number;
    frequencyDisplay?: string;
    operatingFrequency?: string;
    enabled?: boolean;
    isActive?: boolean;
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
    private configuration;
    constructor();
    private initializePinStates;
    initialize(): Promise<boolean>;
    private handlePythonMessage;
    private lastLogTime;
    private logThrottleMs;
    private lastPinLogTimes;
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
    getConfiguration(): {
        sonicator4: {
            operatingFrequencyKHz: number;
            outputFrequencyKHz: number;
            enabled: boolean;
            manualMode: boolean;
            manualFrequencyKHz: number;
        };
        testHarness: {
            pinMapping: {
                FREQ_DIV10_4: string;
                FREQ_LOCK_4: string;
                OVERLOAD_4: string;
                START_4: string;
                RESET_4: string;
                POWER_SENSE_4: string;
                AMPLITUDE_ALL: string;
                UART_TXD: string;
                STATUS_LED: string;
            };
        };
    };
    updateConfiguration(newConfig: any): Promise<{
        success: boolean;
        config: any;
    }>;
}
