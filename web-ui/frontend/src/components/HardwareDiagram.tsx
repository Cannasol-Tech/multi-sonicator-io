import React, { useState, useEffect, useRef } from 'react'
import { HardwareState } from '../types'

interface HardwareDiagramProps {
  hardwareState: HardwareState
  onPinClick: (signal: string, action: string, value?: any) => void
  highlightedPins?: string[]
  onPinHighlight?: (pins: string[]) => void
}

interface PinConnection {
  arduino: string
  atmega: string
  signal: string
  direction: 'IN' | 'OUT' | 'ANALOG' | 'COMM'
  description: string
  arduinoPos?: { x: number; y: number }
  atmegaPos?: { x: number; y: number }
  readonly?: boolean // Prevents manual control for communication pins
}

interface DetailedConnectionInfo {
  signal: string
  testHarnessPin: string
  dutPin: string
  direction: 'IN' | 'OUT' | 'ANALOG' | 'COMM'
  description: string
  purpose: string
  capabilities: string[]
  modbusRegister?: string
  valueRange?: string
  testingNotes: string
  readonly?: boolean
}

const PIN_CONNECTIONS: PinConnection[] = [
  { arduino: 'D7', atmega: 'PB0', signal: 'FREQ_DIV10_4', direction: 'OUT', description: 'Frequency ÷10 output (Configurable)' },
  { arduino: 'D8', atmega: 'PB4', signal: 'FREQ_LOCK_4', direction: 'IN', description: 'Frequency lock input' },
  { arduino: 'A2', atmega: 'PD3', signal: 'OVERLOAD_4', direction: 'IN', description: 'Overload input' },
  { arduino: 'A3', atmega: 'PC0', signal: 'START_4', direction: 'OUT', description: 'Start output' },
  { arduino: 'A4', atmega: 'PC1', signal: 'RESET_4', direction: 'OUT', description: 'Reset output' },
  { arduino: 'A1', atmega: 'PA7', signal: 'POWER_SENSE_4', direction: 'ANALOG', description: 'Sonicator #4 Power Level (Configurable)' },
  { arduino: 'D9', atmega: 'PD7', signal: 'AMPLITUDE_ALL', direction: 'OUT', description: 'Amplitude PWM output' },
  { arduino: 'D11', atmega: 'PD1', signal: 'UART_TXD', direction: 'COMM', description: 'UART TX (Modbus Communication)', readonly: true },
  { arduino: 'D12', atmega: 'PD2', signal: 'STATUS_LED', direction: 'OUT', description: 'Status LED' }
]

const DETAILED_CONNECTION_INFO: Record<string, DetailedConnectionInfo> = {
  'FREQ_DIV10_4': {
    signal: 'FREQ_DIV10_4',
    testHarnessPin: 'D7 (Digital Pin 7)',
    dutPin: 'PB0 (Port B, Pin 0)',
    direction: 'OUT',
    description: 'Frequency ÷10 output signal for Sonicator #4',
    purpose: 'Provides a divided frequency output signal from the sonicator to the test harness for frequency monitoring and validation.',
    capabilities: [
      'Read frequency value in Hz',
      'Monitor frequency changes in real-time',
      'Detect frequency lock status',
      'Configurable frequency division ratio'
    ],
    modbusRegister: '0x1004 (Frequency Register)',
    valueRange: '0 Hz - 50 kHz (typical sonicator range)',
    testingNotes: 'This signal allows the test harness to verify that the sonicator is operating at the correct frequency. The signal is divided by 10 for easier measurement by the Arduino.'
  },
  'FREQ_LOCK_4': {
    signal: 'FREQ_LOCK_4',
    testHarnessPin: 'D8 (Digital Pin 8)',
    dutPin: 'PB4 (Port B, Pin 4)',
    direction: 'IN',
    description: 'Frequency lock status input from Sonicator #4',
    purpose: 'Indicates when the sonicator has achieved and maintained frequency lock.',
    capabilities: [
      'Read lock status (HIGH = locked, LOW = unlocked)',
      'Monitor lock stability',
      'Trigger alerts on lock loss',
      'Integration with test automation'
    ],
    modbusRegister: '0x1008 (Status Register, Bit 0)',
    valueRange: 'Digital: HIGH (5V) = Locked, LOW (0V) = Unlocked',
    testingNotes: 'Critical for ensuring sonicator stability. Lock loss during operation may indicate hardware issues or improper tuning.'
  },
  'OVERLOAD_4': {
    signal: 'OVERLOAD_4',
    testHarnessPin: 'A2 (Analog Pin 2)',
    dutPin: 'PD3 (Port D, Pin 3)',
    direction: 'IN',
    description: 'Overload protection status input',
    purpose: 'Monitors the sonicator for overload conditions that could damage the equipment.',
    capabilities: [
      'Read overload status',
      'Monitor current levels',
      'Automatic shutdown on overload',
      'Overload history logging'
    ],
    modbusRegister: '0x1008 (Status Register, Bit 1)',
    valueRange: 'Digital: HIGH (5V) = Overload, LOW (0V) = Normal',
    testingNotes: 'Essential safety feature. Overload conditions should trigger immediate shutdown and investigation.'
  },
  'START_4': {
    signal: 'START_4',
    testHarnessPin: 'A3 (Analog Pin 3)',
    dutPin: 'PC0 (Port C, Pin 0)',
    direction: 'OUT',
    description: 'Start command output to Sonicator #4',
    purpose: 'Sends start/stop commands to control sonicator operation.',
    capabilities: [
      'Send start command (HIGH)',
      'Send stop command (LOW)',
      'Remote operation control',
      'Integration with test sequences'
    ],
    modbusRegister: '0x1010 (Control Register, Bit 0)',
    valueRange: 'Digital: HIGH (5V) = Start, LOW (0V) = Stop',
    testingNotes: 'Primary control signal for sonicator operation. Should be coordinated with other control signals for safe operation.'
  },
  'RESET_4': {
    signal: 'RESET_4',
    testHarnessPin: 'A4 (Analog Pin 4)',
    dutPin: 'PC1 (Port C, Pin 1)',
    direction: 'OUT',
    description: 'Reset command output to Sonicator #4',
    purpose: 'Provides hardware reset capability for error recovery.',
    capabilities: [
      'Send reset pulse',
      'Clear error conditions',
      'Force system restart',
      'Emergency stop function'
    ],
    modbusRegister: '0x1010 (Control Register, Bit 1)',
    valueRange: 'Digital: Pulse LOW (0V) for 100ms minimum',
    testingNotes: 'Use for error recovery and emergency stops. Reset should be followed by proper initialization sequence.'
  },
  'POWER_SENSE_4': {
    signal: 'POWER_SENSE_4',
    testHarnessPin: 'A1 (Analog Pin 1)',
    dutPin: 'PA7 (Port A, Pin 7)',
    direction: 'ANALOG',
    description: 'Power level sensing for Sonicator #4',
    purpose: 'Monitors the actual power output of the sonicator for feedback control.',
    capabilities: [
      'Read analog power level (0-1023 ADC)',
      'Convert to actual power values',
      'Power trend monitoring',
      'Calibration and validation'
    ],
    modbusRegister: '0x1020 (Power Register)',
    valueRange: '0-1023 ADC counts (0-5V), representing 0-100% power',
    testingNotes: 'Critical for power control validation. Should correlate with commanded power levels and actual sonicator output.'
  },
  'AMPLITUDE_ALL': {
    signal: 'AMPLITUDE_ALL',
    testHarnessPin: 'D9 (Digital Pin 9, PWM)',
    dutPin: 'PD7 (Port D, Pin 7)',
    direction: 'OUT',
    description: 'Amplitude control PWM output for all sonicators',
    purpose: 'Controls the amplitude/power level of all connected sonicators via PWM signal.',
    capabilities: [
      'Generate PWM signal (0-255)',
      'Control amplitude percentage',
      'Real-time amplitude adjustment',
      'Synchronized control of multiple units'
    ],
    modbusRegister: '0x1030 (Amplitude Register)',
    valueRange: 'PWM: 0-255 (0-100% duty cycle)',
    testingNotes: 'Master amplitude control. Changes affect all connected sonicators simultaneously. Monitor power feedback for validation.'
  },

  'UART_TXD': {
    signal: 'UART_TXD',
    testHarnessPin: 'D11 (Digital Pin 11)',
    dutPin: 'PD1 (Port D, Pin 1)',
    direction: 'COMM',
    description: 'UART Transmit Data for Modbus communication',
    purpose: 'Transmits Modbus RTU communication data to the sonicator system.',
    capabilities: [
      'Modbus RTU protocol communication',
      'Command transmission',
      'Parameter configuration',
      'Status requests'
    ],
    modbusRegister: 'N/A (Communication channel)',
    valueRange: 'Serial data: 0-5V TTL levels',
    testingNotes: 'Communication pin - not directly controllable. Used for Modbus protocol data exchange. Ensure proper termination and timing.',
    readonly: true
  },
  'STATUS_LED': {
    signal: 'STATUS_LED',
    testHarnessPin: 'D12 (Digital Pin 12)',
    dutPin: 'PD2 (Port D, Pin 2)',
    direction: 'OUT',
    description: 'Status LED control output',
    purpose: 'Controls external status LED for visual indication of system state.',
    capabilities: [
      'LED on/off control',
      'Visual status indication',
      'Error state signaling',
      'Test progress indication'
    ],
    modbusRegister: '0x1040 (LED Control Register)',
    valueRange: 'Digital: HIGH (5V) = LED On, LOW (0V) = LED Off',
    testingNotes: 'Useful for visual feedback during testing. Can indicate test progress, errors, or system status.'
  }
}

// Wire connection component for visual representation
const WireConnection: React.FC<{
  connection: PinConnection
  isHighlighted: boolean
  pinState: any
  containerRef: React.RefObject<HTMLDivElement | null>
}> = ({ connection, isHighlighted, pinState, containerRef }) => {
  const [path, setPath] = useState('')

  useEffect(() => {
    if (!containerRef.current) return

    const container = containerRef.current
    const arduinoElement = container.querySelector(`[data-pin="${connection.arduino}"]`)
    const atmegaElement = container.querySelector(`[data-pin="${connection.atmega}"]`)

    if (arduinoElement && atmegaElement) {
      const containerRect = container.getBoundingClientRect()
      const arduinoRect = arduinoElement.getBoundingClientRect()
      const atmegaRect = atmegaElement.getBoundingClientRect()

      const startX = arduinoRect.right - containerRect.left
      const startY = arduinoRect.top + arduinoRect.height / 2 - containerRect.top
      const endX = atmegaRect.left - containerRect.left
      const endY = atmegaRect.top + atmegaRect.height / 2 - containerRect.top

      // Create curved path for wire appearance
      const midX = (startX + endX) / 2
      const curveOffset = Math.abs(endY - startY) * 0.3
      const controlY1 = startY + (startY < endY ? curveOffset : -curveOffset)
      const controlY2 = endY + (endY < startY ? curveOffset : -curveOffset)

      const pathData = `M ${startX} ${startY} C ${midX} ${controlY1}, ${midX} ${controlY2}, ${endX} ${endY}`
      setPath(pathData)
    }
  }, [connection, containerRef])

  const getWireColor = () => {
    if (isHighlighted) return '#ffd700' // Bright shining gold
    if (connection.direction === 'COMM') return '#d97706' // Orange for communication pins
    if (connection.direction === 'ANALOG') return '#8b5cf6'
    if (connection.direction === 'IN') return pinState?.state === 'HIGH' ? '#ef4444' : '#6b7280'
    if (connection.direction === 'OUT') return pinState?.state === 'HIGH' ? '#10b981' : '#6b7280'
    return '#6b7280'
  }

  const getWireWidth = () => {
    return isHighlighted ? 4 : 2
  }

  return (
    <path
      d={path}
      stroke={getWireColor()}
      strokeWidth={getWireWidth()}
      fill="none"
      strokeDasharray={connection.direction === 'ANALOG' ? "5,5" : connection.direction === 'COMM' ? "10,5" : "none"}
      className={`wire-connection ${isHighlighted ? 'highlighted' : ''}`}
      style={{
        filter: isHighlighted ? 'drop-shadow(0 0 12px rgba(255, 215, 0, 0.9)) drop-shadow(0 0 24px rgba(255, 215, 0, 0.6))' : 'none',
        transition: 'all 0.3s ease'
      }}
    />
  )
}

export const HardwareDiagram: React.FC<HardwareDiagramProps> = ({
  hardwareState,
  onPinClick,
  highlightedPins = [],
  onPinHighlight
}) => {
  const containerRef = useRef<HTMLDivElement>(null)
  const [hoveredConnection, setHoveredConnection] = useState<string | null>(null)
  const [selectedConnection, setSelectedConnection] = useState<string | null>(null)
  const [showDetailedModal, setShowDetailedModal] = useState(false)



  // Connection hover handler with enhanced feedback
  const handleConnectionHover = (signal: string | null) => {
    setHoveredConnection(signal)
    if (signal) {
      // Add subtle haptic feedback if supported
      if (navigator.vibrate) {
        navigator.vibrate(10)
      }
    }
  }

  // Connection click handler for persistent selection
  const handleConnectionClick = (signal: string) => {
    if (selectedConnection === signal) {
      // Clicking the same connection deselects it
      setSelectedConnection(null)
    } else {
      // Select the new connection
      setSelectedConnection(signal)
      // Add stronger haptic feedback for selection
      if (navigator.vibrate) {
        navigator.vibrate([50, 50, 50])
      }
    }
  }

  // Remove the old WebSocket setup since it's now handled by props

  const handlePinClick = (signal: string, pinState: any) => {
    console.log('Pin clicked:', signal, 'Current state:', pinState.state, 'Direction:', pinState.direction)

    // Handle connection selection first
    handleConnectionClick(signal)

    // Check if this is a readonly communication pin
    const connection = PIN_CONNECTIONS.find(c => c.signal === signal)
    if (connection?.readonly) {
      console.log('Pin', signal, 'is readonly (communication pin) - no manual control allowed')
      return
    }

    // Enhanced highlighting: only one connection can be highlighted at a time
    if (onPinHighlight) {
      const currentlyHighlighted = highlightedPins.includes(signal)
      if (currentlyHighlighted) {
        // If already highlighted, remove highlighting
        onPinHighlight([])
      } else {
        // If not highlighted, set this as the only highlighted connection
        onPinHighlight([signal])
      }
    }

    // Execute the hardware command (only for non-readonly pins)
    if (pinState.direction === 'IN') {
      onPinClick(signal, 'toggle')
    } else {
      onPinClick(signal, 'read')
    }
  }

  // Get enhanced pin state with additional visual information
  const getEnhancedPinState = (signal: string) => {
    const state = getPinState(signal)
    const connection = PIN_CONNECTIONS.find(c => c.signal === signal)
    const isActive = highlightedPins.includes(signal) ||
                    hoveredConnection === signal ||
                    selectedConnection === signal

    return {
      ...state,
      connection,
      isActive,
      isSelected: selectedConnection === signal,
      lastUpdated: (state as any).timestamp ? new Date((state as any).timestamp).toLocaleTimeString() : 'Unknown'
    }
  }

  const getPinState = (signal: string) => {
    const state = hardwareState.pins[signal]
    if (!state) return { pin: '?', direction: 'UNKNOWN', state: 'UNKNOWN' }
    return state
  }



  return (
    <div className="hardware-diagram-enhanced" ref={containerRef}>
      {/* SVG Overlay for Wire Connections */}
      <svg className="wire-overlay" style={{
        position: 'absolute',
        top: 0,
        left: 0,
        width: '100%',
        height: '100%',
        pointerEvents: 'none',
        zIndex: 1
      }}>
        {PIN_CONNECTIONS.map(connection => {
          const enhancedState = getEnhancedPinState(connection.signal)
          return (
            <WireConnection
              key={connection.signal}
              connection={connection}
              isHighlighted={enhancedState.isActive}
              pinState={enhancedState}
              containerRef={containerRef}
            />
          )
        })}
      </svg>

      {/* Main Hardware Layout - Centered and Enlarged */}
      <div className="hardware-layout-centered">
        <div className="devices-row-enlarged">
          {/* Arduino Test Wrapper */}
          <div className="device-container-large arduino-container">
            <div className="device-header">
              <h3>Arduino Uno R3</h3>
              <span className="device-subtitle">HIL Test Wrapper</span>
            </div>
            <div className="device-image-container-large">
              <img
                src="/arduino-uno-r3-icon.png"
                alt="Arduino Uno R3"
                className="device-image-large arduino-image"
              />
              <div className="device-pins-large arduino-pins">
                {PIN_CONNECTIONS.map(connection => {
                  const enhancedState = getEnhancedPinState(connection.signal)
                  return (
                    <div
                      key={connection.arduino}
                      data-pin={connection.arduino}
                      className={`pin-indicator-large arduino-pin ${enhancedState.isActive ? 'highlighted' : ''} ${enhancedState.isSelected ? 'selected' : ''} ${connection.readonly ? 'readonly' : ''}`}
                      onMouseEnter={() => handleConnectionHover(connection.signal)}
                      onMouseLeave={() => handleConnectionHover(null)}
                      onClick={() => handlePinClick(connection.signal, enhancedState)}
                      title={`${connection.arduino} → ${connection.atmega} (${connection.signal})\n${connection.description}\n${connection.readonly ? 'READONLY - Communication Pin' : 'Last updated: ' + enhancedState.lastUpdated}`}
                      style={{ cursor: connection.readonly ? 'not-allowed' : 'pointer' }}
                    >
                      <span className="pin-label-large">{connection.arduino}</span>
                      <div className={`pin-status-large ${connection.direction.toLowerCase()}`}>
                        {enhancedState.state}
                      </div>
                    </div>
                  )
                })}
              </div>
            </div>
          </div>

          {/* Pin Mapping Display - Center Between Devices */}
          <div className="compact-pin-mapping-center">
            <div className="pin-mapping-header">
              <h4>🔌 Pin Connections</h4>
              <div className="mapping-subtitle">Arduino Test Wrapper ↔ ATmega32A DUT</div>
            </div>
            <div className="compact-pin-list">
              {PIN_CONNECTIONS.map(connection => {
                const enhancedState = getEnhancedPinState(connection.signal)
                const direction = connection.direction === 'IN' ? 'IN' : 'OUT'

                // Format current value for display
                const formatCurrentValue = (signal: string, state: any) => {
                  if (signal === 'FREQ_DIV10_4') {
                    if (typeof state === 'string' && state.includes('Hz')) {
                      return state
                    }
                    return state || 'OFF'
                  } else if (signal.includes('POWER_SENSE')) {
                    if (typeof state === 'number') {
                      return `${state} ADC`
                    }
                    return state || 'UNKNOWN'
                  } else {
                    return state || 'UNKNOWN'
                  }
                }

                const currentValue = formatCurrentValue(connection.signal, enhancedState.state)

                return (
                  <div
                    key={connection.signal}
                    className={`compact-pin-row ${enhancedState.isActive ? 'active' : ''} ${enhancedState.isSelected ? 'selected' : ''} ${connection.readonly ? 'readonly' : ''}`}
                    onMouseEnter={() => handleConnectionHover(connection.signal)}
                    onMouseLeave={() => handleConnectionHover(null)}
                    onClick={() => handlePinClick(connection.signal, enhancedState)}
                  >
                    <span className="wrapper-pin-badge">{connection.arduino}</span>
                    <span className="direction-badge">[{direction}]</span>
                    <span className="signal-name">{connection.signal}</span>
                    <span className="arrow">→</span>
                    <span className="dut-pin-badge">{connection.atmega}</span>
                    <span className="current-value-badge">[{currentValue}]</span>
                  </div>
                )
              })}
            </div>
          </div>

          {/* ATmega32A DUT */}
          <div className="device-container-large atmega-container">
            <div className="device-header">
              <h3>ATmega32A</h3>
              <span className="device-subtitle">Device Under Test</span>
            </div>
            <div className="device-image-container-large">
              <img
                src="/atmega-32-a-icon.png"
                alt="ATmega32A"
                className="device-image-large atmega-image"
              />
              <div className="device-pins-large atmega-pins">
                {PIN_CONNECTIONS.map(connection => {
                  const enhancedState = getEnhancedPinState(connection.signal)
                  return (
                    <div
                      key={connection.atmega}
                      data-pin={connection.atmega}
                      className={`pin-indicator-large atmega-pin ${enhancedState.isActive ? 'highlighted' : ''} ${enhancedState.isSelected ? 'selected' : ''}`}
                      onMouseEnter={() => handleConnectionHover(connection.signal)}
                      onMouseLeave={() => handleConnectionHover(null)}
                      onClick={() => handlePinClick(connection.signal, enhancedState)}
                      title={`${connection.atmega} ← ${connection.arduino} (${connection.signal})\n${connection.description}\nLast updated: ${enhancedState.lastUpdated}`}
                      style={{ cursor: 'pointer' }}
                    >
                      <span className="pin-label-large">{connection.atmega}</span>
                      <div className={`pin-status-large ${connection.direction.toLowerCase()}`}>
                        {enhancedState.state}
                      </div>
                    </div>
                  )
                })}
              </div>
            </div>
            <div className="device-status">
              <div className={`status-indicator ${hardwareState.connection.connected ? 'connected' : 'disconnected'}`}></div>
              <span>{hardwareState.connection.connected ? 'Active' : 'Disconnected'}</span>
            </div>
          </div>
        </div>

        {/* Compact Connection Information Section */}
        {selectedConnection && DETAILED_CONNECTION_INFO[selectedConnection] && (
          <div className="connection-details-compact">
            {(() => {
              const info = DETAILED_CONNECTION_INFO[selectedConnection]
              return (
                <div className="compact-info-container">
                  <div className="compact-info-header">
                    <div className="compact-signal-info">
                      <span className="compact-signal-name">{info.signal}</span>
                      <span className={`compact-direction-badge ${info.direction.toLowerCase()}`}>
                        {info.direction}
                      </span>
                    </div>
                    <button
                      className="expand-details-button"
                      onClick={() => setShowDetailedModal(true)}
                    >
                      📋 View Details
                    </button>
                  </div>

                  <div className="compact-info-content">
                    <div className="compact-info-row">
                      <span className="compact-label">Pins:</span>
                      <span className="compact-value">
                        {info.testHarnessPin.split(' ')[0]} ↔ {info.dutPin.split(' ')[0]}
                      </span>
                    </div>

                    <div className="compact-info-row">
                      <span className="compact-label">Purpose:</span>
                      <span className="compact-value">{info.description}</span>
                    </div>

                    {info.modbusRegister && (
                      <div className="compact-info-row">
                        <span className="compact-label">Modbus:</span>
                        <span className="compact-value modbus-compact">{info.modbusRegister}</span>
                      </div>
                    )}

                    <div className="compact-info-row">
                      <span className="compact-label">Range:</span>
                      <span className="compact-value">{info.valueRange}</span>
                    </div>
                  </div>
                </div>
              )
            })()}
          </div>
        )}

        {/* Detailed Connection Modal */}
        {showDetailedModal && selectedConnection && DETAILED_CONNECTION_INFO[selectedConnection] && (
          <div className="connection-modal-overlay" onClick={() => setShowDetailedModal(false)}>
            <div className="connection-modal-content" onClick={(e) => e.stopPropagation()}>
              {(() => {
                const info = DETAILED_CONNECTION_INFO[selectedConnection]
                return (
                  <>
                    <div className="modal-header">
                      <h2>📋 {info.signal} - Detailed Information</h2>
                      <button
                        className="modal-close-button"
                        onClick={() => setShowDetailedModal(false)}
                      >
                        ✕
                      </button>
                    </div>

                    <div className="modal-body">
                      <div className="modal-info-grid">
                        {/* Signal Overview */}
                        <div className="modal-section signal-overview">
                          <h3>🔌 Signal Overview</h3>
                          <div className="modal-info-item">
                            <span className="modal-label">Signal Name:</span>
                            <span className="modal-value signal-name">{info.signal}</span>
                          </div>
                          <div className="modal-info-item">
                            <span className="modal-label">Direction:</span>
                            <span className={`modal-value direction-badge ${info.direction.toLowerCase()}`}>
                              {info.direction}
                            </span>
                          </div>
                          <div className="modal-info-item">
                            <span className="modal-label">Description:</span>
                            <span className="modal-value">{info.description}</span>
                          </div>
                          <div className="modal-info-item">
                            <span className="modal-label">Purpose:</span>
                            <span className="modal-value">{info.purpose}</span>
                          </div>
                        </div>

                        {/* Pin Connections */}
                        <div className="modal-section pin-connections">
                          <h3>📍 Pin Connections</h3>
                          <div className="modal-pin-connection-details">
                            <div className="modal-pin-detail">
                              <span className="modal-pin-label">Test Harness:</span>
                              <span className="modal-pin-value arduino-pin">{info.testHarnessPin}</span>
                            </div>
                            <div className="modal-connection-arrow">↔</div>
                            <div className="modal-pin-detail">
                              <span className="modal-pin-label">ATmega32A DUT:</span>
                              <span className="modal-pin-value atmega-pin">{info.dutPin}</span>
                            </div>
                          </div>
                        </div>

                        {/* Technical Details */}
                        <div className="modal-section technical-details">
                          <h3>⚙️ Technical Details</h3>
                          {info.modbusRegister && (
                            <div className="modal-info-item">
                              <span className="modal-label">Modbus Register:</span>
                              <span className="modal-value modbus-register">{info.modbusRegister}</span>
                            </div>
                          )}
                          <div className="modal-info-item">
                            <span className="modal-label">Value Range:</span>
                            <span className="modal-value">{info.valueRange}</span>
                          </div>
                          {info.readonly && (
                            <div className="modal-info-item readonly-warning">
                              <span className="modal-label">⚠️ Access:</span>
                              <span className="modal-value">Read-Only (Communication Pin)</span>
                            </div>
                          )}
                        </div>

                        {/* Capabilities */}
                        <div className="modal-section capabilities">
                          <h3>🛠️ UI Capabilities</h3>
                          <ul className="modal-capabilities-list">
                            {info.capabilities.map((capability, index) => (
                              <li key={index} className="modal-capability-item">
                                <span className="modal-capability-bullet">•</span>
                                <span className="modal-capability-text">{capability}</span>
                              </li>
                            ))}
                          </ul>
                        </div>

                        {/* Testing Notes */}
                        <div className="modal-section testing-notes">
                          <h3>📝 Testing Notes</h3>
                          <div className="modal-testing-notes-content">
                            {info.testingNotes}
                          </div>
                        </div>
                      </div>
                    </div>
                  </>
                )
              })()}
            </div>
          </div>
        )}
      </div>
    </div>
  )
}

export default HardwareDiagram
