import { WebSocket } from 'ws'
import { HardwareInterface } from '../adapters/HardwareInterface.js'
import { TestAutomationService } from '../services/TestAutomationService.js'
import { modularConfigService } from '../config/ModularConfigService'

export interface WebSocketMessage {
  type: string
  data: any
  timestamp: number
}

export class WebSocketHandler {
  private clients: Set<WebSocket> = new Set()
  private hardwareInterface: HardwareInterface
  private testAutomationService?: TestAutomationService
  private pendingCommands: Map<string, { command: string; timestamp: number }> = new Map()

  constructor(hardwareInterface: HardwareInterface, testAutomationService?: TestAutomationService) {
    this.hardwareInterface = hardwareInterface
    this.testAutomationService = testAutomationService
    this.setupHardwareListeners()
    this.setupTestAutomationListeners()
    this.setupConfigurationListeners()
  }

  private setupHardwareListeners() {
    // Listen for hardware events and broadcast to all connected clients
    this.hardwareInterface.on('connected', (data) => {
      this.broadcast({
        type: 'connection_status',
        data: { connected: true, ...data },
        timestamp: Date.now()
      })
    })

    this.hardwareInterface.on('disconnected', () => {
      this.broadcast({
        type: 'connection_status', 
        data: { connected: false },
        timestamp: Date.now()
      })
    })

    this.hardwareInterface.on('pin_update', (signal, pinState) => {
      this.broadcast({
        type: 'pin_update',
        data: { signal, pinState },
        timestamp: Date.now()
      })
    })

    this.hardwareInterface.on('error', (error) => {
      this.broadcast({
        type: 'error',
        data: { error },
        timestamp: Date.now()
      })
    })

    // Handle Arduino command logging for proper command/response pairing
    this.hardwareInterface.on('arduino_command', (commandData) => {
      if (commandData.direction === 'sent') {
        // Store sent command for pairing with response
        const commandId = `${commandData.timestamp}-${commandData.type}`
        this.pendingCommands.set(commandId, {
          command: commandData.data,
          timestamp: commandData.timestamp
        })
        
        // Broadcast arduino_command_sent message
        this.broadcast({
          type: 'arduino_command_sent',
          data: {
            command: commandData.data,
            timestamp: commandData.timestamp,
            type: commandData.type
          },
          timestamp: Date.now()
        })
      } else if (commandData.direction === 'received') {
        // Find matching sent command and create response pair
        let matchedCommand = null
        let responseTime = 0

        // Enhanced matching algorithm: prefer same-type command, then fallback to most recent
        let matchedEntry: { commandId: string; commandInfo: { command: string; timestamp: number } } | null = null

        const commandType = commandData.type
        const sameTypeCommands = Array.from(this.pendingCommands.entries())
          .filter(([id]) => id.endsWith(`-${commandType}`))
          .sort((a, b) => b[1].timestamp - a[1].timestamp)

        if (sameTypeCommands.length > 0) {
          const [commandId, commandInfo] = sameTypeCommands[0]
          matchedEntry = { commandId, commandInfo }
        } else {
          const recentCommands = Array.from(this.pendingCommands.entries())
            .sort((a, b) => b[1].timestamp - a[1].timestamp)
          if (recentCommands.length > 0) {
            const [commandId, commandInfo] = recentCommands[0]
            matchedEntry = { commandId, commandInfo }
          }
        }

        if (matchedEntry) {
          matchedCommand = matchedEntry.commandInfo.command
          responseTime = commandData.timestamp - matchedEntry.commandInfo.timestamp
          this.pendingCommands.delete(matchedEntry.commandId)
        }

        // Broadcast arduino_command_response message
        const dataStr: string = typeof commandData.data === 'string' ? commandData.data : JSON.stringify(commandData.data)
        const dataLower = dataStr.toLowerCase()
        this.broadcast({
          type: 'arduino_command_response',
          data: {
            command: matchedCommand || 'Unknown command',
            response: commandData.data,
            responseTime: Math.max(responseTime, 0),
            success: !dataLower.includes('error') && !dataLower.includes('fail'),
            error: (dataLower.includes('error') || dataLower.includes('fail')) ? dataStr : undefined,
            timestamp: commandData.timestamp
          },
          timestamp: Date.now()
        })
      }
    })
  }

  private setupTestAutomationListeners() {
    if (!this.testAutomationService) {
      return
    }

    // Listen for test execution progress updates
    this.testAutomationService.on('progress_update', (execution) => {
      this.broadcast({
        type: 'test_progress',
        data: execution,
        timestamp: Date.now()
      })
    })

    // Listen for test execution completion
    this.testAutomationService.on('execution_complete', (execution) => {
      this.broadcast({
        type: 'test_complete',
        data: execution,
        timestamp: Date.now()
      })
    })

    // Listen for test execution errors
    this.testAutomationService.on('execution_error', (error) => {
      this.broadcast({
        type: 'test_error',
        data: { error },
        timestamp: Date.now()
      })
    })

    // Listen for test execution stopped
    this.testAutomationService.on('execution_stopped', (execution) => {
      this.broadcast({
        type: 'test_stopped',
        data: execution,
        timestamp: Date.now()
      })
    })
  }

  private setupConfigurationListeners() {
    // Listen for configuration events and broadcast to all connected clients
    modularConfigService.on('configurationLoaded', (data) => {
      this.broadcast({
        type: 'configuration_loaded',
        data,
        timestamp: Date.now()
      })
    })

    modularConfigService.on('configurationError', (error) => {
      this.broadcast({
        type: 'configuration_error',
        data: { error: error.message },
        timestamp: Date.now()
      })
    })

    modularConfigService.on('moduleUpdated', (data) => {
      this.broadcast({
        type: 'module_updated',
        data,
        timestamp: Date.now()
      })
    })

    modularConfigService.on('moduleUpdateError', (data) => {
      this.broadcast({
        type: 'module_update_error',
        data,
        timestamp: Date.now()
      })
    })
  }

  handleConnection(ws: WebSocket) {
    this.clients.add(ws)
    console.log(`WebSocket client connected. Total clients: ${this.clients.size}`)

    // Send initial state to new client
    this.sendInitialState(ws)

    ws.on('message', (data) => {
      try {
        const message = JSON.parse(data.toString())
        this.handleMessage(ws, message)
      } catch (error) {
        console.error('Failed to parse WebSocket message:', error)
        this.sendError(ws, 'Invalid message format')
      }
    })

    ws.on('close', () => {
      this.clients.delete(ws)
      console.log(`WebSocket client disconnected. Total clients: ${this.clients.size}`)
    })

    ws.on('error', (error) => {
      console.error('WebSocket error:', error)
      this.clients.delete(ws)
    })
  }

  private sendInitialState(ws: WebSocket) {
    // Send current connection status
    const message: WebSocketMessage = {
      type: 'initial_state',
      data: {
        connected: this.hardwareInterface.isConnected(),
        port: this.hardwareInterface.getSerialPort(),
        pins: Object.fromEntries(this.hardwareInterface.getPinStates()),
        test_execution: this.testAutomationService?.getCurrentExecution() || null,
        test_in_progress: this.testAutomationService?.isExecutionInProgress() || false
      },
      timestamp: Date.now()
    }

    this.sendToClient(ws, message)
  }

  private async handleMessage(ws: WebSocket, message: any) {
    try {
      switch (message.type) {
        case 'hardware_command':
          await this.handleHardwareCommand(ws, message.data)
          break

        case 'ping':
          this.sendToClient(ws, {
            type: 'pong',
            data: { timestamp: message.timestamp },
            timestamp: Date.now()
          })
          break

        case 'get_pin_states':
          this.sendToClient(ws, {
            type: 'pin_states',
            data: Object.fromEntries(this.hardwareInterface.getPinStates()),
            timestamp: Date.now()
          })
          break

        default:
          console.warn('Unknown WebSocket message type:', message.type)
          this.sendError(ws, `Unknown message type: ${message.type}`)
      }
    } catch (error) {
      console.error('Error handling WebSocket message:', error)
      this.sendError(ws, 'Failed to process message')
    }
  }

  private async handleHardwareCommand(ws: WebSocket, commandData: any) {
    const { command, pin, value } = commandData

    try {
      let hardwareCommand
      
      switch (command) {
        case 'write_pin':
          hardwareCommand = {
            command: `WRITE_PIN ${pin} ${value}`,
            expectResponse: true
          }
          break

        case 'read_pin':
          hardwareCommand = {
            command: `READ_PIN ${pin}`,
            expectResponse: true
          }
          break

        case 'read_adc':
          hardwareCommand = {
            command: `READ_ADC ${pin}`,
            expectResponse: true
          }
          break

        case 'read_pwm':
          hardwareCommand = {
            command: `READ_PWM ${pin}`,
            expectResponse: true
          }
          break

        case 'set_pwm':
          hardwareCommand = {
            command: `SET_PWM ${pin} ${value.frequency} ${value.dutyCycle}`,
            expectResponse: true
          }
          break

        case 'ping':
          hardwareCommand = {
            command: 'PING',
            expectResponse: true
          }
          break

        default:
          throw new Error(`Unknown hardware command: ${command}`)
      }

      const response = await this.hardwareInterface.sendCommand(hardwareCommand)
      
      this.sendToClient(ws, {
        type: 'command_response',
        data: {
          command,
          pin,
          value,
          response,
          success: response.success
        },
        timestamp: Date.now()
      })

    } catch (error) {
      this.sendError(ws, `Hardware command failed: ${error instanceof Error ? error.message : 'Unknown error'}`)
    }
  }

  private sendToClient(ws: WebSocket, message: WebSocketMessage) {
    if (ws.readyState === WebSocket.OPEN) {
      try {
        ws.send(JSON.stringify(message))
      } catch (error) {
        console.error('Failed to send message to WebSocket client:', error)
      }
    }
  }

  private sendError(ws: WebSocket, error: string) {
    this.sendToClient(ws, {
      type: 'error',
      data: { error },
      timestamp: Date.now()
    })
  }

  private broadcast(message: WebSocketMessage) {
    const messageStr = JSON.stringify(message)
    
    this.clients.forEach(client => {
      if (client.readyState === WebSocket.OPEN) {
        try {
          client.send(messageStr)
        } catch (error) {
          console.error('Failed to broadcast to WebSocket client:', error)
          this.clients.delete(client)
        }
      } else {
        this.clients.delete(client)
      }
    })
  }

  getClientCount(): number {
    return this.clients.size
  }
}
