import React, { useState, useEffect, useRef } from 'react'
import { HardwareState } from '../types'

interface HardwareDiagramProps {
  hardwareState: HardwareState
  onPinClick: (signal: string, action: string, value?: any) => void
  highlightedPins?: string[]
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

const PIN_CONNECTIONS: PinConnection[] = [
  { arduino: 'D7', atmega: 'PB0', signal: 'FREQ_DIV10_4', direction: 'OUT', description: 'Frequency ÷10 output (Configurable)' },
  { arduino: 'D8', atmega: 'PB4', signal: 'FREQ_LOCK_4', direction: 'IN', description: 'Frequency lock input' },
  { arduino: 'A2', atmega: 'PD3', signal: 'OVERLOAD_4', direction: 'IN', description: 'Overload input' },
  { arduino: 'A3', atmega: 'PC0', signal: 'START_4', direction: 'OUT', description: 'Start output' },
  { arduino: 'A4', atmega: 'PC1', signal: 'RESET_4', direction: 'OUT', description: 'Reset output' },
  { arduino: 'A1', atmega: 'PA7', signal: 'POWER_SENSE_4', direction: 'ANALOG', description: 'Sonicator #4 Power Level (Configurable)' },
  { arduino: 'D9', atmega: 'PD7', signal: 'AMPLITUDE_ALL', direction: 'OUT', description: 'Amplitude PWM output' },
  { arduino: 'D10', atmega: 'PD0', signal: 'UART_RXD', direction: 'COMM', description: 'UART RX (Modbus Communication)', readonly: true },
  { arduino: 'D11', atmega: 'PD1', signal: 'UART_TXD', direction: 'COMM', description: 'UART TX (Modbus Communication)', readonly: true },
  { arduino: 'D12', atmega: 'PD2', signal: 'STATUS_LED', direction: 'OUT', description: 'Status LED' }
]

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
    if (isHighlighted) return '#f59e0b'
    if (connection.direction === 'COMM') return '#d97706' // Orange for communication pins
    if (connection.direction === 'ANALOG') return '#8b5cf6'
    if (connection.direction === 'IN') return pinState?.state === 'HIGH' ? '#ef4444' : '#6b7280'
    if (connection.direction === 'OUT') return pinState?.state === 'HIGH' ? '#10b981' : '#6b7280'
    return '#6b7280'
  }

  return (
    <path
      d={path}
      stroke={getWireColor()}
      strokeWidth={isHighlighted ? "3" : "2"}
      fill="none"
      strokeDasharray={connection.direction === 'ANALOG' ? "5,5" : connection.direction === 'COMM' ? "10,5" : "none"}
      className={`wire-connection ${isHighlighted ? 'highlighted' : ''}`}
      style={{
        filter: isHighlighted ? 'drop-shadow(0 0 4px currentColor)' : 'none',
        transition: 'all 0.3s ease'
      }}
    />
  )
}

export const HardwareDiagram: React.FC<HardwareDiagramProps> = ({
  hardwareState,
  onPinClick,
  highlightedPins = []
}) => {
  const containerRef = useRef<HTMLDivElement>(null)
  const [hoveredConnection, setHoveredConnection] = useState<string | null>(null)



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

  // Remove the old WebSocket setup since it's now handled by props

  const handlePinClick = (signal: string, pinState: any) => {
    console.log('Pin clicked:', signal, 'Current state:', pinState.state, 'Direction:', pinState.direction)

    // Check if this is a readonly communication pin
    const connection = PIN_CONNECTIONS.find(c => c.signal === signal)
    if (connection?.readonly) {
      console.log('Pin', signal, 'is readonly (communication pin) - no manual control allowed')
      return
    }

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
                    hoveredConnection === signal

    return {
      ...state,
      connection,
      isActive,
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
                      className={`pin-indicator-large arduino-pin ${enhancedState.isActive ? 'highlighted' : ''} ${connection.readonly ? 'readonly' : ''}`}
                      onMouseEnter={() => handleConnectionHover(connection.signal)}
                      onMouseLeave={() => handleConnectionHover(null)}
                      onClick={() => !connection.readonly && handlePinClick(connection.signal, enhancedState)}
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
                      className={`pin-indicator-large atmega-pin ${enhancedState.isActive ? 'highlighted' : ''}`}
                      onMouseEnter={() => handleConnectionHover(connection.signal)}
                      onMouseLeave={() => handleConnectionHover(null)}
                      title={`${connection.atmega} ← ${connection.arduino} (${connection.signal})\n${connection.description}\nLast updated: ${enhancedState.lastUpdated}`}
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

        {/* Pin Mapping Display - Front and Center */}
        <div className="pin-mapping-display-prominent">
          <div className="pin-mapping-header-large">
            <h2>🔌 Pin Connections & Real-Time Status</h2>
            <div className="mapping-subtitle-large">Arduino Test Wrapper ↔ ATmega32A DUT</div>
          </div>
          <div className="pin-mapping-grid">
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
                  className={`pin-mapping-row-large ${enhancedState.isActive ? 'active' : ''} ${connection.readonly ? 'readonly' : ''}`}
                  onMouseEnter={() => handleConnectionHover(connection.signal)}
                  onMouseLeave={() => handleConnectionHover(null)}
                  onClick={() => !connection.readonly && handlePinClick(connection.signal, enhancedState)}
                >
                  <div className="pin-connection-visual">
                    <span className="arduino-pin-badge-large">{connection.arduino}</span>
                    <span className="direction-badge-large">{direction}</span>
                    <span className="signal-name-large">{connection.signal}</span>
                    <span className="connection-arrow-large">→</span>
                    <span className="atmega-pin-badge-large">{connection.atmega}</span>
                    <span className="current-value-badge-large">{currentValue}</span>
                  </div>
                  <div className="pin-description-large">
                    {connection.description}
                    {connection.readonly && <span className="readonly-indicator"> (Communication Pin)</span>}
                  </div>
                </div>
              )
            })}
          </div>
        </div>
      </div>
    </div>
  )
}

export default HardwareDiagram
