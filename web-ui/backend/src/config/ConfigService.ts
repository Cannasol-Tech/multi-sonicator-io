/**
 * Configuration Service for Multi-Sonicator-IO Web UI
 * Manages hardware configuration and test capabilities
 */

import * as fs from 'fs';
import * as path from 'path';
import * as yaml from 'js-yaml';

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

export class ConfigService {
  private config: HardwareConfig | null = null;
  private configPath: string;

  constructor(configPath?: string) {
    this.configPath = configPath || path.join(__dirname, '../../config/hardware-config.yaml');
  }

  /**
   * Load hardware configuration from YAML file
   */
  public loadConfig(): HardwareConfig {
    try {
      if (!fs.existsSync(this.configPath)) {
        throw new Error(`Configuration file not found: ${this.configPath}`);
      }

      const configData = fs.readFileSync(this.configPath, 'utf8');
      this.config = yaml.load(configData) as HardwareConfig;

      if (!this.config) {
        throw new Error('Failed to parse configuration file');
      }

      console.log(`✅ Hardware configuration loaded: ${this.config.project.name} v${this.config.project.version}`);
      return this.config;
    } catch (error) {
      console.error('❌ Failed to load hardware configuration:', error);
      throw error;
    }
  }

  /**
   * Get current configuration
   */
  public getConfig(): HardwareConfig {
    if (!this.config) {
      return this.loadConfig();
    }
    return this.config;
  }

  /**
   * Get DUT pin configuration by signal name
   */
  public getDUTPinBySignal(signal: string): { pinName: string; pin: HardwarePin } | null {
    const config = this.getConfig();
    
    for (const [pinName, pinConfig] of Object.entries(config.dut.pins)) {
      if (pinConfig.signal === signal) {
        return { pinName, pin: pinConfig };
      }
    }
    
    return null;
  }

  /**
   * Get harness connection by Arduino pin
   */
  public getHarnessConnection(arduinoPin: string): HarnessConnection | null {
    const config = this.getConfig();
    return config.harness.connections[arduinoPin] || null;
  }

  /**
   * Get sonicator channel configuration
   */
  public getSonicatorChannel(channelId: number): SonicatorChannel | null {
    const config = this.getConfig();
    return config.sonicators.channels.find(ch => ch.id === channelId) || null;
  }

  /**
   * Get all connected sonicator channels
   */
  public getConnectedSonicators(): SonicatorChannel[] {
    const config = this.getConfig();
    return config.sonicators.channels.filter(ch => ch.status === 'connected');
  }

  /**
   * Get supported test capabilities
   */
  public getSupportedCapabilities(): TestCapability[] {
    const config = this.getConfig();
    return config.test_capabilities.filter(cap => cap.supported);
  }

  /**
   * Check if a test capability is supported
   */
  public isCapabilitySupported(capabilityName: string): boolean {
    const config = this.getConfig();
    const capability = config.test_capabilities.find(cap => cap.name === capabilityName);
    return capability?.supported || false;
  }

  /**
   * Get MODBUS configuration
   */
  public getModbusConfig() {
    const config = this.getConfig();
    return config.communication.modbus;
  }

  /**
   * Get serial communication configuration
   */
  public getSerialConfig() {
    const config = this.getConfig();
    return config.communication.serial;
  }

  /**
   * Check if simulation mode is enabled
   */
  public isSimulationMode(): boolean {
    const config = this.getConfig();
    return config.environment.simulation_mode;
  }

  /**
   * Update simulation mode
   */
  public setSimulationMode(enabled: boolean): void {
    const config = this.getConfig();
    config.environment.simulation_mode = enabled;
    console.log(`🔧 Simulation mode ${enabled ? 'enabled' : 'disabled'}`);
  }

  /**
   * Get hardware timeout settings
   */
  public getTimeouts() {
    const config = this.getConfig();
    return {
      hardware: config.environment.hardware_timeout_ms,
      command: config.environment.command_timeout_ms
    };
  }

  /**
   * Get pin mapping for web UI
   * Converts configuration to format expected by existing types
   */
  public getPinMapping(): Record<string, any> {
    const config = this.getConfig();
    const mapping: Record<string, any> = {};

    // Convert DUT pins to web UI format
    for (const [pinName, pinConfig] of Object.entries(config.dut.pins)) {
      mapping[pinConfig.signal] = {
        pin: pinConfig.pin,
        signal: pinConfig.signal,
        direction: pinConfig.direction,
        sonicator: pinConfig.sonicator,
        function: pinConfig.function,
        scaling: pinConfig.scaling
      };
    }

    return mapping;
  }

  /**
   * Validate configuration integrity
   */
  public validateConfig(): { valid: boolean; errors: string[] } {
    const errors: string[] = [];
    
    try {
      const config = this.getConfig();

      // Validate project info
      if (!config.project?.name) {
        errors.push('Missing project name');
      }

      // Validate DUT configuration
      if (!config.dut?.microcontroller?.type) {
        errors.push('Missing DUT microcontroller type');
      }

      if (!config.dut?.pins || Object.keys(config.dut.pins).length === 0) {
        errors.push('Missing DUT pin configuration');
      }

      // Validate harness configuration
      if (!config.harness?.connections || Object.keys(config.harness.connections).length === 0) {
        errors.push('Missing harness connection configuration');
      }

      // Validate sonicator configuration
      if (!config.sonicators?.channels || config.sonicators.channels.length === 0) {
        errors.push('Missing sonicator channel configuration');
      }

      // Validate communication settings
      if (!config.communication?.modbus?.slave_id) {
        errors.push('Missing MODBUS slave ID');
      }

      console.log(errors.length === 0 ? '✅ Configuration validation passed' : `❌ Configuration validation failed: ${errors.length} errors`);
      
      return {
        valid: errors.length === 0,
        errors
      };
    } catch (error) {
      errors.push(`Configuration validation error: ${error}`);
      return { valid: false, errors };
    }
  }

  /**
   * Get configuration summary for status display
   */
  public getConfigSummary() {
    const config = this.getConfig();
    const connectedSonicators = this.getConnectedSonicators();
    const supportedCapabilities = this.getSupportedCapabilities();

    return {
      project: config.project.name,
      version: config.project.version,
      dut: config.dut.microcontroller.type,
      harness: config.harness.type,
      sonicators: {
        total: config.sonicators.count,
        connected: connectedSonicators.length,
        channels: connectedSonicators.map(ch => ch.id)
      },
      capabilities: {
        total: config.test_capabilities.length,
        supported: supportedCapabilities.length,
        names: supportedCapabilities.map(cap => cap.name)
      },
      simulation_mode: config.environment.simulation_mode,
      limitations: config.limitations.current_prototype
    };
  }
}

// Export singleton instance
export const configService = new ConfigService();
