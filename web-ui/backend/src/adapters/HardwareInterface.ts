import { spawn, ChildProcess } from 'child_process'
import { EventEmitter } from 'events'
// import { DirectHardwareInterface } from './DirectHardwareInterface'

export interface PinState {
  pin: string
  signal: string
  direction: 'IN' | 'OUT' | 'ANALOG'
  state: 'HIGH' | 'LOW' | number | string  // Allow string for frequency displays
  timestamp: number
  frequency?: number  // For frequency pins
  frequencyDisplay?: string  // For frequency pins
  operatingFrequency?: string  // For frequency pins
  enabled?: boolean  // For frequency pins
  isActive?: boolean  // For frequency pins
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
  private configuration = {
    sonicator4: {
      operatingFrequencyKHz: 20.0,  // Default 20kHz
      outputFrequencyKHz: 2.0,      // 1/10th of operating frequency
      enabled: true,
      manualMode: false,            // Manual override mode
      manualFrequencyKHz: 2.0       // Manual frequency override
    },
    testHarness: {
      pinMapping: {
        FREQ_DIV10_4: 'D7',
        FREQ_LOCK_4: 'D8',
        OVERLOAD_4: 'A2',
        START_4: 'A3',
        RESET_4: 'A4',
        POWER_SENSE_4: 'A1',
        AMPLITUDE_ALL: 'D9',
        UART_TXD: 'D11',
        STATUS_LED: 'D12'
      }
    }
  }

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
      { pin: 'D9', signal: 'AMPLITUDE_ALL', direction: 'OUT' as const },
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



      // Fallback: First, try to auto-detect Arduino test harness
      const detectedPort = await this.detectHardware()
      if (detectedPort) {
        console.log(`Auto-detected Arduino test harness on port: ${detectedPort}`)
      }

      // Spawn Python HIL interface process using the web-ui venv
      const pythonPath = `${process.cwd()}/../venv/bin/python`
      this.pythonProcess = spawn(pythonPath, [
        '-c', `
import sys
import os
import time

# Add paths for HIL framework
sys.path.insert(0, '${process.cwd()}/test/acceptance')
sys.path.insert(0, '${process.cwd()}/scripts')

# Try to import HIL framework with fallback
try:
    from hil_framework.hardware_interface import HardwareInterface
except ImportError:
    # Fallback: create a mock hardware interface for development
    class HardwareInterface:
        def __init__(self):
            self.connected = False
            self.serial_port = None
            # Mock pin states for testing
            self.pin_states = {
                'FREQ_DIV10_4': 'LOW',
                'FREQ_LOCK_4': 'LOW',
                'OVERLOAD_4': 'LOW',
                'START_4': 'LOW',
                'RESET_4': 'LOW',
                'POWER_SENSE_4': 512,  # ADC value
                'AMPLITUDE_ALL': '0.0%',
                'UART_TXD': 'LOW',
                'STATUS_LED': 'LOW'
            }
            # Frequency generation configuration
            self.frequency_config = {
                'FREQ_DIV10_4': {
                    'frequency_hz': 2000,  # Default 2kHz (20kHz / 10)
                    'enabled': True,
                    'last_toggle': time.time(),
                    'state': False
                }
            }
            # Start frequency generation thread
            self.frequency_thread = threading.Thread(target=self.frequency_generator, daemon=True)
            self.frequency_thread.start()

        def verify_connection(self):
            print('{"type": "connection", "status": "connected", "port": "mock"}')
            return True

        def disconnect(self):
            self.connected = False

        def frequency_generator(self):
            """Generate frequency signals and monitor PWM for configured pins"""
            pwm_update_counter = 0
            while True:
                try:
                    current_time = time.time()

                    # Update frequency pins
                    for signal, config in self.frequency_config.items():
                        if config['enabled'] and config['frequency_hz'] > 0:
                            period = 1.0 / config['frequency_hz']
                            half_period = period / 2.0

                            if current_time - config['last_toggle'] >= half_period:
                                # Toggle the pin state
                                config['state'] = not config['state']
                                config['last_toggle'] = current_time

                                # Update pin state and send update
                                new_state = 'HIGH' if config['state'] else 'LOW'
                                self.pin_states[signal] = new_state

                                # Send pin state update
                                print(json.dumps({"type": "pin_state", "pin": signal, "data": new_state}))
                                sys.stdout.flush()

                    # Update PWM pins every 100ms (reduce update frequency for PWM)
                    pwm_update_counter += 1
                    if pwm_update_counter >= 100:  # Update PWM every 100ms
                        pwm_update_counter = 0
                        self.update_pwm_monitoring()

                    time.sleep(0.001)  # 1ms sleep for reasonable precision
                except Exception as e:
                    print(f"Frequency generator error: {e}")
                    time.sleep(0.1)

        def update_pwm_monitoring(self):
            """Continuously monitor PWM signals"""
            import random

            # Simulate AMPLITUDE_ALL PWM monitoring with varying duty cycle
            # In real hardware, this would read the actual PWM signal from the ATmega32A
            duty_cycle = random.uniform(0, 100)
            pwm_value = f"{duty_cycle:.1f}%"

            # Only update if value changed significantly (reduce noise)
            current_value = self.pin_states.get('AMPLITUDE_ALL', '0.0%')
            try:
                current_duty = float(current_value.replace('%', ''))
                new_duty = float(pwm_value.replace('%', ''))

                # Update if difference is > 1% to reduce noise
                if abs(new_duty - current_duty) > 1.0:
                    self.pin_states['AMPLITUDE_ALL'] = pwm_value

                    # Send PWM state update
                    print(json.dumps({"type": "pin_state", "pin": "AMPLITUDE_ALL", "data": pwm_value}))
                    sys.stdout.flush()
            except:
                # Fallback: always update if parsing fails
                self.pin_states['AMPLITUDE_ALL'] = pwm_value
                print(json.dumps({"type": "pin_state", "pin": "AMPLITUDE_ALL", "data": pwm_value}))
                sys.stdout.flush()

        def send_command(self, cmd):
            # Parse command and update mock states
            parts = cmd.split()
            if parts[0] == 'PING':
                return "PONG - Multi-Sonicator-IO Mock Hardware v1.0"
            elif len(parts) >= 3 and parts[0] == 'SET_FREQUENCY':
                signal = parts[1]
                frequency_hz = float(parts[2])
                if signal in self.frequency_config:
                    self.frequency_config[signal]['frequency_hz'] = frequency_hz
                    print(f"Set {signal} frequency to {frequency_hz}Hz")
                return "OK"
            elif len(parts) >= 3 and parts[0] == 'WRITE_PIN':
                signal = parts[1]
                state = parts[2]
                if signal in self.pin_states:
                    self.pin_states[signal] = state
                    # Send pin state update
                    print(json.dumps({"type": "pin_state", "pin": signal, "data": state}))
                    sys.stdout.flush()
                return "OK"
            elif len(parts) >= 2 and parts[0] == 'READ_PIN':
                signal_or_pin = parts[1]
                # Map Arduino pin names to signal names
                pin_to_signal = {
                    'D7': 'FREQ_DIV10_4',
                    'D8': 'FREQ_LOCK_4',
                    'A2': 'OVERLOAD_4',
                    'A3': 'START_4',
                    'A4': 'RESET_4',
                    'A1': 'POWER_SENSE_4',
                    'D9': 'AMPLITUDE_ALL',
                    'D10': 'UART_RXD',
                    'D11': 'UART_TXD',
                    'D12': 'STATUS_LED'
                }

                # Check if it's an Arduino pin name, convert to signal name
                if signal_or_pin in pin_to_signal:
                    signal = pin_to_signal[signal_or_pin]
                else:
                    signal = signal_or_pin

                if signal in self.pin_states:
                    return str(self.pin_states[signal])
                return "UNKNOWN"
            elif len(parts) >= 2 and parts[0] == 'READ_PWM':
                signal_or_pin = parts[1]
                # Map Arduino pin names to signal names
                pin_to_signal = {
                    'D9': 'AMPLITUDE_ALL',
                    'AMPLITUDE_ALL': 'AMPLITUDE_ALL'
                }

                # Check if it's an Arduino pin name, convert to signal name
                if signal_or_pin in pin_to_signal:
                    signal = pin_to_signal[signal_or_pin]
                else:
                    signal = signal_or_pin

                if signal == 'AMPLITUDE_ALL':
                    # Simulate continuous PWM monitoring with varying duty cycle
                    import random
                    duty_cycle = random.uniform(0, 100)
                    # Update pin state for continuous monitoring
                    self.pin_states[signal] = f"{duty_cycle:.1f}%"
                    return f"OK PWM={duty_cycle:.1f}%"
                return "ERR INVALID_PWM_CHANNEL"
            return "OK"

import json
import time
import threading

# Initialize HIL hardware interface
hil = HardwareInterface()

try:
    if hil.verify_connection():
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

                elif cmd["type"] == "command":
                    # Handle generic commands from the API
                    command = cmd["command"]
                    args = cmd.get("args", [])

                    if command == "ping":
                        response = hil.send_command("PING")
                        print(json.dumps({"type": "response", "data": response}))

                    elif command == "set_frequency" and len(args) >= 2:
                        signal = args[0]
                        frequency_hz = args[1]
                        response = hil.send_command(f"SET_FREQUENCY {signal} {frequency_hz}")
                        print(json.dumps({"type": "response", "data": response}))

                    elif command == "write_pin" and len(args) >= 2:
                        signal = args[0]
                        state = args[1]
                        response = hil.send_command(f"WRITE_PIN {signal} {state}")
                        print(json.dumps({"type": "response", "data": response}))

                    elif command == "read_pin" and len(args) >= 1:
                        signal = args[0]
                        response = hil.send_command(f"READ_PIN {signal}")
                        print(json.dumps({"type": "pin_state", "pin": signal, "data": response}))

                    else:
                        # Generic command
                        cmd_str = f"{command} {' '.join(args)}" if args else command
                        response = hil.send_command(cmd_str)
                        print(json.dumps({"type": "response", "data": response}))

                elif cmd["type"] == "pin_read":
                    pin = cmd["pin"]
                    if pin == "D9" or pin == "AMPLITUDE_ALL":
                        # PWM read for AMPLITUDE_ALL
                        response = hil.send_command(f"READ_PWM {pin}")
                    elif pin.startswith("A"):
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
        cwd: `${process.cwd()}/../..`,
        stdio: ['pipe', 'pipe', 'pipe'],
        env: { ...process.env, PYTHONPATH: `${process.cwd()}/../../test/acceptance:${process.env.PYTHONPATH || ''}` }
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
        // Emit Arduino response for logging (reduced verbosity)
        this.emit('arduino_command', {
          direction: 'received',
          data: message.data,
          timestamp: Date.now(),
          type: 'response'
        })
        this.emit('command_response', message.data)
        break

      case 'error':
        console.error('Hardware error:', message.error)
        this.emit('error', message.error)
        break
    }
  }

  private lastLogTime: number = 0
  private logThrottleMs: number = 5000 // Only log once every 5 seconds per pin (reduced frequency)
  private lastPinLogTimes: Map<string, number> = new Map()

  private updatePinState(pin: string, data: string) {
    // Parse hardware response and update pin state
    const timestamp = Date.now()

    // Throttle logging to prevent spam - only log occasionally  
    const lastLogTime = this.lastPinLogTimes.get(pin) || 0
    const shouldLog = timestamp - lastLogTime > this.logThrottleMs

    // Find pin by Arduino pin name or signal name
    for (const [signal, pinState] of this.pinStates.entries()) {
      if (pinState.pin === pin || signal === pin) {
        let newState: 'HIGH' | 'LOW' | number | string = 'LOW'

        // Special handling for frequency pins
        if (signal === 'FREQ_DIV10_4') {
          if (this.configuration.sonicator4.enabled) {
            const modeIndicator = this.configuration.sonicator4.manualMode ? ' (M)' : ''
            newState = `${this.configuration.sonicator4.outputFrequencyKHz}kHz${modeIndicator}`
          } else {
            newState = 'OFF'
          }
        } else {
          // Handle direct state values for regular pins
          if (data === 'HIGH' || data.includes('HIGH')) {
            newState = 'HIGH'
          } else if (data === 'LOW' || data.includes('LOW')) {
            newState = 'LOW'
          } else if (data.includes('ADC=')) {
            const match = data.match(/ADC=(\d+)/)
            if (match) {
              newState = parseInt(match[1])
            }
          } else if (!isNaN(parseInt(data))) {
            // Handle numeric values (ADC readings)
            newState = parseInt(data)
          }
        }

        // Only log significant state changes occasionally (reduced verbosity)
        if (shouldLog && pinState.state !== newState) {
          this.lastPinLogTimes.set(pin, timestamp)
        }

        const updatedPin = {
          ...pinState,
          state: newState,
          timestamp,
          // Add frequency information for frequency pins
          ...(signal === 'FREQ_DIV10_4' && {
            frequency: this.configuration.sonicator4.outputFrequencyKHz * 1000,
            frequencyDisplay: `${this.configuration.sonicator4.outputFrequencyKHz}kHz`,
            operatingFrequency: `${this.configuration.sonicator4.operatingFrequencyKHz}kHz`,
            enabled: this.configuration.sonicator4.enabled,
            isActive: this.configuration.sonicator4.enabled && (data === 'HIGH' || data === 'LOW')
          })
        }

        this.pinStates.set(signal, updatedPin as PinState)
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
        const commandStr = JSON.stringify(command)

        // Emit command being sent for logging (reduced verbosity)
        this.emit('arduino_command', {
          direction: 'sent',
          data: commandStr,
          timestamp: Date.now(),
          type: command.type || 'unknown'
        })

        this.pythonProcess.stdin?.write(commandStr + '\n')
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



  getSerialPort(): string | null {
    return this.serialPort
  }

  private async detectHardware(): Promise<string | null> {
    try {
      // Use existing hardware detection script
      const pythonPath = `${process.cwd()}/../venv/bin/python`
      const detectProcess = spawn(pythonPath, [
        'scripts/detect_hardware.py'
      ], {
        cwd: `${process.cwd()}/../..`,
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

  getConfiguration() {
    return { ...this.configuration }
  }

  async updateConfiguration(newConfig: any): Promise<{ success: boolean; config: any }> {
    try {
      console.log('Hardware: Updating configuration:', newConfig)

      // Update sonicator frequency configuration
      if (newConfig.sonicator4?.operatingFrequencyKHz !== undefined) {
        const operatingFreq = parseFloat(newConfig.sonicator4.operatingFrequencyKHz)
        if (operatingFreq >= 18 && operatingFreq <= 22) { // CT2000 sonicator operating range: 18-22 kHz
          this.configuration.sonicator4.operatingFrequencyKHz = operatingFreq

          // Only update output frequency if not in manual mode
          if (!this.configuration.sonicator4.manualMode) {
            this.configuration.sonicator4.outputFrequencyKHz = operatingFreq / 10.0
          }

          console.log(`Hardware: Updated operating frequency to ${operatingFreq}kHz`)

          // Send frequency configuration to hardware (use manual frequency if in manual mode)
          const actualFreq = this.configuration.sonicator4.manualMode
            ? this.configuration.sonicator4.manualFrequencyKHz
            : operatingFreq / 10.0

          const command = {
            command: 'set_frequency',
            args: [
              'FREQ_DIV10_4',
              (actualFreq * 1000).toString() // Convert kHz to Hz
            ],
            expectResponse: true
          }

          const result = await this.sendCommand(command)
          console.log('Hardware: Frequency configuration result:', result)

          // Update the FREQ_DIV10_4 pin state to reflect new frequency
          this.updatePinState('FREQ_DIV10_4', 'HIGH')
        }
      }

      // Handle manual mode toggle
      if (newConfig.sonicator4?.manualMode !== undefined) {
        this.configuration.sonicator4.manualMode = newConfig.sonicator4.manualMode
        console.log(`Hardware: Manual mode ${newConfig.sonicator4.manualMode ? 'enabled' : 'disabled'}`)

        // If switching to automatic mode, recalculate output frequency
        if (!newConfig.sonicator4.manualMode) {
          this.configuration.sonicator4.outputFrequencyKHz = this.configuration.sonicator4.operatingFrequencyKHz / 10.0
        }
      }

      // Handle manual frequency override
      if (newConfig.sonicator4?.manualFrequencyKHz !== undefined) {
        const manualFreq = parseFloat(newConfig.sonicator4.manualFrequencyKHz)
        if (manualFreq >= 1.8 && manualFreq <= 2.2) { // CT2000 output frequency range: 1.8-2.2 kHz (div/10)
          this.configuration.sonicator4.manualFrequencyKHz = manualFreq

          // If in manual mode, update the actual output frequency
          if (this.configuration.sonicator4.manualMode) {
            this.configuration.sonicator4.outputFrequencyKHz = manualFreq

            console.log(`Hardware: Manual frequency set to ${manualFreq}kHz`)

            // Send manual frequency to hardware
            const command = {
              command: 'set_frequency',
              args: [
                'FREQ_DIV10_4',
                (manualFreq * 1000).toString() // Convert kHz to Hz
              ],
              expectResponse: true
            }

            const result = await this.sendCommand(command)
            console.log('Hardware: Manual frequency configuration result:', result)

            // Update the FREQ_DIV10_4 pin state
            this.updatePinState('FREQ_DIV10_4', 'HIGH')
          }
        }
      }

      // Update other configuration options as needed
      if (newConfig.sonicator4?.enabled !== undefined) {
        this.configuration.sonicator4.enabled = newConfig.sonicator4.enabled

        // Update the FREQ_DIV10_4 pin state to reflect enabled/disabled status
        this.updatePinState('FREQ_DIV10_4', newConfig.sonicator4.enabled ? 'HIGH' : 'LOW')
      }

      return {
        success: true,
        config: this.configuration
      }
    } catch (error) {
      console.error('Hardware: Error updating configuration:', error)
      return {
        success: false,
        config: this.configuration
      }
    }
  }

  // Web UI Communication Methods
  async ping(): Promise<{ success: boolean; data?: any; error?: string; responseTime?: number }> {
    const startTime = Date.now()

    try {
      const command = {
        command: 'ping',
        args: [],
        expectResponse: true
      }

      const result = await this.sendCommand(command)
      return {
        success: result.success,
        data: result.data,
        error: result.error,
        responseTime: Date.now() - startTime
      }
    } catch (error) {
      return {
        success: false,
        error: `Ping failed: ${error}`,
        responseTime: Date.now() - startTime
      }
    }
  }

  async getInfo(): Promise<{ success: boolean; data?: any; error?: string }> {
    try {
      const command = {
        command: 'info',
        args: [],
        expectResponse: true
      }

      return await this.sendCommand(command)
    } catch (error) {
      return {
        success: false,
        error: `Get info failed: ${error}`
      }
    }
  }

  async getStatus(sonicator: number = 4): Promise<{ success: boolean; data?: any; error?: string }> {
    try {
      const command = {
        command: 'status',
        args: [sonicator.toString()],
        expectResponse: true
      }

      return await this.sendCommand(command)
    } catch (error) {
      return {
        success: false,
        error: `Get status failed: ${error}`
      }
    }
  }

  isConnected(): boolean {
    return this.connected
  }

  getPortPath(): string | null {
    return this.serialPort
  }
}
