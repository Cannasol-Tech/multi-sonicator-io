/**
 * Configuration Service for Multi-Sonicator-IO Web UI
 * Manages hardware configuration and test capabilities
 */
export interface HardwarePin {
    pin: number;
    signal: string;
    direction: 'IN' | 'OUT' | 'ANALOG';
    sonicator?: number;
    function?: string;
    scaling?: string;
}
export interface HarnessConnection {
    dut_pin: string;
    signal: string;
    direction: 'harness_to_dut' | 'dut_to_harness' | 'bidirectional';
    sonicator?: number;
}
export interface SonicatorChannel {
    id: number;
    name: string;
    db9_connector: string;
    power_range: [number, number];
    frequency_range: [number, number];
    amplitude_range: [number, number];
    status: 'connected' | 'not_connected' | 'error';
}
export interface TestCapability {
    name: string;
    description: string;
    pins: string[];
    supported: boolean;
    note?: string;
}
export interface HardwareConfig {
    project: {
        name: string;
        description: string;
        version: string;
    };
    dut: {
        microcontroller: {
            type: string;
            package: string;
            clock_frequency: number;
            voltage: number;
            manufacturer: string;
        };
        pins: Record<string, HardwarePin>;
    };
    harness: {
        type: string;
        microcontroller: string;
        clock_frequency: number;
        voltage: number;
        connections: Record<string, HarnessConnection>;
    };
    sonicators: {
        count: number;
        channels: SonicatorChannel[];
    };
    communication: {
        modbus: {
            slave_id: number;
            baud_rate: number;
            data_bits: number;
            stop_bits: number;
            parity: string;
            timeout_ms: number;
        };
        serial: {
            port: string;
            baud_rate: number;
            timeout_ms: number;
        };
    };
    test_capabilities: TestCapability[];
    limitations: {
        current_prototype: string;
        supported_channels: number[];
        unsupported_channels: number[];
        expansion_notes: string;
    };
    environment: {
        simulation_mode: boolean;
        hardware_timeout_ms: number;
        command_timeout_ms: number;
        log_level: string;
    };
}
export declare class ConfigService {
    private config;
    private configPath;
    constructor(configPath?: string);
    /**
     * Load hardware configuration from YAML file
     */
    loadConfig(): HardwareConfig;
    /**
     * Get current configuration
     */
    getConfig(): HardwareConfig;
    /**
     * Get DUT pin configuration by signal name
     */
    getDUTPinBySignal(signal: string): {
        pinName: string;
        pin: HardwarePin;
    } | null;
    /**
     * Get harness connection by Arduino pin
     */
    getHarnessConnection(arduinoPin: string): HarnessConnection | null;
    /**
     * Get sonicator channel configuration
     */
    getSonicatorChannel(channelId: number): SonicatorChannel | null;
    /**
     * Get all connected sonicator channels
     */
    getConnectedSonicators(): SonicatorChannel[];
    /**
     * Get supported test capabilities
     */
    getSupportedCapabilities(): TestCapability[];
    /**
     * Check if a test capability is supported
     */
    isCapabilitySupported(capabilityName: string): boolean;
    /**
     * Get MODBUS configuration
     */
    getModbusConfig(): {
        slave_id: number;
        baud_rate: number;
        data_bits: number;
        stop_bits: number;
        parity: string;
        timeout_ms: number;
    };
    /**
     * Get serial communication configuration
     */
    getSerialConfig(): {
        port: string;
        baud_rate: number;
        timeout_ms: number;
    };
    /**
     * Check if simulation mode is enabled
     */
    isSimulationMode(): boolean;
    /**
     * Update simulation mode
     */
    setSimulationMode(enabled: boolean): void;
    /**
     * Get hardware timeout settings
     */
    getTimeouts(): {
        hardware: number;
        command: number;
    };
    /**
     * Get pin mapping for web UI
     * Converts configuration to format expected by existing types
     */
    getPinMapping(): Record<string, any>;
    /**
     * Validate configuration integrity
     */
    validateConfig(): {
        valid: boolean;
        errors: string[];
    };
    /**
     * Get configuration summary for status display
     */
    getConfigSummary(): {
        project: string;
        version: string;
        dut: string;
        harness: string;
        sonicators: {
            total: number;
            connected: number;
            channels: number[];
        };
        capabilities: {
            total: number;
            supported: number;
            names: string[];
        };
        simulation_mode: boolean;
        limitations: string;
    };
}
export declare const configService: ConfigService;
