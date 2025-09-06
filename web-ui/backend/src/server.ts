import express from 'express'
import { createServer } from 'http'
import { WebSocketServer } from 'ws'
import cors from 'cors'
import { HardwareInterface } from './adapters/HardwareInterface'
import { WebSocketHandler } from './websocket/WebSocketHandler'
import { TestAutomationService } from './services/TestAutomationService'
import { setupRoutes } from './routes/index'

const app = express()
const server = createServer(app)
const wss = new WebSocketServer({ server, path: '/ws' })

// Middleware
app.use(cors({
  origin: process.env.NODE_ENV === 'production' ? false : 'http://localhost:3000',
  credentials: true
}))
app.use(express.json())

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
