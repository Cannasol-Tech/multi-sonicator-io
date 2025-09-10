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
      description: 'Sonicator #4 Frequency Output (output as Hz/10)',
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
  const [activeSubTab, setActiveSubTab] = useState<'parameters' | 'monitoring'>('parameters')

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
        <div className="enhanced-pin-display">
          <div className="pin-state enhanced analog">
            <div className="pin-value">
              {pinState.state || '0Hz'}
            </div>

            <div className="pin-controls">
              <div className="pin-indicator">
                <div className="status-dot analog"></div>
                <span className="text-xs" style={{ color: 'var(--text-secondary)' }}>Frequency</span>
              </div>
            </div>

            <div className="pin-details">
              <div className="flex items-center justify-between mb-1">
                <span className="text-xs font-medium" style={{ color: 'var(--text-secondary)' }}>Configured:</span>
                <span className="text-xs font-mono font-bold" style={{ color: 'var(--color-primary)' }}>
                  {configuration.sonicator4.operatingFrequencyKHz}kHz
                </span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs font-medium" style={{ color: 'var(--text-secondary)' }}>Current:</span>
                <span className="text-xs font-mono font-bold" style={{ color: 'var(--text-primary)' }}>
                  {pinState.state || '0Hz'}
                </span>
              </div>
            </div>

            <div className="text-xs mt-2" style={{ color: 'var(--text-tertiary)' }}>
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
                  <span className="text-xs ml-1" style={{ color: 'var(--text-secondary)' }}>PWM</span>
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
                  <span className="text-xs font-medium" style={{ color: 'var(--color-success)' }}>Live Monitoring</span>
                </div>
              )}

              <div className="pin-indicator">
                <div className={`status-dot ${isPWM ? 'pwm' : isHigh ? 'high' : isAnalog ? 'analog' : 'low'}`}></div>
                <span className="text-xs" style={{ color: 'var(--text-secondary)' }}>
                  {isPWM ? 'PWM' : isAnalog ? 'Analog' : (isHigh ? 'High' : 'Low')}
                </span>
              </div>
            </div>

            <div className="text-xs mt-2" style={{ color: 'var(--text-tertiary)' }}>
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
      }
    } catch (error) {
      console.error('Failed to load configuration:', error)
    }
  }









  return (
    <div className="control-panel">
      {/* Sub-tab Navigation */}
      <div className="sub-tab-navigation">
        <button
          className={`sub-tab-button ${activeSubTab === 'parameters' ? 'active' : ''}`}
          onClick={() => setActiveSubTab('parameters')}
        >
          🔧 Configurable Parameters
        </button>
        <button
          className={`sub-tab-button ${activeSubTab === 'monitoring' ? 'active' : ''}`}
          onClick={() => setActiveSubTab('monitoring')}
        >
          📊 Live DUT Monitoring
        </button>
      </div>

      {/* Sub-tab Content */}
      <div className="sub-tab-content">
        {activeSubTab === 'parameters' && (
          <div className="control-section">
          {inputPins.map(([signal, pinState]) => {
            const pinInfo = pinDescriptions[signal]
            return (
              <div key={signal} className="parameter-card-compact mb-3 p-3 rounded-lg" style={{
                background: 'var(--bg-secondary)',
                border: '1px solid var(--border-color)',
                transition: 'all 0.3s ease'
              }}>
                {/* Sonicator #4 identifier and Parameter Description */}
                <div className="parameter-header mb-2">
                  <div className="flex items-center gap-2 mb-1">
                    <span className="text-sm font-semibold" style={{ color: 'var(--color-primary)' }}>
                      🔧 Sonicator #4
                    </span>
                    <span className="text-xs px-2 py-1 rounded" style={{
                      background: 'var(--bg-tertiary)',
                      color: 'var(--text-secondary)',
                      fontSize: '10px'
                    }}>
                      Signal Configuration
                    </span>
                  </div>
                  <h5 className="text-sm font-bold mb-1" style={{ color: 'var(--text-primary)' }}>
                    {pinInfo ? pinInfo.description : signal}
                  </h5>

                  {/* Compact info row: Signal name and Connection */}
                  <div className="parameter-info-row flex items-center justify-between text-xs" style={{ color: 'var(--text-secondary)' }}>
                    <span className="signal-name font-mono" style={{ color: 'var(--text-primary)' }}>
                      {signal}
                    </span>
                    {pinInfo && (
                      <span className="connection-info" style={{ color: 'var(--text-tertiary)' }}>
                        {pinInfo.wrapperPin} → {pinInfo.dutPin}, Pin {pinInfo.physicalPin}
                      </span>
                    )}
                  </div>
                </div>

                {renderPinState(signal, pinState)}
              </div>
            )
          })}
          </div>
        )}

        {activeSubTab === 'monitoring' && (
          <div className="control-section">
            {outputPins.map(([signal, pinState]) => {
              const pinInfo = pinDescriptions[signal]
              return (
                <div key={signal} className="parameter-card-compact mb-3 p-3 rounded-lg" style={{
                  background: 'var(--bg-secondary)',
                  border: '1px solid var(--border-color)',
                  transition: 'all 0.3s ease'
                }}>
                  {/* Sonicator #4 identifier and Signal Description */}
                  <div className="parameter-header mb-2">
                    <div className="flex items-center gap-2 mb-1">
                      <span className="text-sm font-semibold" style={{ color: 'var(--color-primary)' }}>
                        🔊 Sonicator #4
                      </span>
                      <span className="text-xs px-2 py-1 rounded" style={{
                        background: 'var(--bg-tertiary)',
                        color: 'var(--text-secondary)',
                        fontSize: '10px'
                      }}>
                        Live Monitoring
                      </span>
                    </div>
                    <h5 className="text-sm font-bold mb-1" style={{ color: 'var(--text-primary)' }}>
                      {pinInfo ? pinInfo.description : signal}
                    </h5>

                    {/* Compact info row: Signal name and Connection */}
                    <div className="parameter-info-row flex items-center justify-between text-xs" style={{ color: 'var(--text-secondary)' }}>
                      <span className="signal-name font-mono" style={{ color: 'var(--text-primary)' }}>
                        {signal}
                      </span>
                      {pinInfo && (
                        <span className="connection-info" style={{ color: 'var(--text-tertiary)' }}>
                          {pinInfo.wrapperPin} → {pinInfo.dutPin}, Pin {pinInfo.physicalPin}
                        </span>
                      )}
                    </div>
                  </div>

                  {renderPinState(signal, pinState)}
                </div>
              )
            })}
          </div>
        )}
      </div>

      {/* System Status */}
      <div className="control-section">
        <h3>System Status</h3>
        <div className="text-xs mb-2" style={{ color: 'var(--text-secondary)' }}>
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
        <div className="text-xs space-y-1" style={{ color: 'var(--text-secondary)' }}>
          <div><strong style={{ color: 'var(--text-primary)' }}>Configurable Parameters:</strong> Control signals sent to ATmega32A with detailed pin mapping</div>
          <div><strong style={{ color: 'var(--text-primary)' }}>Live DUT Monitoring:</strong> Real-time digital & PWM signals from ATmega32A</div>
          <div><strong style={{ color: 'var(--text-primary)' }}>Connection Format:</strong> Arduino Pin → ATmega32A Pin, Physical Pin Number</div>

          <div className="mt-2 pt-2" style={{ borderTop: '1px solid var(--border-color)' }}>
            <strong style={{ color: 'var(--text-primary)' }}>Pin Mapping:</strong> Based on <code style={{
              background: 'var(--bg-tertiary)',
              padding: '2px 4px',
              borderRadius: '3px',
              color: 'var(--text-primary)'
            }}>docs/planning/pin-matrix.md</code>
          </div>
        </div>
      </div>
    </div>
  )
}
