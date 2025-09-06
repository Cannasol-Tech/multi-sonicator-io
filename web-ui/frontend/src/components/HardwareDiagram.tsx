import React, { useState, useEffect } from 'react'

interface HardwareState {
  pins: Record<string, any>
  connection: { connected: boolean; port: string | null }
}

interface HardwareDiagramProps {
  hardwareState: HardwareState
  onPinClick: (signal: string, action: string, value?: any) => void
  highlightedPins?: string[]
}

const PIN_CONNECTIONS = [
  { arduino: 'D7', atmega: 'PB0', signal: 'FREQ_DIV10_4', direction: 'IN', description: 'Frequency ÷10 input' },
  { arduino: 'D8', atmega: 'PB4', signal: 'FREQ_LOCK_4', direction: 'IN', description: 'Frequency lock input' },
  { arduino: 'A2', atmega: 'PD3', signal: 'OVERLOAD_4', direction: 'IN', description: 'Overload input' },
  { arduino: 'A3', atmega: 'PC0', signal: 'START_4', direction: 'OUT', description: 'Start output' },
  { arduino: 'A4', atmega: 'PC1', signal: 'RESET_4', direction: 'OUT', description: 'Reset output' },
  { arduino: 'A1', atmega: 'PA7', signal: 'POWER_SENSE_4', direction: 'ANALOG', description: 'Power sense analog' },
  { arduino: 'D9', atmega: 'PD7', signal: 'AMPLITUDE_ALL', direction: 'IN', description: 'Amplitude PWM input' },
  { arduino: 'D10', atmega: 'PD0', signal: 'UART_RXD', direction: 'IN', description: 'UART RX' },
  { arduino: 'D11', atmega: 'PD1', signal: 'UART_TXD', direction: 'OUT', description: 'UART TX' },
  { arduino: 'D12', atmega: 'PD2', signal: 'STATUS_LED', direction: 'OUT', description: 'Status LED' }
]

export const HardwareDiagram: React.FC = () => {
  const [hardwareState, setHardwareState] = useState<HardwareState>({
    pins: {},
    connection: { connected: false, port: null }
  })
  const [highlightedPins, setHighlightedPins] = useState<string[]>([])

  useEffect(() => {
    // Fetch initial hardware state
    fetch('/api/pins')
      .then(res => res.json())
      .then(data => {
        setHardwareState(prev => ({ ...prev, pins: data.pins }))
      })
      .catch(err => console.error('Failed to fetch pins:', err))

    fetch('/api/connection')
      .then(res => res.json())
      .then(data => {
        setHardwareState(prev => ({ ...prev, connection: data }))
      })
      .catch(err => console.error('Failed to fetch connection:', err))

    // Set up WebSocket for real-time updates
    const ws = new WebSocket('ws://localhost:3001/ws')

    ws.onopen = () => {
      console.log('WebSocket connected')
    }

    ws.onmessage = (event) => {
      console.log('WebSocket message received:', event.data)
      const data = JSON.parse(event.data)
      if (data.type === 'pin_update') {
        console.log('Pin update received:', data)
        setHardwareState(prev => ({
          ...prev,
          pins: { ...prev.pins, [data.signal]: data }
        }))
      } else if (data.type === 'connection_update') {
        console.log('Connection update received:', data)
        setHardwareState(prev => ({ ...prev, connection: data }))
      }
    }

    ws.onerror = (error) => {
      console.error('WebSocket error:', error)
    }

    ws.onclose = () => {
      console.log('WebSocket disconnected')
    }

    return () => ws.close()
  }, [])

  const handlePinClick = (signal: string, pinState: any) => {
    console.log('Pin clicked:', signal, 'Current state:', pinState.state, 'Direction:', pinState.direction)

    if (pinState.direction === 'IN') {
      // Toggle input pin
      const newState = pinState.state === 'HIGH' ? 'LOW' : 'HIGH'
      console.log('Toggling input pin', signal, 'from', pinState.state, 'to', newState)

      fetch(`/api/pins/${signal}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ state: newState })
      })
      .then(res => res.json())
      .then(data => {
        console.log('Pin control response:', data)
        if (data.success) {
          // Update local state immediately for better UX
          setHardwareState(prev => ({
            ...prev,
            pins: {
              ...prev.pins,
              [signal]: { ...prev.pins[signal], state: newState, timestamp: Date.now() }
            }
          }))
        }
      })
      .catch(err => {
        console.error('Failed to control pin:', err)
      })
    } else {
      // Read output/analog pin
      console.log('Reading output/analog pin:', signal)
      fetch(`/api/pins/${signal}`)
        .then(res => res.json())
        .then(data => {
          console.log('Pin read response:', data)
          setHighlightedPins([signal])
          setTimeout(() => setHighlightedPins([]), 1000)
        })
        .catch(err => {
          console.error('Failed to read pin:', err)
        })
    }
  }

  const getPinState = (signal: string) => {
    const state = hardwareState.pins[signal]
    if (!state) return { pin: '?', direction: 'UNKNOWN', state: 'UNKNOWN' }
    return state
  }

  const getConnectionColor = (direction: string, state: any) => {
    if (direction === 'ANALOG') return '#8b5cf6' // Purple for analog
    if (direction === 'IN') return state === 'HIGH' ? '#ef4444' : '#6b7280' // Red for HIGH, gray for LOW
    if (direction === 'OUT') return state === 'HIGH' ? '#10b981' : '#6b7280' // Green for HIGH, gray for LOW
    return '#6b7280'
  }

  return (
    <div className="hardware-diagram" style={{ position: 'relative', minHeight: '600px', padding: '20px' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start' }}>
        
        {/* Arduino Test Wrapper */}
        <div style={{ textAlign: 'center', flex: '0 0 300px' }}>
          <h3 className="mb-4 font-bold text-lg text-blue-800">Arduino Test Wrapper</h3>
          <div style={{ position: 'relative' }}>
            <img
              src="/arduino-uno-r3-icon.png"
              alt="Arduino Uno R3"
              style={{
                width: '300px',
                height: 'auto',
                border: '2px solid #1e40af',
                borderRadius: '8px',
                backgroundColor: 'white'
              }}
              onLoad={(e) => {
                const parent = e.currentTarget.parentElement
                if (parent) {
                  const overlay = parent.querySelector('.chip-overlay') as HTMLElement
                  if (overlay) {
                    overlay.innerHTML = '<div style="color: #1e40af; font-weight: 700; font-size: 24px;">Arduino Uno R3<br/><span style="font-size: 16px;">HIL Test Wrapper</span></div>'
                  }
                }
              }}
            />
            <div
              className="chip-overlay"
              style={{
                position: 'absolute',
                top: '50%',
                left: '50%',
                transform: 'translate(-50%, -50%)',
                textAlign: 'center',
                pointerEvents: 'none',
                background: 'rgba(255, 255, 255, 0.9)',
                padding: '10px',
                borderRadius: '4px',
                fontSize: '14px',
                fontWeight: '700',
                color: '#1e40af',
                letterSpacing: '0.5px'
              }}
            >
              HIL TEST WRAPPER
            </div>
          </div>
        </div>

        {/* Connection Lines and Pin States */}
        <div style={{ 
          display: 'flex', 
          flexDirection: 'column',
          gap: '8px',
          minWidth: '300px'
        }}>
          {PIN_CONNECTIONS.map(connection => {
            const pinState = getPinState(connection.signal)
            const isHighlighted = highlightedPins.includes(connection.signal)
            return (
              <div
                key={connection.signal}
                onClick={() => handlePinClick(connection.signal, pinState)}
                style={{
                  padding: '8px',
                  border: `2px solid ${isHighlighted ? '#f59e0b' : '#e5e7eb'}`,
                  borderRadius: '4px',
                  backgroundColor: isHighlighted ? '#fef3c7' : 'white',
                  cursor: 'pointer'
                }}
              >
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2">
                    <span className="font-mono text-xs">{pinState.pin}</span>
                    <span className="text-xs">→</span>
                    <span className="font-mono text-xs">{connection.signal}</span>
                  </div>
                  <div className="flex items-center gap-2">
                    <span className={`text-xs px-1 rounded ${
                      connection.direction === 'IN' ? 'bg-blue-100 text-blue-800' :
                      connection.direction === 'OUT' ? 'bg-green-100 text-green-800' :
                      'bg-purple-100 text-purple-800'
                    }`}>
                      {connection.direction}
                    </span>
                    <span className={`text-xs font-mono px-1 rounded ${
                      pinState.state === 'HIGH' ? 'bg-red-100 text-red-800' :
                      pinState.state === 'LOW' ? 'bg-gray-100 text-gray-800' :
                      'bg-yellow-100 text-yellow-800'
                    }`}>
                      {pinState.state}
                    </span>
                  </div>
                </div>
              </div>
            )
          })}
        </div>

        {/* ATmega32A DUT */}
        <div style={{ textAlign: 'center' }}>
          <h3 className="mb-4 font-semibold">ATmega32A (DUT)</h3>
          <div style={{ position: 'relative' }}>
            <img
              src="/atmega-32-a-icon.png"
              alt="ATmega32A"
              style={{
                width: '250px',
                height: 'auto',
                border: '2px solid #dc2626',
                borderRadius: '8px',
                backgroundColor: 'white'
              }}
              onLoad={(e) => {
                const parent = e.currentTarget.parentElement
                if (parent) {
                  const overlay = parent.querySelector('.chip-overlay') as HTMLElement
                  if (overlay) {
                    overlay.innerHTML = '<div style="color: #dc2626; font-weight: 600;">ATmega32A</div>'
                  }
                }
              }}
            />
            <div
              className="chip-overlay"
              style={{
                position: 'absolute',
                top: '50%',
                left: '50%',
                transform: 'translate(-50%, -50%)',
                textAlign: 'center',
                pointerEvents: 'none',
                background: 'rgba(255, 255, 255, 0.9)',
                padding: '8px',
                borderRadius: '4px',
                fontSize: '12px',
                fontWeight: '600',
                color: '#dc2626',
                letterSpacing: '0.5px'
              }}
            >
              Sonicator Multiplexer DUT
            </div>
          </div>
          
          {/* DUT Status */}
          <div style={{ marginTop: '20px', fontSize: '11px' }}>
            <div className="text-gray-600">Device Under Test</div>
            <div className={`text-xs ${hardwareState.connection.connected ? 'text-green-600' : 'text-red-600'}`}>
              {hardwareState.connection.connected ? 'Active' : 'Disconnected'}
            </div>
          </div>
        </div>
      </div>

      {/* Help Text */}
      <div style={{
        marginTop: '40px',
        background: '#f9fafb',
        border: '1px solid #e5e7eb',
        borderRadius: '4px',
        padding: '12px',
        fontSize: '12px',
        color: '#6b7280'
      }}>
        <strong>Help:</strong> Click on pin connections to interact. 
        Input pins (IN) can be toggled HIGH/LOW. 
        Output pins (OUT) and analog pins (ANALOG) can be read. 
        Pin mapping based on <code>docs/planning/pin-matrix.md</code> (SOLE SOURCE OF TRUTH).
      </div>
    </div>
  )
}

export default HardwareDiagram
