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

/**
 * @class HardwareInterface
 * @classdesc Adapter for communicating with the hardware via the Arduino Test Wrapper
 * Based on scripts/hil_serial.py and test/acceptance/hil_framework/hardware_interface.py
 * @emits 'pinStateChange' when a pin state changes
 * @emits 'connectionChange' when the connection status changes
 * @emits 'commandResponse' when a command response is received
 * @emits 'error' when an error occurs
 * @emits 'arduinoCommand' when a command is sent to the Arduino
 * @emits 'arduinoLog' when a log message is received from the Arduino
 * @emits 'arduinoResponse' when a response is received from the Arduino
 * @emits 'arduinoError' when an error is received from the Arduino
 * @emits 'arduinoStatus' when a status update is received from the Arduino
 * @emits 'arduinoPinState' when a pin state update is received from the Arduino
 * @emits 'arduinoConnection' when a connection status update is received from the Arduino
 * @emits 'arduinoPinUpdate' when a pin state update is received from the Arduino
 * @emits 'arduinoPinMonitoring' when a pin monitoring status update is received from the Arduino
 * @emits 'arduinoPinMonitoringChange' when a pin monitoring status change is received from the Arduino
 * @emits 'arduinoPinMonitoringStart' when a pin monitoring start is received from the Arduino
 */
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
    ping(): Promise<{
        success: boolean;
        data?: any;
        error?: string;
        responseTime?: number;
    }>;
    getInfo(): Promise<{
        success: boolean;
        data?: any;
        error?: string;
    }>;
    getStatus(sonicator?: number): Promise<{
        success: boolean;
        data?: any;
        error?: string;
    }>;
    isConnected(): boolean;
    getPortPath(): string | null;
}
