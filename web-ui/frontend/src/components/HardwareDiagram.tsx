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
  { arduino: 'D7', atmega: 'PB0', signal: 'FREQ_DIV10_4', direction: 'IN', description: 'Frequency ÷10 input' },
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
  const [selectedConnection, setSelectedConnection] = useState<string | null>(null)
  const [showConnectionDetails, setShowConnectionDetails] = useState(false)
  const [pingStatus, setPingStatus] = useState<{
    isLoading: boolean
    lastPing: number | null
    responseTime: number | null
    success: boolean | null
    message: string
  }>({
    isLoading: false,
    lastPing: null,
    responseTime: null,
    success: null,
    message: 'Not tested'
  })


  // Enhanced pin click handler with visual feedback
  const handlePinClickWithFeedback = (signal: string, pinState: any) => {
    setSelectedConnection(signal)
    handlePinClick(signal, pinState)

    // Clear selection after a delay
    setTimeout(() => {
      setSelectedConnection(null)
    }, 2000)
  }

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

  // Toggle connection details panel
  const toggleConnectionDetails = () => {
    setShowConnectionDetails(!showConnectionDetails)
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
                    hoveredConnection === signal ||
                    selectedConnection === signal

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

  const handlePingCommand = async () => {
    console.log('PING: Starting ping test')
    setPingStatus(prev => ({ ...prev, isLoading: true, message: 'Testing...' }))

    try {
      const startTime = Date.now()
      const response = await fetch('/api/ping', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' }
      })

      const data = await response.json()
      const clientResponseTime = Date.now() - startTime

      console.log('PING: Response received:', data)

      if (data.success) {
        setPingStatus({
          isLoading: false,
          lastPing: Date.now(),
          responseTime: data.responseTime || clientResponseTime,
          success: true,
          message: data.result?.data || 'Hardware responding'
        })
      } else {
        setPingStatus({
          isLoading: false,
          lastPing: Date.now(),
          responseTime: clientResponseTime,
          success: false,
          message: data.error || 'Hardware not responding'
        })
      }
    } catch (error) {
      console.error('PING: Error:', error)
      setPingStatus({
        isLoading: false,
        lastPing: Date.now(),
        responseTime: null,
        success: false,
        message: 'Network error - cannot reach backend'
      })
    }
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

      {/* Main Hardware Layout */}
      <div className="hardware-layout">
        <div className="devices-row">
          {/* Arduino Test Wrapper */}
          <div className="device-container arduino-container">
            <div className="device-header">
              <h3>Arduino Uno R3</h3>
              <span className="device-subtitle">HIL Test Wrapper</span>
            </div>
            <div className="device-image-container">
              <img
                src="/arduino-uno-r3-icon.png"
                alt="Arduino Uno R3"
                className="device-image arduino-image"
              />
              <div className="device-pins arduino-pins">
                {PIN_CONNECTIONS.map(connection => {
                  const enhancedState = getEnhancedPinState(connection.signal)
                  return (
                    <div
                      key={connection.arduino}
                      data-pin={connection.arduino}
                      className={`pin-indicator arduino-pin ${enhancedState.isActive ? 'highlighted' : ''} ${connection.readonly ? 'readonly' : ''}`}
                      onMouseEnter={() => handleConnectionHover(connection.signal)}
                      onMouseLeave={() => handleConnectionHover(null)}
                      onClick={() => !connection.readonly && handlePinClickWithFeedback(connection.signal, enhancedState)}
                      title={`${connection.arduino} → ${connection.atmega} (${connection.signal})\n${connection.description}\n${connection.readonly ? 'READONLY - Communication Pin' : 'Last updated: ' + enhancedState.lastUpdated}`}
                      style={{ cursor: connection.readonly ? 'not-allowed' : 'pointer' }}
                    >
                      <span className="pin-label">{connection.arduino}</span>
                      <div className={`pin-status ${connection.direction.toLowerCase()}`}>
                        {enhancedState.state}
                      </div>
                    </div>
                  )
                })}
              </div>
            </div>
          </div>

          {/* ATmega32A DUT */}
          <div className="device-container atmega-container">
            <div className="device-header">
              <h3>ATmega32A</h3>
              <span className="device-subtitle">Device Under Test</span>
            </div>
            <div className="device-image-container">
              <img
                src="/atmega-32-a-icon.png"
                alt="ATmega32A"
                className="device-image atmega-image"
              />
              <div className="device-pins atmega-pins">
                {PIN_CONNECTIONS.map(connection => {
                  const enhancedState = getEnhancedPinState(connection.signal)
                  return (
                    <div
                      key={connection.atmega}
                      data-pin={connection.atmega}
                      className={`pin-indicator atmega-pin ${enhancedState.isActive ? 'highlighted' : ''}`}
                      onMouseEnter={() => handleConnectionHover(connection.signal)}
                      onMouseLeave={() => handleConnectionHover(null)}
                      title={`${connection.atmega} ← ${connection.arduino} (${connection.signal})\n${connection.description}\nLast updated: ${enhancedState.lastUpdated}`}
                    >
                      <span className="pin-label">{connection.atmega}</span>
                      <div className={`pin-status ${connection.direction.toLowerCase()}`}>
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

        {/* Connection Info Panel */}
        <div className="connection-info-panel">
          <div className="panel-header">
            <h4>Pin Connections</h4>
            <div className="panel-controls">
              <button
                className="details-toggle"
                onClick={toggleConnectionDetails}
                title="Toggle detailed view"
              >
                {showConnectionDetails ? '📋' : '📝'}
              </button>
              <div className="connection-status">
                <div className={`status-indicator ${hardwareState.connection.connected ? 'connected' : 'disconnected'}`}></div>
                <span>{hardwareState.connection.connected ? 'Connected' : 'Disconnected'}</span>
              </div>
            </div>
          </div>
          <div className="connections-list">
            {PIN_CONNECTIONS.map(connection => {
              const enhancedState = getEnhancedPinState(connection.signal)
              return (
                <div
                  key={connection.signal}
                  className={`connection-item ${enhancedState.isActive ? 'highlighted' : ''}`}
                  onMouseEnter={() => handleConnectionHover(connection.signal)}
                  onMouseLeave={() => handleConnectionHover(null)}
                  onClick={() => handlePinClickWithFeedback(connection.signal, enhancedState)}
                >
                  <div className="connection-pins">
                    <span className="arduino-pin-label">{connection.arduino}</span>
                    <div className="connection-arrow">→</div>
                    <span className="atmega-pin-label">{connection.atmega}</span>
                  </div>
                  <div className="connection-details">
                    <span className="signal-name">{connection.signal}</span>
                    <div className="pin-info">
                      <span className={`direction-badge ${connection.direction.toLowerCase()}`}>
                        {connection.direction}
                      </span>
                      <span className={`state-badge ${String(enhancedState.state || '').toLowerCase() || 'unknown'}`}>
                        {enhancedState.state || 'UNKNOWN'}
                      </span>
                    </div>
                  </div>
                  <div className="connection-description">
                    {connection.description}
                  </div>
                  {showConnectionDetails && (
                    <div className="connection-extended-info">
                      <div className="info-row">
                        <span>Last Updated:</span>
                        <span>{enhancedState.lastUpdated}</span>
                      </div>
                      <div className="info-row">
                        <span>Click Action:</span>
                        <span>{connection.direction === 'IN' ? 'Toggle State' : 'Read Value'}</span>
                      </div>
                    </div>
                  )}
                </div>
              )
            })}
          </div>
        </div>
      </div>

      {/* Control Panel */}
      <div className="control-panel-bottom">
        {/* PING Test Section */}
        <div className="ping-test-section">
          <h4>Hardware Communication Test</h4>
          <div className="ping-controls">
            <button
              onClick={handlePingCommand}
              disabled={pingStatus.isLoading}
              className={`ping-button ${pingStatus.isLoading ? 'loading' : ''}`}
            >
              {pingStatus.isLoading ? 'Testing...' : 'PING Hardware'}
            </button>
            <div className={`ping-status ${
              pingStatus.success === null ? 'unknown' :
              pingStatus.success ? 'success' : 'error'
            }`}>
              <div className="status-dot"></div>
              <span>{
                pingStatus.success === null ? 'Not tested' :
                pingStatus.success ? 'Connected' : 'Failed'
              }</span>
            </div>
          </div>
          <div className="ping-details">
            <div><strong>Status:</strong> {pingStatus.message}</div>
            {pingStatus.responseTime && (
              <div><strong>Response Time:</strong> {pingStatus.responseTime}ms</div>
            )}
            {pingStatus.lastPing !== null && (
              <div><strong>Last Test:</strong> {new Date(pingStatus.lastPing).toLocaleTimeString()}</div>
            )}
          </div>
        </div>

        {/* System Info */}
        <div className="system-info">
          <div className="info-item">
            <strong>Backend:</strong> {hardwareState.connection.connected ? 'Connected' : 'Disconnected'}
          </div>
          <div className="info-item">
            <strong>Port:</strong> {hardwareState.connection.port || 'None'}
          </div>
          <div className="info-item">
            <strong>WebSocket:</strong> Active
          </div>
        </div>

        {/* Help Text */}
        <div className="help-text">
          <strong>Help:</strong> Click on pin connections to interact.
          Input pins (IN) can be toggled HIGH/LOW.
          Output pins (OUT) and analog pins (ANALOG) can be read.
          Pin mapping based on <code>docs/planning/pin-matrix.md</code> (SOLE SOURCE OF TRUTH).
        </div>
      </div>
    </div>
  )
}

export default HardwareDiagram
