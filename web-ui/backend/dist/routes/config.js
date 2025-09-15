"use strict";
/**
 * Configuration API Routes
 * Provides endpoints for hardware configuration management
 */
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = require("express");
const ConfigService_1 = require("../config/ConfigService");
const router = (0, express_1.Router)();
/**
 * GET /api/config
 * Get complete hardware configuration
 */
router.get('/', (req, res) => {
    try {
        const config = ConfigService_1.configService.getConfig();
        res.json({
            success: true,
            data: config,
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get configuration:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to load configuration',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/summary
 * Get configuration summary for dashboard
 */
router.get('/summary', (req, res) => {
    try {
        const summary = ConfigService_1.configService.getConfigSummary();
        res.json({
            success: true,
            data: summary,
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get configuration summary:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get configuration summary',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/validate
 * Validate configuration integrity
 */
router.get('/validate', (req, res) => {
    try {
        const validation = ConfigService_1.configService.validateConfig();
        res.json({
            success: validation.valid,
            data: validation,
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Configuration validation failed:', error);
        res.status(500).json({
            success: false,
            error: 'Configuration validation failed',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/pins
 * Get pin mapping for web UI
 */
router.get('/pins', (req, res) => {
    try {
        const pinMapping = ConfigService_1.configService.getPinMapping();
        res.json({
            success: true,
            data: pinMapping,
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get pin mapping:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get pin mapping',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/sonicators
 * Get sonicator channel configuration
 */
router.get('/sonicators', (req, res) => {
    try {
        const config = ConfigService_1.configService.getConfig();
        const connectedSonicators = ConfigService_1.configService.getConnectedSonicators();
        res.json({
            success: true,
            data: {
                total: config.sonicators.count,
                channels: config.sonicators.channels,
                connected: connectedSonicators,
                connected_count: connectedSonicators.length
            },
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get sonicator configuration:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get sonicator configuration',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/sonicators/:id
 * Get specific sonicator channel configuration
 */
router.get('/sonicators/:id', (req, res) => {
    try {
        const channelId = parseInt(req.params.id);
        if (isNaN(channelId) || channelId < 1 || channelId > 4) {
            return res.status(400).json({
                success: false,
                error: 'Invalid sonicator channel ID',
                message: 'Channel ID must be between 1 and 4'
            });
        }
        const sonicator = ConfigService_1.configService.getSonicatorChannel(channelId);
        if (!sonicator) {
            return res.status(404).json({
                success: false,
                error: 'Sonicator channel not found',
                message: `Channel ${channelId} not configured`
            });
        }
        res.json({
            success: true,
            data: sonicator,
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get sonicator channel:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get sonicator channel',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/capabilities
 * Get test capabilities
 */
router.get('/capabilities', (req, res) => {
    try {
        const config = ConfigService_1.configService.getConfig();
        const supportedCapabilities = ConfigService_1.configService.getSupportedCapabilities();
        res.json({
            success: true,
            data: {
                all: config.test_capabilities,
                supported: supportedCapabilities,
                supported_count: supportedCapabilities.length
            },
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get test capabilities:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get test capabilities',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/communication
 * Get communication settings
 */
router.get('/communication', (req, res) => {
    try {
        const modbusConfig = ConfigService_1.configService.getModbusConfig();
        const serialConfig = ConfigService_1.configService.getSerialConfig();
        res.json({
            success: true,
            data: {
                modbus: modbusConfig,
                serial: serialConfig
            },
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get communication configuration:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get communication configuration',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * POST /api/config/simulation
 * Toggle simulation mode
 */
router.post('/simulation', (req, res) => {
    try {
        const { enabled } = req.body;
        if (typeof enabled !== 'boolean') {
            return res.status(400).json({
                success: false,
                error: 'Invalid request',
                message: 'enabled field must be a boolean'
            });
        }
        ConfigService_1.configService.setSimulationMode(enabled);
        res.json({
            success: true,
            data: {
                simulation_mode: enabled,
                message: `Simulation mode ${enabled ? 'enabled' : 'disabled'}`
            },
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to set simulation mode:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to set simulation mode',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * GET /api/config/limitations
 * Get current hardware limitations
 */
router.get('/limitations', (req, res) => {
    try {
        const config = ConfigService_1.configService.getConfig();
        res.json({
            success: true,
            data: config.limitations,
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to get limitations:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get limitations',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
/**
 * POST /api/config/reload
 * Reload configuration from file
 */
router.post('/reload', (req, res) => {
    try {
        const config = ConfigService_1.configService.loadConfig();
        const summary = ConfigService_1.configService.getConfigSummary();
        res.json({
            success: true,
            data: {
                message: 'Configuration reloaded successfully',
                summary: summary
            },
            timestamp: Date.now()
        });
    }
    catch (error) {
        console.error('Failed to reload configuration:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to reload configuration',
            message: error instanceof Error ? error.message : 'Unknown error'
        });
    }
});
exports.default = router;
