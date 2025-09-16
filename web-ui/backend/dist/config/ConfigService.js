"use strict";
/**
 * Configuration Service for Multi-Sonicator-IO Web UI
 * Manages hardware configuration and test capabilities
 */
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.configService = exports.ConfigService = void 0;
const fs = __importStar(require("fs"));
const path = __importStar(require("path"));
const yaml = __importStar(require("js-yaml"));
class ConfigService {
    constructor(configPath) {
        this.config = null;
        // Priority: explicit arg > ENV var > centralized repo path > legacy backend-local path
        const envPath = process.env.HARDWARE_CONFIG_PATH;
        const centralizedPath = path.join(__dirname, '../../../../config/hardware-config.yaml');
        const legacyBackendLocal = path.join(__dirname, '../../config/hardware-config.yaml');
        this.configPath = configPath || envPath || centralizedPath;
        // Backward compatibility: if centralized path missing but legacy exists, use it
        if (!fs.existsSync(this.configPath) && fs.existsSync(legacyBackendLocal)) {
            this.configPath = legacyBackendLocal;
        }
    }
    /**
     * Load hardware configuration from YAML file
     */
    loadConfig() {
        try {
            if (!fs.existsSync(this.configPath)) {
                throw new Error(`Configuration file not found: ${this.configPath}`);
            }
            const configData = fs.readFileSync(this.configPath, 'utf8');
            this.config = yaml.load(configData);
            if (!this.config) {
                throw new Error('Failed to parse configuration file');
            }
            // Override simulation mode based on HARDWARE_PRESENT environment variable
            // HARDWARE_PRESENT=true means hardware mode (simulation_mode=false)
            // HARDWARE_PRESENT=false means simulation mode (simulation_mode=true)
            const hardwarePresent = process.env.HARDWARE_PRESENT;
            if (hardwarePresent !== undefined) {
                const isHardwarePresent = hardwarePresent.toLowerCase() === 'true';
                this.config.environment.simulation_mode = !isHardwarePresent;
                console.log(`🔧 HARDWARE_PRESENT=${hardwarePresent} -> Simulation mode: ${this.config.environment.simulation_mode ? 'enabled' : 'disabled'}`);
            }
            console.log(`✅ Hardware configuration loaded: ${this.config.project.name} v${this.config.project.version}`);
            return this.config;
        }
        catch (error) {
            console.error('❌ Failed to load hardware configuration:', error);
            throw error;
        }
    }
    /**
     * Get current configuration
     */
    getConfig() {
        if (!this.config) {
            return this.loadConfig();
        }
        return this.config;
    }
    /**
     * Get DUT pin configuration by signal name
     */
    getDUTPinBySignal(signal) {
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
    getHarnessConnection(arduinoPin) {
        const config = this.getConfig();
        return config.harness.connections[arduinoPin] || null;
    }
    /**
     * Get sonicator channel configuration
     */
    getSonicatorChannel(channelId) {
        const config = this.getConfig();
        return config.sonicators.channels.find(ch => ch.id === channelId) || null;
    }
    /**
     * Get all connected sonicator channels
     */
    getConnectedSonicators() {
        const config = this.getConfig();
        return config.sonicators.channels.filter(ch => ch.status === 'connected');
    }
    /**
     * Get supported test capabilities
     */
    getSupportedCapabilities() {
        const config = this.getConfig();
        return config.test_capabilities.filter(cap => cap.supported);
    }
    /**
     * Check if a test capability is supported
     */
    isCapabilitySupported(capabilityName) {
        const config = this.getConfig();
        const capability = config.test_capabilities.find(cap => cap.name === capabilityName);
        return capability?.supported || false;
    }
    /**
     * Get MODBUS configuration
     */
    getModbusConfig() {
        const config = this.getConfig();
        return config.communication.modbus;
    }
    /**
     * Get serial communication configuration
     */
    getSerialConfig() {
        const config = this.getConfig();
        return config.communication.serial;
    }
    /**
     * Check if simulation mode is enabled
     */
    isSimulationMode() {
        const config = this.getConfig();
        return config.environment.simulation_mode;
    }
    /**
     * Update simulation mode
     */
    setSimulationMode(enabled) {
        const config = this.getConfig();
        config.environment.simulation_mode = enabled;
        console.log(`🔧 Simulation mode ${enabled ? 'enabled' : 'disabled'}`);
    }
    /**
     * Get hardware timeout settings
     */
    getTimeouts() {
        const config = this.getConfig();
        return {
            hardware: config.environment.hardware_timeout_ms,
            command: config.environment.command_timeout_ms
        };
    }
    /**
     * Get configuration summary for status display
     */
    getConfigSummary() {
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
    /**
     * Get pin mapping for web UI
     * Converts configuration to format expected by existing types
     */
    getPinMapping() {
        const config = this.getConfig();
        const mapping = {};
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
    validateConfig() {
        const errors = [];
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
        }
        catch (error) {
            errors.push(`Configuration validation error: ${error}`);
            return { valid: false, errors };
        }
    }
    /**
     * Save configuration to YAML file
     */
    saveConfig(config) {
        try {
            // Validate the configuration before saving
            const validation = this.validateConfig();
            if (!validation.valid) {
                throw new Error(`Configuration validation failed: ${validation.errors.join(', ')}`);
            }
            // Convert to YAML and save
            const yamlContent = yaml.dump(config, {
                indent: 2,
                lineWidth: -1,
                noRefs: true
            });
            fs.writeFileSync(this.configPath, yamlContent, 'utf8');
            // Update the cached config
            this.config = config;
            console.log(`✅ Configuration saved to: ${this.configPath}`);
        }
        catch (error) {
            console.error('❌ Failed to save configuration:', error);
            throw error;
        }
    }
}
exports.ConfigService = ConfigService;
// Export singleton instance
exports.configService = new ConfigService();
