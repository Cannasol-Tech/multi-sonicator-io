"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const http_1 = require("http");
const ws_1 = require("ws");
const cors_1 = __importDefault(require("cors"));
const HardwareInterface_1 = require("./adapters/HardwareInterface");
const WebSocketHandler_1 = require("./websocket/WebSocketHandler");
const TestAutomationService_1 = require("./services/TestAutomationService");
const index_1 = require("./routes/index");
const ConfigService_1 = require("./config/ConfigService");
const app = (0, express_1.default)();
const server = (0, http_1.createServer)(app);
const wss = new ws_1.WebSocketServer({ server, path: '/ws' });
// Middleware
app.use((0, cors_1.default)({
    origin: process.env.NODE_ENV === 'production' ? false : 'http://localhost:3000',
    credentials: true
}));
app.use(express_1.default.json());
// Initialize configuration service
console.log('🔧 Loading hardware configuration...');
try {
    ConfigService_1.configService.loadConfig();
    const summary = ConfigService_1.configService.getConfigSummary();
    console.log(`✅ Configuration loaded: ${summary.project} v${summary.version}`);
    console.log(`🔌 Sonicators: ${summary.sonicators.connected}/${summary.sonicators.total} connected`);
    console.log(`🧪 Test capabilities: ${summary.capabilities.supported}/${summary.capabilities.total} supported`);
    console.log(`🔧 Simulation mode: ${summary.simulation_mode ? 'enabled' : 'disabled'}`);
}
catch (error) {
    console.error('❌ Failed to load configuration:', error);
    console.log('⚠️  Continuing with default hardware interface...');
}
// Initialize hardware interface
const hardwareInterface = new HardwareInterface_1.HardwareInterface();
// Initialize test automation service
const testAutomationService = new TestAutomationService_1.TestAutomationService();
// Setup routes
(0, index_1.setupRoutes)(app, hardwareInterface, testAutomationService);
// Setup WebSocket handling
const wsHandler = new WebSocketHandler_1.WebSocketHandler(hardwareInterface, testAutomationService);
wss.on('connection', (ws, request) => {
    console.log('New WebSocket connection from:', request.socket.remoteAddress);
    wsHandler.handleConnection(ws);
});
// Error handling
app.use((err, req, res, next) => {
    console.error('Express error:', err);
    res.status(500).json({
        error: 'Internal server error',
        message: process.env.NODE_ENV === 'development' ? err.message : 'Something went wrong'
    });
});
// Start server
const PORT = process.env.PORT || 3001;
server.listen(PORT, () => {
    console.log(`Multi-Sonicator-IO Backend Server running on port ${PORT}`);
    console.log(`WebSocket endpoint: ws://localhost:${PORT}/ws`);
    console.log(`HTTP API endpoint: http://localhost:${PORT}/api`);
    // Initialize hardware connection
    hardwareInterface.initialize().catch(err => {
        console.error('Failed to initialize hardware interface:', err);
    });
});
// Graceful shutdown
process.on('SIGTERM', () => {
    console.log('SIGTERM received, shutting down gracefully');
    server.close(() => {
        hardwareInterface.disconnect();
        process.exit(0);
    });
});
process.on('SIGINT', () => {
    console.log('SIGINT received, shutting down gracefully');
    server.close(() => {
        hardwareInterface.disconnect();
        process.exit(0);
    });
});
