import { useState, useEffect } from 'react'
import { HardwareState } from '../types'

interface ControlPanelProps {
  hardwareState: HardwareState
  onPinControl: (pin: string, action: string, value?: any) => void
  connected: boolean
}

export default function ControlPanel({ hardwareState, onPinControl, connected }: ControlPanelProps) {
  const [pwmFrequency, setPwmFrequency] = useState(1000)
  const [pwmDutyCycle, setPwmDutyCycle] = useState(50)
  const [selectedPin, setSelectedPin] = useState('')
  const [configuration, setConfiguration] = useState({
    sonicator4: {
      operatingFrequencyKHz: 20.0,
      outputFrequencyKHz: 2.0,
      enabled: true
    }
  })
  const [configLoading, setConfigLoading] = useState(false)
  const [frequencyInput, setFrequencyInput] = useState('20.0')

  const inputPins = Object.entries(hardwareState.pins).filter(([_, pin]) => pin.direction === 'IN')
  const outputPins = Object.entries(hardwareState.pins).filter(([_, pin]) => pin.direction === 'OUT')
  const analogPins = Object.entries(hardwareState.pins).filter(([_, pin]) => pin.direction === 'ANALOG')

  const handlePinToggle = (signal: string, currentState: any) => {
    // Don't allow toggling frequency pins
    if (signal === 'FREQ_DIV10_4') {
      return
    }
    const newState = currentState === 'HIGH' ? 'LOW' : 'HIGH'
    onPinControl(signal, 'write_pin', newState)
  }

  const isFrequencyPin = (signal: string) => {
    return signal === 'FREQ_DIV10_4'
  }

  const renderPinState = (signal: string, pinState: any) => {
    if (isFrequencyPin(signal)) {
      return (
        <div className="frequency-pin-display">
          <div className={`pin-state frequency ${pinState.enabled ? 'active' : 'inactive'}`}>
            <div className="frequency-value">
              {pinState.state}
            </div>
            {pinState.enabled && (
              <div className="frequency-indicator">
                <div className="pulse-dot"></div>
                <span className="text-xs text-gray-500">Active</span>
              </div>
            )}
            <div className="text-xs text-gray-500 mt-1">
              Operating: {pinState.operatingFrequency || 'N/A'}
            </div>
            <div className="text-xs text-gray-400">
              {formatTimestamp(pinState.timestamp)}
            </div>
          </div>
        </div>
      )
    } else {
      return (
        <button
          className={`btn ${pinState.state === 'HIGH' ? 'success' : ''} w-full`}
          onClick={() => handlePinToggle(signal, pinState.state)}
          disabled={!connected}
          title={`Toggle ${signal} (currently ${pinState.state})`}
        >
          {pinState.state}
          <span className="text-xs ml-1">
            {formatTimestamp(pinState.timestamp)}
          </span>
        </button>
      )
    }
  }

  const handlePinRead = (signal: string, isAnalog: boolean = false) => {
    onPinControl(signal, isAnalog ? 'read_adc' : 'read_pin')
  }

  const handlePWMSet = () => {
    if (!selectedPin) return

    // Validate PWM parameters
    if (pwmFrequency < 1 || pwmFrequency > 50000) {
      alert('PWM frequency must be between 1 and 50000 Hz')
      return
    }

    if (pwmDutyCycle < 0 || pwmDutyCycle > 100) {
      alert('PWM duty cycle must be between 0 and 100%')
      return
    }

    onPinControl(selectedPin, 'set_pwm', {
      frequency: pwmFrequency,
      dutyCycle: pwmDutyCycle
    })
  }

  const handlePing = () => {
    onPinControl('', 'ping')
  }

  const handleReadAllPins = () => {
    // Read all pins for comprehensive status update
    Object.entries(hardwareState.pins).forEach(([signal, pinState]) => {
      if (pinState.direction === 'ANALOG') {
        onPinControl(signal, 'read_adc')
      } else {
        onPinControl(signal, 'read_pin')
      }
    })
  }

  const handleSetAllInputsLow = () => {
    inputPins.forEach(([signal]) => {
      onPinControl(signal, 'write_pin', 'LOW')
    })
  }

  const handleSetAllInputsHigh = () => {
    inputPins.forEach(([signal]) => {
      onPinControl(signal, 'write_pin', 'HIGH')
    })
  }

  const formatTimestamp = (timestamp: number) => {
    return new Date(timestamp).toLocaleTimeString()
  }

  // Load configuration on component mount
  useEffect(() => {
    loadConfiguration()
  }, [])

  const loadConfiguration = async () => {
    try {
      const response = await fetch('/api/config')
      const data = await response.json()
      if (data.success && data.config) {
        setConfiguration(data.config)
        setFrequencyInput(data.config.sonicator4.operatingFrequencyKHz.toString())
      }
    } catch (error) {
      console.error('Failed to load configuration:', error)
    }
  }

  const handleFrequencyUpdate = async () => {
    const frequency = parseFloat(frequencyInput)

    // Validate frequency
    if (isNaN(frequency) || frequency <= 0 || frequency > 100) {
      alert('Operating frequency must be between 0.1 and 100 kHz')
      return
    }

    setConfigLoading(true)
    try {
      const response = await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          config: {
            sonicator4: {
              operatingFrequencyKHz: frequency,
              enabled: configuration.sonicator4.enabled
            }
          }
        })
      })

      const data = await response.json()
      if (data.success) {
        setConfiguration(data.config)
        console.log(`Frequency updated to ${frequency}kHz (output: ${frequency / 10}kHz)`)
      } else {
        alert('Failed to update frequency configuration')
      }
    } catch (error) {
      console.error('Error updating frequency:', error)
      alert('Network error updating frequency')
    } finally {
      setConfigLoading(false)
    }
  }

  const handleFrequencyToggle = async () => {
    setConfigLoading(true)
    try {
      const newEnabled = !configuration.sonicator4.enabled
      const response = await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          config: {
            sonicator4: {
              operatingFrequencyKHz: configuration.sonicator4.operatingFrequencyKHz,
              enabled: newEnabled
            }
          }
        })
      })

      const data = await response.json()
      if (data.success) {
        setConfiguration(data.config)
        console.log(`Frequency generation ${newEnabled ? 'enabled' : 'disabled'}`)
      }
    } catch (error) {
      console.error('Error toggling frequency:', error)
    } finally {
      setConfigLoading(false)
    }
  }

  return (
    <div className="control-panel">
      {/* Connection Status */}
      <div className="control-section">
        <h3>Connection Status</h3>
        <div className="connection-status">
          <div className={`connection-indicator ${connected ? 'connected' : 'disconnected'}`}></div>
          <span>{connected ? 'Connected' : 'Disconnected'}</span>
        </div>
        {hardwareState.connection.port && (
          <div className="text-xs text-gray-500 mt-1">
            Port: {hardwareState.connection.port}
          </div>
        )}
        <div className="flex gap-2 mt-2">
          <button
            className="btn primary"
            onClick={handlePing}
            disabled={!connected}
            title="Send PING command to test communication"
          >
            Ping
          </button>
          <button
            className="btn"
            onClick={handleReadAllPins}
            disabled={!connected}
            title="Read current state of all pins"
          >
            Read All
          </button>
        </div>
      </div>

      {/* Input Pin Controls */}
      <div className="control-section">
        <h3>Input Pins (Arduino → ATmega32A)</h3>
        <div className="text-xs text-gray-500 mb-2">
          Click to toggle HIGH/LOW states
        </div>
        <div className="flex gap-1 mb-2">
          <button
            className="btn text-xs"
            onClick={handleSetAllInputsLow}
            disabled={!connected}
            title="Set all input pins to LOW"
          >
            All LOW
          </button>
          <button
            className="btn text-xs"
            onClick={handleSetAllInputsHigh}
            disabled={!connected}
            title="Set all input pins to HIGH"
          >
            All HIGH
          </button>
        </div>
        {inputPins.map(([signal, pinState]) => (
          <div key={signal} className="mb-2">
            <div className="flex items-center justify-between mb-1">
              <span className="text-xs font-mono">{signal}</span>
              <span className="text-xs text-gray-500">{pinState.pin}</span>
            </div>
            {renderPinState(signal, pinState)}
          </div>
        ))}
      </div>

      {/* Output Pin Monitoring */}
      <div className="control-section">
        <h3>Output Pins (ATmega32A → Arduino)</h3>
        <div className="text-xs text-gray-500 mb-2">
          Read-only monitoring
        </div>
        {outputPins.map(([signal, pinState]) => (
          <div key={signal} className="mb-2">
            <div className="flex items-center justify-between mb-1">
              <span className="text-xs font-mono">{signal}</span>
              <span className="text-xs text-gray-500">{pinState.pin}</span>
            </div>
            <div className={`pin-state ${pinState.state === 'HIGH' ? 'high' : 'low'}`}>
              <span>{pinState.state}</span>
              <span className="text-xs">
                {formatTimestamp(pinState.timestamp)}
              </span>
            </div>
            <button
              className="btn text-xs mt-1"
              onClick={() => handlePinRead(signal)}
              disabled={!connected}
              title={`Read current state of ${signal}`}
            >
              Read Pin
            </button>
          </div>
        ))}
      </div>

      {/* Analog Pin Monitoring */}
      <div className="control-section">
        <h3>Analog Pins</h3>
        <div className="text-xs text-gray-500 mb-2">
          ADC readings (0-1023, 0-5V)
        </div>
        {analogPins.map(([signal, pinState]) => (
          <div key={signal} className="mb-2">
            <div className="flex items-center justify-between mb-1">
              <span className="text-xs font-mono">{signal}</span>
              <span className="text-xs text-gray-500">{pinState.pin}</span>
            </div>
            <div className="pin-state analog">
              <span className="font-mono">
                {typeof pinState.state === 'number' ? pinState.state : '---'}
                {typeof pinState.state === 'number' && (
                  <span className="text-xs ml-1">
                    ({((pinState.state / 1023) * 5).toFixed(2)}V)
                  </span>
                )}
              </span>
              <span className="text-xs">
                {formatTimestamp(pinState.timestamp)}
              </span>
            </div>
            <button
              className="btn text-xs mt-1"
              onClick={() => handlePinRead(signal, true)}
              disabled={!connected}
              title={`Read ADC value from ${signal}`}
            >
              Read ADC
            </button>
          </div>
        ))}
      </div>

      {/* PWM Control */}
      <div className="control-section">
        <h3>PWM Control</h3>
        <div className="text-xs text-gray-500 mb-2">
          Generate PWM signals on capable pins
        </div>
        
        <div className="mb-2">
          <label className="text-xs text-gray-700">Pin:</label>
          <select 
            className="input text-xs mt-1"
            value={selectedPin}
            onChange={(e) => setSelectedPin(e.target.value)}
          >
            <option value="">Select pin...</option>
            {inputPins.map(([signal, pinState]) => (
              <option key={signal} value={signal}>
                {signal} ({pinState.pin})
              </option>
            ))}
          </select>
        </div>

        <div className="mb-2">
          <label className="text-xs text-gray-700">Frequency (Hz):</label>
          <input
            type="number"
            className="input text-xs mt-1"
            value={pwmFrequency}
            onChange={(e) => setPwmFrequency(parseInt(e.target.value) || 0)}
            min="1"
            max="50000"
          />
        </div>

        <div className="mb-2">
          <label className="text-xs text-gray-700">Duty Cycle (%):</label>
          <input
            type="number"
            className="input text-xs mt-1"
            value={pwmDutyCycle}
            onChange={(e) => setPwmDutyCycle(parseInt(e.target.value) || 0)}
            min="0"
            max="100"
          />
        </div>

        <button
          className="btn primary"
          onClick={handlePWMSet}
          disabled={!connected || !selectedPin}
          title={`Set PWM on ${selectedPin}: ${pwmFrequency}Hz at ${pwmDutyCycle}%`}
        >
          Set PWM
        </button>
      </div>

      {/* Frequency Configuration */}
      <div className="control-section">
        <h3>Sonicator 4 Frequency Configuration</h3>
        <div className="text-xs text-gray-600 mb-3">
          Configure the operating frequency for Sonicator 4. The test harness will output 1/10th of this frequency on FREQ_DIV10_4 (D7).
        </div>

        <div className="mb-3">
          <label className="text-xs text-gray-700">Operating Frequency (kHz):</label>
          <div className="flex gap-2 mt-1">
            <input
              type="number"
              className="input text-xs flex-1"
              value={frequencyInput}
              onChange={(e) => setFrequencyInput(e.target.value)}
              min="0.1"
              max="100"
              step="0.1"
              placeholder="20.0"
            />
            <button
              className="btn primary text-xs px-3"
              onClick={handleFrequencyUpdate}
              disabled={configLoading || !connected}
              title="Update frequency configuration"
            >
              {configLoading ? 'Setting...' : 'Set'}
            </button>
          </div>
        </div>

        <div className="mb-3 p-2 bg-gray-50 rounded text-xs">
          <div className="flex justify-between items-center mb-1">
            <span className="font-medium">Current Configuration:</span>
            <button
              className={`btn text-xs px-2 py-1 ${configuration.sonicator4.enabled ? 'btn-success' : 'btn-secondary'}`}
              onClick={handleFrequencyToggle}
              disabled={configLoading || !connected}
              title={`${configuration.sonicator4.enabled ? 'Disable' : 'Enable'} frequency generation`}
            >
              {configuration.sonicator4.enabled ? 'ON' : 'OFF'}
            </button>
          </div>
          <div className="text-gray-600 space-y-1">
            <div>Operating: <strong>{configuration.sonicator4.operatingFrequencyKHz}kHz</strong></div>
            <div>Output (D7): <strong>{configuration.sonicator4.outputFrequencyKHz}kHz</strong></div>
            <div>Status: <strong className={configuration.sonicator4.enabled ? 'text-green-600' : 'text-gray-500'}>
              {configuration.sonicator4.enabled ? 'Generating' : 'Stopped'}
            </strong></div>
          </div>
        </div>

        <div className="text-xs text-blue-600 bg-blue-50 p-2 rounded">
          <strong>Note:</strong> The Arduino Test Harness automatically outputs frequency signals at 1/10th the configured operating frequency on pin D7 (FREQ_DIV10_4) to simulate the sonicator's frequency divider circuit.
        </div>
      </div>

      {/* System Status */}
      <div className="control-section">
        <h3>System Status</h3>
        <div className="text-xs text-gray-500 mb-2">
          Last update: {formatTimestamp(hardwareState.lastUpdate)}
        </div>
        <div className="text-xs">
          <div>Total pins: {Object.keys(hardwareState.pins).length}</div>
          <div>Input pins: {inputPins.length}</div>
          <div>Output pins: {outputPins.length}</div>
          <div>Analog pins: {analogPins.length}</div>
        </div>
      </div>

      {/* Help Section */}
      <div className="control-section">
        <h3>Quick Help</h3>
        <div className="text-xs text-gray-600 space-y-1">
          <div><strong>Input Pins:</strong> Control signals sent to ATmega32A</div>
          <div><strong>Output Pins:</strong> Monitor signals from ATmega32A</div>
          <div><strong>Analog Pins:</strong> Read ADC values (0-1023)</div>
          <div><strong>PWM:</strong> Generate pulse-width modulated signals</div>
          <div className="mt-2 pt-2 border-t border-gray-200">
            <strong>Pin Mapping:</strong> Based on <code>docs/planning/pin-matrix.md</code>
          </div>
        </div>
      </div>
    </div>
  )
}
