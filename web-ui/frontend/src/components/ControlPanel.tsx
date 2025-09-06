import { useState, useEffect } from 'react'
import { HardwareState } from '../types'

interface ControlPanelProps {
  hardwareState: HardwareState
  onPinControl: (pin: string, action: string, value?: any) => void
  connected: boolean
}

export default function ControlPanel({ hardwareState, onPinControl, connected }: ControlPanelProps) {

  // Pin descriptions and mappings based on docs/planning/pin-matrix.md
  const pinDescriptions: Record<string, { description: string; wrapperPin: string; dutPin: string; physicalPin: string }> = {
    'FREQ_DIV10_4': {
      description: 'Sonicator #4 Frequency Output (in Hz/10)',
      wrapperPin: 'D7',
      dutPin: 'PB0',
      physicalPin: '1'
    },
    'FREQ_LOCK_4': {
      description: 'Sonicator #4 Frequency Lock Signal',
      wrapperPin: 'D8',
      dutPin: 'PB4',
      physicalPin: '5'
    },
    'OVERLOAD_4': {
      description: 'Sonicator #4 Overload Protection Signal',
      wrapperPin: 'A2',
      dutPin: 'PD3',
      physicalPin: '17'
    },
    'START_4': {
      description: 'Sonicator #4 Start Control Signal',
      wrapperPin: 'A3',
      dutPin: 'PC0',
      physicalPin: '22'
    },
    'RESET_4': {
      description: 'Sonicator #4 Reset Control Signal',
      wrapperPin: 'A4',
      dutPin: 'PC1',
      physicalPin: '23'
    },
    'POWER_SENSE_4': {
      description: 'Sonicator #4 Power Sensing (5.44 mV/W scaling)',
      wrapperPin: 'A1',
      dutPin: 'PA7',
      physicalPin: '33'
    },
    'AMPLITUDE_ALL': {
      description: 'Sonicator Amplitude Control (PWM Output)',
      wrapperPin: 'D9',
      dutPin: 'PD7',
      physicalPin: '21'
    },
    'UART_TXD': {
      description: 'UART Transmit Data Output',
      wrapperPin: 'D11',
      dutPin: 'PD1',
      physicalPin: '15'
    },
    'STATUS_LED': {
      description: 'System Status LED Output',
      wrapperPin: 'D12',
      dutPin: 'PD4',
      physicalPin: '18'
    }
  }

  const [configuration, setConfiguration] = useState({
    sonicator4: {
      operatingFrequencyKHz: 20.0,
      outputFrequencyKHz: 2.0,
      enabled: true,
      manualMode: false,
      manualFrequencyKHz: 2.0
    }
  })
  const [configLoading, setConfigLoading] = useState(false)
  const [frequencyInput, setFrequencyInput] = useState('20.0')
  const [manualFrequencyInput, setManualFrequencyInput] = useState('2.0')

  const inputPins = Object.entries(hardwareState.pins).filter(([signal, pin]) => pin.direction === 'IN' || signal === 'POWER_SENSE_4')
  const outputPins = Object.entries(hardwareState.pins).filter(([signal, pin]) => pin.direction === 'OUT' && signal !== 'UART_TXD')

  const handlePinToggle = (signal: string, currentState: any) => {
    // Don't allow toggling frequency pins or PWM output pins
    if (signal === 'FREQ_DIV10_4' || signal === 'AMPLITUDE_ALL') {
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

            {/* Inline Frequency Configuration Controls */}
            <div className="frequency-controls mt-3 p-3 bg-gray-50 rounded-lg border">
              <div className="flex items-center justify-between mb-2">
                <span className="text-xs font-medium text-gray-700">Mode:</span>
                <button
                  className={`btn text-xs px-2 py-1 ${configuration.sonicator4.manualMode ? 'btn-warning' : 'btn-secondary'}`}
                  onClick={handleManualModeToggle}
                  disabled={configLoading || !connected}
                  title={`${configuration.sonicator4.manualMode ? 'Switch to' : 'Switch to'} ${configuration.sonicator4.manualMode ? 'automatic' : 'manual'} mode`}
                >
                  {configuration.sonicator4.manualMode ? 'MANUAL' : 'AUTO'}
                </button>
              </div>

              {configuration.sonicator4.manualMode ? (
                <div className="mb-2">
                  <label className="text-xs text-gray-600 block mb-1">Manual Frequency (kHz):</label>
                  <div className="flex gap-1">
                    <input
                      type="number"
                      className="input text-xs flex-1"
                      value={manualFrequencyInput}
                      onChange={(e) => setManualFrequencyInput(e.target.value)}
                      min="0.1"
                      max="50"
                      step="0.1"
                      placeholder="2.0"
                    />
                    <button
                      className="btn primary text-xs px-2"
                      onClick={handleManualFrequencyUpdate}
                      disabled={configLoading || !connected}
                      title="Set manual frequency"
                    >
                      Set
                    </button>
                  </div>
                </div>
              ) : (
                <div className="mb-2">
                  <label className="text-xs text-gray-600 block mb-1">Operating Frequency (kHz):</label>
                  <div className="flex gap-1">
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
                      className="btn primary text-xs px-2"
                      onClick={handleFrequencyUpdate}
                      disabled={configLoading || !connected}
                      title="Update operating frequency"
                    >
                      Set
                    </button>
                  </div>
                  <div className="text-xs text-gray-500 mt-1">
                    Output: {configuration.sonicator4.operatingFrequencyKHz / 10}kHz (÷10)
                  </div>
                </div>
              )}

              <div className="flex items-center justify-between">
                <span className="text-xs text-gray-600">Status:</span>
                <button
                  className={`btn text-xs px-2 py-1 ${configuration.sonicator4.enabled ? 'btn-success' : 'btn-secondary'}`}
                  onClick={handleFrequencyToggle}
                  disabled={configLoading || !connected}
                  title={`${configuration.sonicator4.enabled ? 'Stop' : 'Start'} frequency generation`}
                >
                  {configuration.sonicator4.enabled ? 'ON' : 'OFF'}
                </button>
              </div>
            </div>

            <div className="text-xs text-gray-400 mt-2">
              {formatTimestamp(pinState.timestamp)}
            </div>
          </div>
        </div>
      )
    } else {
      // Enhanced design for all other pins
      const isHigh = pinState.state === 'HIGH'
      const isAnalog = typeof pinState.state === 'number'
      const isPWM = signal === 'AMPLITUDE_ALL' || (typeof pinState.state === 'string' && pinState.state.includes('%'))

      return (
        <div className="enhanced-pin-display">
          <div className={`pin-state enhanced ${isPWM ? 'pwm' : isHigh ? 'high' : isAnalog ? 'analog' : 'low'}`}>
            <div className="pin-value">
              {isPWM ? pinState.state : isAnalog ? `${pinState.state}` : pinState.state}
              {isAnalog && <span className="text-xs ml-1">ADC</span>}
              {isPWM && (
                <div className="pwm-indicator">
                  <div className="pwm-pulse-dot"></div>
                  <span className="text-xs text-gray-500 ml-1">PWM</span>
                </div>
              )}
            </div>

            <div className="pin-controls">
              {!isAnalog && !isPWM && (
                <button
                  className={`btn-mini ${isHigh ? 'btn-success' : 'btn-secondary'}`}
                  onClick={() => handlePinToggle(signal, pinState.state)}
                  disabled={!connected}
                  title={`Toggle ${signal} (currently ${pinState.state})`}
                >
                  Toggle
                </button>
              )}

              {isPWM && (
                <div className="pwm-monitoring-indicator">
                  <div className="pwm-live-dot"></div>
                  <span className="text-xs text-green-600 font-medium">Live Monitoring</span>
                </div>
              )}

              <div className="pin-indicator">
                <div className={`status-dot ${isPWM ? 'pwm' : isHigh ? 'high' : isAnalog ? 'analog' : 'low'}`}></div>
                <span className="text-xs text-gray-500">
                  {isPWM ? 'PWM' : isAnalog ? 'Analog' : (isHigh ? 'High' : 'Low')}
                </span>
              </div>
            </div>

            <div className="text-xs text-gray-400 mt-2">
              {formatTimestamp(pinState.timestamp)}
            </div>
          </div>
        </div>
      )
    }
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
        setManualFrequencyInput(data.config.sonicator4.manualFrequencyKHz?.toString() || '2.0')
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

  const handleManualModeToggle = async () => {
    setConfigLoading(true)
    try {
      const newManualMode = !configuration.sonicator4.manualMode
      const response = await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          config: {
            sonicator4: {
              manualMode: newManualMode
            }
          }
        })
      })

      const data = await response.json()
      if (data.success) {
        setConfiguration(data.config)
        console.log(`Manual mode ${newManualMode ? 'enabled' : 'disabled'}`)
      }
    } catch (error) {
      console.error('Error toggling manual mode:', error)
    } finally {
      setConfigLoading(false)
    }
  }

  const handleManualFrequencyUpdate = async () => {
    const frequency = parseFloat(manualFrequencyInput)

    // Validate frequency
    if (isNaN(frequency) || frequency <= 0 || frequency > 50) {
      alert('Manual frequency must be between 0.1 and 50 kHz')
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
              manualFrequencyKHz: frequency
            }
          }
        })
      })

      const data = await response.json()
      if (data.success) {
        setConfiguration(data.config)
        console.log(`Manual frequency updated to ${frequency}kHz`)
      } else {
        alert('Failed to update manual frequency')
      }
    } catch (error) {
      console.error('Error updating manual frequency:', error)
      alert('Network error updating manual frequency')
    } finally {
      setConfigLoading(false)
    }
  }

  return (
    <div className="control-panel">

      {/* Configurable Parameters */}
      <div className="control-section">
        <h3>Configurable Parameters</h3>
        <div className="text-xs text-gray-500 mb-4">
          Click to toggle HIGH/LOW states
        </div>

        {/* Sonicator #4 Header */}
        <div className="mb-4">
          <h4 className="sonicator-4-header text-lg font-semibold mb-3 pb-2 border-b border-blue-200">
            🔧 Sonicator #4
          </h4>



          {inputPins.map(([signal, pinState]) => {
            const pinInfo = pinDescriptions[signal]
            return (
              <div key={signal} className="mb-3 p-3 bg-gray-50 rounded-lg border">
                <div className="flex items-center justify-between mb-2">
                  <span className="text-sm font-mono font-semibold">{signal}</span>
                  <span className="text-xs text-gray-500">{pinState.pin}</span>
                </div>

                {pinInfo && (
                  <div className="mb-2 text-xs text-gray-600 space-y-1">
                    <div><strong>Parameter:</strong> {pinInfo.description}</div>
                    <div><strong>Connection:</strong> {pinInfo.wrapperPin} → {pinInfo.dutPin}, {pinInfo.physicalPin}</div>
                    <div className="text-gray-500">Arduino Test Wrapper Pin → ATmega32A Pin, Physical Pin</div>
                  </div>
                )}

                {renderPinState(signal, pinState)}
              </div>
            )
          })}
        </div>
      </div>
      {/* Live DUT Monitoring */}
      <div className="control-section">
        <h3>Live DUT Monitoring</h3>
        <div className="text-xs text-gray-500 mb-2">
          Real-time signals from ATmega32A
        </div>
        {outputPins.map(([signal, pinState]) => (
          <div key={signal} className="mb-2">
            <div className="flex items-center justify-between mb-1">
              <span className="text-xs font-mono">{signal}</span>
              <span className="text-xs text-gray-500">{pinState.pin}</span>
            </div>
            {renderPinState(signal, pinState)}
          </div>
        ))}
      </div>











      {/* System Status */}
      <div className="control-section">
        <h3>System Status</h3>
        <div className="text-xs text-gray-500 mb-2">
          Last update: {formatTimestamp(hardwareState.lastUpdate)}
        </div>
        <div className="text-xs">
          <div>Total pins: {Object.keys(hardwareState.pins).length}</div>
          <div>Configurable parameters: {inputPins.length}</div>
          <div>Live DUT signals: {outputPins.length}</div>
        </div>
      </div>

      {/* Help Section */}
      <div className="control-section">
        <h3>Quick Help</h3>
        <div className="text-xs text-gray-600 space-y-1">
          <div><strong>Sonicator #4 Parameters:</strong> Configure input signals with connection details</div>
          <div><strong>Connection Format:</strong> Arduino Pin → ATmega32A Pin, Physical Pin</div>
          <div><strong>Parameter Control:</strong> Click buttons to toggle HIGH/LOW states</div>

          <div className="mt-2 pt-2 border-t border-gray-200">
            <strong>Pin Mapping:</strong> Based on <code>docs/planning/pin-matrix.md</code>
          </div>
        </div>
      </div>
    </div>
  )
}
