import { spawn, ChildProcess } from 'child_process'
import { EventEmitter } from 'events'

export interface PinState {
  pin: string
  signal: string
  direction: 'IN' | 'OUT' | 'ANALOG'
  state: 'HIGH' | 'LOW' | number
  timestamp: number
}

export interface HardwareCommand {
  command: string
  args?: string[]
  expectResponse?: boolean
}

export interface HardwareResponse {
  success: boolean
  data?: any
  error?: string
  timestamp: number
}

/**
 * Hardware Interface Adapter
 * Integrates with existing Python HIL framework for Arduino Test Wrapper communication
 * Based on scripts/hil_serial.py and test/acceptance/hil_framework/hardware_interface.py
 */
export class HardwareInterface extends EventEmitter {
  private pythonProcess: ChildProcess | null = null
  private connected = false
  private serialPort: string | null = null
  private pinStates: Map<string, PinState> = new Map()
  private commandQueue: Array<{ command: HardwareCommand; resolve: Function; reject: Function }> = []
  private processingCommand = false

  constructor() {
    super()
    this.initializePinStates()
  }

  private initializePinStates() {
    // Initialize pin states based on pin-matrix.md (SOLE SOURCE OF TRUTH)
    const initialPins = [
      { pin: 'D7', signal: 'FREQ_DIV10_4', direction: 'IN' as const },
      { pin: 'D8', signal: 'FREQ_LOCK_4', direction: 'IN' as const },
      { pin: 'A2', signal: 'OVERLOAD_4', direction: 'IN' as const },
      { pin: 'A3', signal: 'START_4', direction: 'OUT' as const },
      { pin: 'A4', signal: 'RESET_4', direction: 'OUT' as const },
      { pin: 'A1', signal: 'POWER_SENSE_4', direction: 'ANALOG' as const },
      { pin: 'D9', signal: 'AMPLITUDE_ALL', direction: 'IN' as const },
      { pin: 'D10', signal: 'UART_RXD', direction: 'IN' as const },
      { pin: 'D11', signal: 'UART_TXD', direction: 'OUT' as const },
      { pin: 'D12', signal: 'STATUS_LED', direction: 'OUT' as const }
    ]

    initialPins.forEach(({ pin, signal, direction }) => {
      this.pinStates.set(signal, {
        pin,
        signal,
        direction,
        state: direction === 'ANALOG' ? 0 : 'LOW',
        timestamp: Date.now()
      })
    })
  }

  async initialize(): Promise<boolean> {
    try {
      console.log('Initializing hardware interface with auto-detection...')

      // First, try to auto-detect Arduino test harness
      const detectedPort = await this.detectHardware()
      if (detectedPort) {
        console.log(`Auto-detected Arduino test harness on port: ${detectedPort}`)
      }

      // Spawn Python HIL interface process
      this.pythonProcess = spawn('python3', [
        '-c', `
import sys
import os
sys.path.insert(0, '${process.cwd()}/test/acceptance')
sys.path.insert(0, '${process.cwd()}/scripts')

from hil_framework.hardware_interface import HardwareInterface
import json
import time

# Initialize HIL hardware interface
hil = HardwareInterface()

try:
    if hil.connect():
        print(json.dumps({"type": "connection", "status": "connected", "port": hil.serial_port}))
        sys.stdout.flush()
        
        # Continuous monitoring loop for sandbox mode
        while True:
            try:
                # Read commands from stdin
                line = sys.stdin.readline().strip()
                if not line:
                    continue
                    
                cmd = json.loads(line)
                
                if cmd["type"] == "ping":
                    response = hil.send_command("PING")
                    print(json.dumps({"type": "response", "data": response}))
                    
                elif cmd["type"] == "pin_read":
                    pin = cmd["pin"]
                    if pin.startswith("A"):
                        # ADC read
                        response = hil.send_command(f"READ_ADC {pin}")
                    else:
                        # Digital read
                        response = hil.send_command(f"READ_PIN {pin}")
                    print(json.dumps({"type": "pin_state", "pin": pin, "data": response}))
                    
                elif cmd["type"] == "pin_write":
                    pin = cmd["pin"]
                    value = cmd["value"]
                    response = hil.send_command(f"WRITE_PIN {pin} {value}")
                    print(json.dumps({"type": "response", "data": response}))
                    
                elif cmd["type"] == "status":
                    response = hil.send_command("STATUS")
                    print(json.dumps({"type": "status", "data": response}))
                    
                sys.stdout.flush()
                
            except json.JSONDecodeError:
                continue
            except Exception as e:
                print(json.dumps({"type": "error", "error": str(e)}))
                sys.stdout.flush()
                
    else:
        print(json.dumps({"type": "connection", "status": "failed", "error": "Could not connect to hardware"}))
        sys.exit(1)
        
except Exception as e:
    print(json.dumps({"type": "error", "error": str(e)}))
    sys.exit(1)
`
      ], {
        cwd: process.cwd(),
        stdio: ['pipe', 'pipe', 'pipe']
      })

      this.pythonProcess.stdout?.on('data', (data) => {
        const lines = data.toString().split('\n').filter((line: string) => line.trim())
        lines.forEach((line: string) => {
          try {
            const message = JSON.parse(line)
            this.handlePythonMessage(message)
          } catch (err) {
            console.error('Failed to parse Python message:', line)
          }
        })
      })

      this.pythonProcess.stderr?.on('data', (data) => {
        console.error('Python HIL error:', data.toString())
      })

      this.pythonProcess.on('exit', (code) => {
        console.log(`Python HIL process exited with code ${code}`)
        this.connected = false
        this.emit('disconnected')
      })

      // Wait for connection confirmation
      return new Promise((resolve) => {
        const timeout = setTimeout(() => {
          resolve(false)
        }, 10000)

        this.once('connected', () => {
          clearTimeout(timeout)
          resolve(true)
        })

        this.once('connection_failed', () => {
          clearTimeout(timeout)
          resolve(false)
        })
      })

    } catch (error) {
      console.error('Failed to initialize hardware interface:', error)
      return false
    }
  }

  private handlePythonMessage(message: any) {
    switch (message.type) {
      case 'connection':
        if (message.status === 'connected') {
          this.connected = true
          this.serialPort = message.port
          console.log(`Hardware connected on port: ${this.serialPort}`)
          this.emit('connected', { port: this.serialPort })
          this.startPinMonitoring()
        } else {
          this.emit('connection_failed', message.error)
        }
        break

      case 'pin_state':
        this.updatePinState(message.pin, message.data)
        break

      case 'response':
        this.emit('command_response', message.data)
        break

      case 'error':
        console.error('Hardware error:', message.error)
        this.emit('error', message.error)
        break
    }
  }

  private updatePinState(pin: string, data: string) {
    // Parse hardware response and update pin state
    const timestamp = Date.now()
    
    // Find pin by Arduino pin name or signal name
    for (const [signal, pinState] of this.pinStates.entries()) {
      if (pinState.pin === pin || signal === pin) {
        let newState: 'HIGH' | 'LOW' | number = 'LOW'
        
        if (data.includes('HIGH')) {
          newState = 'HIGH'
        } else if (data.includes('LOW')) {
          newState = 'LOW'
        } else if (data.includes('ADC=')) {
          const match = data.match(/ADC=(\d+)/)
          if (match) {
            newState = parseInt(match[1])
          }
        }
        
        const updatedPin = {
          ...pinState,
          state: newState,
          timestamp
        }
        
        this.pinStates.set(signal, updatedPin)
        this.emit('pin_update', signal, updatedPin)
        break
      }
    }
  }

  private startPinMonitoring() {
    // Start continuous pin monitoring for sandbox mode
    setInterval(() => {
      if (this.connected && this.pythonProcess) {
        // Monitor all pins periodically
        this.pinStates.forEach((pinState, signal) => {
          this.sendPythonCommand({
            type: 'pin_read',
            pin: pinState.pin
          })
        })
      }
    }, 1000) // Monitor every second
  }

  private sendPythonCommand(command: any): Promise<any> {
    return new Promise((resolve, reject) => {
      if (!this.pythonProcess || !this.connected) {
        reject(new Error('Hardware not connected'))
        return
      }

      try {
        this.pythonProcess.stdin?.write(JSON.stringify(command) + '\n')
        resolve(true)
      } catch (error) {
        reject(error)
      }
    })
  }

  async sendCommand(command: HardwareCommand): Promise<HardwareResponse> {
    if (!this.connected) {
      return {
        success: false,
        error: 'Hardware not connected',
        timestamp: Date.now()
      }
    }

    try {
      await this.sendPythonCommand({
        type: 'command',
        command: command.command,
        args: command.args
      })

      return {
        success: true,
        timestamp: Date.now()
      }
    } catch (error) {
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error',
        timestamp: Date.now()
      }
    }
  }

  getPinStates(): Map<string, PinState> {
    return new Map(this.pinStates)
  }

  isConnected(): boolean {
    return this.connected
  }

  getSerialPort(): string | null {
    return this.serialPort
  }

  private async detectHardware(): Promise<string | null> {
    try {
      // Use existing hardware detection script
      const detectProcess = spawn('python3', [
        'scripts/detect_hardware.py'
      ], {
        cwd: process.cwd(),
        stdio: ['pipe', 'pipe', 'pipe']
      })

      return new Promise((resolve) => {
        let output = ''

        detectProcess.stdout?.on('data', (data) => {
          output += data.toString()
        })

        detectProcess.on('exit', (code) => {
          if (code === 0 && output.trim()) {
            // Parse detected port from output
            const lines = output.trim().split('\n')
            const portLine = lines.find(line => line.includes('/dev/') || line.includes('COM'))
            if (portLine) {
              const match = portLine.match(/(\/dev\/[^\s]+|COM\d+)/)
              resolve(match ? match[1] : null)
            } else {
              resolve(null)
            }
          } else {
            resolve(null)
          }
        })

        // Timeout after 5 seconds
        setTimeout(() => {
          detectProcess.kill()
          resolve(null)
        }, 5000)
      })
    } catch (error) {
      console.error('Hardware detection failed:', error)
      return null
    }
  }

  async reconnect(): Promise<boolean> {
    console.log('Attempting to reconnect to hardware...')
    this.disconnect()

    // Wait a moment before reconnecting
    await new Promise(resolve => setTimeout(resolve, 1000))

    return this.initialize()
  }

  async retryConnection(maxAttempts: number = 3, delayMs: number = 2000): Promise<boolean> {
    for (let attempt = 1; attempt <= maxAttempts; attempt++) {
      console.log(`Connection attempt ${attempt}/${maxAttempts}`)

      const success = await this.initialize()
      if (success) {
        return true
      }

      if (attempt < maxAttempts) {
        console.log(`Attempt ${attempt} failed, waiting ${delayMs}ms before retry...`)
        await new Promise(resolve => setTimeout(resolve, delayMs))
        // Exponential backoff
        delayMs *= 1.5
      }
    }

    console.log('All connection attempts failed')
    return false
  }

  disconnect() {
    if (this.pythonProcess) {
      this.pythonProcess.kill()
      this.pythonProcess = null
    }
    this.connected = false
    this.emit('disconnected')
  }
}
