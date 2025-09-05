import { WebSocket } from 'ws'
import { HardwareInterface } from '../adapters/HardwareInterface.js'

export interface WebSocketMessage {
  type: string
  data: any
  timestamp: number
}

export class WebSocketHandler {
  private clients: Set<WebSocket> = new Set()
  private hardwareInterface: HardwareInterface

  constructor(hardwareInterface: HardwareInterface) {
    this.hardwareInterface = hardwareInterface
    this.setupHardwareListeners()
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
        pins: Object.fromEntries(this.hardwareInterface.getPinStates())
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
