import { Express, Request, Response } from 'express'
import { HardwareInterface } from '../adapters/HardwareInterface.js'

export function setupRoutes(app: Express, hardwareInterface: HardwareInterface) {
  // Health check endpoint
  app.get('/api/health', (req: Request, res: Response) => {
    res.json({
      status: 'ok',
      timestamp: Date.now(),
      hardware: {
        connected: hardwareInterface.isConnected(),
        port: hardwareInterface.getSerialPort()
      }
    })
  })

  // Get current pin states
  app.get('/api/pins', (req: Request, res: Response) => {
    const pinStates = Object.fromEntries(hardwareInterface.getPinStates())
    res.json({
      pins: pinStates,
      timestamp: Date.now()
    })
  })

  // Get specific pin state
  app.get('/api/pins/:signal', (req: Request, res: Response) => {
    const { signal } = req.params
    const pinStates = hardwareInterface.getPinStates()
    const pinState = pinStates.get(signal)
    
    if (!pinState) {
      return res.status(404).json({
        error: `Pin signal '${signal}' not found`,
        availablePins: Array.from(pinStates.keys())
      })
    }
    
    res.json({
      pin: pinState,
      timestamp: Date.now()
    })
  })

  // Send hardware command
  app.post('/api/command', async (req: Request, res: Response) => {
    try {
      const { command, args } = req.body
      
      if (!command) {
        return res.status(400).json({
          error: 'Command is required'
        })
      }

      const response = await hardwareInterface.sendCommand({
        command,
        args,
        expectResponse: true
      })

      res.json({
        command,
        args,
        response,
        timestamp: Date.now()
      })

    } catch (error) {
      res.status(500).json({
        error: 'Failed to execute command',
        message: error instanceof Error ? error.message : 'Unknown error',
        timestamp: Date.now()
      })
    }
  })

  // Get connection status
  app.get('/api/connection', (req: Request, res: Response) => {
    res.json({
      connected: hardwareInterface.isConnected(),
      port: hardwareInterface.getSerialPort(),
      timestamp: Date.now()
    })
  })

  // Reconnect to hardware
  app.post('/api/connection/reconnect', async (req: Request, res: Response) => {
    try {
      console.log('Manual reconnection requested')
      const success = await (hardwareInterface as any).reconnect()

      res.json({
        success,
        connected: hardwareInterface.isConnected(),
        port: hardwareInterface.getSerialPort(),
        timestamp: Date.now()
      })
    } catch (error) {
      res.status(500).json({
        success: false,
        error: 'Reconnection failed',
        message: error instanceof Error ? error.message : 'Unknown error',
        timestamp: Date.now()
      })
    }
  })

  // Retry connection with multiple attempts
  app.post('/api/connection/retry', async (req: Request, res: Response) => {
    try {
      const { maxAttempts = 3, delayMs = 2000 } = req.body
      console.log(`Connection retry requested: ${maxAttempts} attempts`)

      const success = await (hardwareInterface as any).retryConnection(maxAttempts, delayMs)

      res.json({
        success,
        connected: hardwareInterface.isConnected(),
        port: hardwareInterface.getSerialPort(),
        attempts: maxAttempts,
        timestamp: Date.now()
      })
    } catch (error) {
      res.status(500).json({
        success: false,
        error: 'Connection retry failed',
        message: error instanceof Error ? error.message : 'Unknown error',
        timestamp: Date.now()
      })
    }
  })

  // API documentation endpoint
  app.get('/api', (req: Request, res: Response) => {
    res.json({
      name: 'Multi-Sonicator-IO Web UI API',
      version: '1.0.0',
      description: 'REST API for Arduino Test Harness - ATmega32A DUT control',
      endpoints: {
        'GET /api/health': 'Health check and system status',
        'GET /api/pins': 'Get all pin states',
        'GET /api/pins/:signal': 'Get specific pin state by signal name',
        'POST /api/command': 'Send hardware command',
        'GET /api/connection': 'Get hardware connection status'
      },
      websocket: 'ws://localhost:3001/ws',
      pinMapping: 'Based on docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)'
    })
  })


}
