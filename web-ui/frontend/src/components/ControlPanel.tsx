import { useState, useEffect } from 'react'
import { HardwareState } from '../types'
import { formatFrequency } from '../utils/formatters'

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
      description: 'Sonicator #4 Frequency Lock Input (Control Signal)',
      wrapperPin: 'D8',
      dutPin: 'PB4',
      physicalPin: '5'
    },
    'OVERLOAD_4': {
      description: 'Sonicator #4 Overload Input (Control Signal)',
      wrapperPin: 'A2',
      dutPin: 'PD3',
      physicalPin: '17'
    },
    'START_4': {
      description: 'Sonicator #4 Start Status Output',
      wrapperPin: 'A3',
      dutPin: 'PC0',
      physicalPin: '22'
    },
    'RESET_4': {
      description: 'Sonicator #4 Reset Status Output',
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
      wrapperPin: 'D4',
      dutPin: 'PD2',
      physicalPin: '16'
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

  // Control signals that can be manipulated in "Configurable Parameters" tab
  // These are inputs TO the DUT that we can control from the harness
  const controlPins = Object.entries(hardwareState.pins).filter(([signal, pin]) => 
    signal === 'FREQ_DIV10_4' || signal === 'FREQ_LOCK_4' || signal === 'OVERLOAD_4'
  )
  
  // Monitoring signals that are read-only in "Live DUT Monitoring" tab
  // These are outputs FROM the DUT that we can only monitor
  const monitoringPins = Object.entries(hardwareState.pins).filter(([signal, pin]) => 
    signal === 'START_4' || signal === 'RESET_4' || signal === 'AMPLITUDE_ALL' || 
    signal === 'STATUS_LED' || signal === 'POWER_SENSE_4'
  )

  const handlePinToggle = (signal: string, currentState: any) => {
    // Only allow toggling control signals (inputs to the DUT)
    // Don't allow toggling frequency pins as they have their own controls
    if (signal === 'FREQ_DIV10_4') {
      return // Frequency controlled via input field
    }
    
    // Toggle the digital control signals
    const newState = currentState === 'HIGH' ? 'LOW' : 'HIGH'
    onPinControl(signal, 'write_pin', newState)
  }

  const handleFrequencyChange = async (newFrequencyKHz: number) => {
    try {
      setConfigLoading(true)
      // Clamp to 18–22 kHz range for CT2000 sonicators
      const clamped = Math.min(22, Math.max(18, newFrequencyKHz))
      const updatedConfig = {
        ...configuration,
        sonicator4: {
          ...configuration.sonicator4,
          operatingFrequencyKHz: clamped,
          outputFrequencyKHz: clamped / 10 // FREQ_DIV10 is frequency divided by 10
        }
      }
      
      // Update local state immediately for UI responsiveness
      setConfiguration(updatedConfig)
      
      // Send to backend
      onPinControl('FREQ_DIV10_4', 'set_frequency', clamped)
      
      // Also save configuration
      const response = await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ config: updatedConfig })
      })
      
      if (!response.ok) {
        throw new Error('Failed to save configuration')
      }
    } catch (error) {
      console.error('Failed to update frequency:', error)
      // Reload configuration on error
      loadConfiguration()
    } finally {
      setConfigLoading(false)
    }
  }

  const handleFrequencyInputChange = (newFrequency: number) => {
    if (!isNaN(newFrequency)) {
      // Allow immediate input but clamp in handler
      handleFrequencyChange(newFrequency)
    }
  }

  const isFrequencyPin = (signal: string) => {
    return signal === 'FREQ_DIV10_4'
  }

  const renderPinState = (signal: string, pinState: any, isMonitoringMode: boolean = false) => {
    if (isFrequencyPin(signal)) {
      return (
        <div className="enhanced-pin-display">
          <div className="pin-state enhanced analog">
            <div className="pin-value">
              {(() => {
                if (!pinState.state) return formatFrequency(0);
                const stateStr = pinState.state.toString();
                // If already contains Hz/kHz units, extract the numeric part
                if (stateStr.includes('Hz') || stateStr.includes('kHz')) {
                  const numericPart = stateStr.replace(/[^0-9.]/g, '');
                  const frequencyValue = parseFloat(numericPart) || 0;
                  // If the original contained 'kHz', multiply by 1000 to get Hz
                  const frequencyHz = stateStr.includes('kHz') ? frequencyValue * 1000 : frequencyValue;
                  return formatFrequency(frequencyHz);
                }
                // Otherwise treat as raw frequency in Hz
                const rawFreq = parseFloat(stateStr) || 0;
                return formatFrequency(rawFreq);
              })()
              }
            </div>

            <div className="pin-controls">
              <div className="pin-indicator">
                <div className="status-dot analog"></div>
                <span className="text-xs" style={{ color: 'var(--text-secondary)' }}>Frequency</span>
              </div>
            </div>

            {/* Frequency control - only show in control mode */}
            {!isMonitoringMode && (
              <div className="pin-controls">
                <input
                  type="number"
                  min="18"
                  max="22"
                  step="0.1"
                  value={configuration.sonicator4.operatingFrequencyKHz}
                  onChange={(e) => handleFrequencyInputChange(parseFloat(e.target.value))}
                  disabled={!connected || configLoading}
                  className="btn-mini"
                  style={{ 
                    width: '80px',
                    textAlign: 'center',
                    fontFamily: 'monospace'
                  }}
                  title="Frequency (kHz)"
                />
                <span className="text-xs" style={{ color: 'var(--text-secondary)' }}>kHz</span>
              </div>
            )}

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
              {!isAnalog && !isPWM && !isMonitoringMode && (
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
            <div className="parameter-grid">
              {controlPins.map(([signal, pinState]) => {
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

                {renderPinState(signal, pinState, false)}
              </div>
              )
            })}
            </div>
          </div>
        )}

        {activeSubTab === 'monitoring' && (
          <div className="control-section">
            <div className="parameter-grid">
            {monitoringPins.map(([signal, pinState]) => {
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

                  {renderPinState(signal, pinState, true)}
                </div>
              )
            })}
            </div>
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
          <div>Configurable parameters: {controlPins.length}</div>
          <div>Live DUT signals: {monitoringPins.length}</div>
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
