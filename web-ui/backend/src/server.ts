import fs from 'fs'
import path from 'path'
import dotenv from 'dotenv'
import express from 'express'
import { createServer } from 'http'
import { WebSocketServer } from 'ws'
import cors from 'cors'
import { HardwareInterface } from './adapters/HardwareInterface'
import { WebSocketHandler } from './websocket/WebSocketHandler'
import { TestAutomationService } from './services/TestAutomationService'
import { setupRoutes } from './routes/index'
import { configService } from './config/ConfigService'

// Load .env from backend and repository root (root overrides backend)
// Works for both ts-node (src) and compiled (dist) because we resolve from __dirname
const rootEnvPath = path.resolve(__dirname, '../../../.env')
const backendEnvPath = path.resolve(__dirname, '../.env')
// Precedence: runtime env > root .env > backend .env
// Load root first (fills missing), then backend (fills remaining); neither overrides runtime env
dotenv.config({ path: rootEnvPath })
dotenv.config({ path: backendEnvPath })

const app = express()
const server = createServer(app)
const wss = new WebSocketServer({ server, path: '/ws' })

// Middleware
app.use(cors({
  origin: process.env.NODE_ENV === 'production' ? false : 'http://localhost:3000',
  credentials: true
}))
app.use(express.json())

// If HARDWARE_PRESENT=false, do not start the server/UI
if (process.env.HARDWARE_PRESENT && process.env.HARDWARE_PRESENT.toLowerCase() === 'false') {
  console.error('❌ HARDWARE_PRESENT=false: Web UI backend disabled by configuration')
  process.exit(1)
}

// Initialize configuration service
console.log('🔧 Loading hardware configuration...')
try {
  configService.loadConfig()
  const summary = configService.getConfigSummary()
  console.log(`✅ Configuration loaded: ${summary.project} v${summary.version}`)
  console.log(`🔌 Sonicators: ${summary.sonicators.connected}/${summary.sonicators.total} connected`)
  console.log(`🧪 Test capabilities: ${summary.capabilities.supported}/${summary.capabilities.total} supported`)
  console.log(`🔧 Simulation mode: ${summary.simulation_mode ? 'enabled' : 'disabled'}`)
} catch (error) {
  console.error('❌ Failed to load configuration:', error)
  console.log('⚠️  Continuing with default hardware interface...')
}

// Initialize hardware interface
const hardwareInterface = new HardwareInterface()

// Initialize test automation service
const testAutomationService = new TestAutomationService()

// Setup routes
setupRoutes(app, hardwareInterface, testAutomationService)

// Setup WebSocket handling
const wsHandler = new WebSocketHandler(hardwareInterface, testAutomationService)
wss.on('connection', (ws, request) => {
  console.log('New WebSocket connection from:', request.socket.remoteAddress)
  wsHandler.handleConnection(ws)
})

// Error handling
app.use((err: any, req: express.Request, res: express.Response, next: express.NextFunction) => {
  console.error('Express error:', err)
  res.status(500).json({ 
    error: 'Internal server error',
    message: process.env.NODE_ENV === 'development' ? err.message : 'Something went wrong'
  })
})

// Start server
const PORT = process.env.PORT || 3001
server.listen(PORT, () => {
  console.log(`Multi-Sonicator-IO Backend Server running on port ${PORT}`)
  console.log(`WebSocket endpoint: ws://localhost:${PORT}/ws`)
  console.log(`HTTP API endpoint: http://localhost:${PORT}/api`)
  
  // Initialize hardware connection
  hardwareInterface.initialize().catch(err => {
    console.error('Failed to initialize hardware interface:', err)
  })
})

// Graceful shutdown
process.on('SIGTERM', () => {
  console.log('SIGTERM received, shutting down gracefully')
  server.close(() => {
    hardwareInterface.disconnect()
    process.exit(0)
  })
})

process.on('SIGINT', () => {
  console.log('SIGINT received, shutting down gracefully')
  server.close(() => {
    hardwareInterface.disconnect()
    process.exit(0)
  })
})
