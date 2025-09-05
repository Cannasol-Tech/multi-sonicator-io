import { HardwareState } from '../types'

interface HardwareDiagramProps {
  hardwareState: HardwareState
  onPinClick: (pin: string, action: string, value?: any) => void
}

// Pin connection mapping based on pin-matrix.md (SOLE SOURCE OF TRUTH)
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

export default function HardwareDiagram({ hardwareState, onPinClick }: HardwareDiagramProps) {
  const handlePinClick = (signal: string, currentState: any) => {
    const pinState = hardwareState.pins[signal]
    if (!pinState) return

    if (pinState.direction === 'IN') {
      // Toggle input pins
      const newState = currentState === 'HIGH' ? 'LOW' : 'HIGH'
      onPinClick(signal, 'write_pin', newState)
    } else if (pinState.direction === 'OUT') {
      // Read output pins
      onPinClick(signal, 'read_pin')
    } else if (pinState.direction === 'ANALOG') {
      // Read analog pins
      onPinClick(signal, 'read_adc')
    }
  }

  const getPinStateClass = (state: any) => {
    if (typeof state === 'number') return 'analog'
    return state === 'HIGH' ? 'high' : 'low'
  }

  const formatPinValue = (state: any) => {
    if (typeof state === 'number') {
      return `${state} (${((state / 1023) * 5).toFixed(2)}V)`
    }
    return state
  }

  const getConnectionColor = (direction: string, state: any) => {
    if (direction === 'ANALOG') return '#8b5cf6' // Purple for analog
    if (direction === 'IN') return state === 'HIGH' ? '#ef4444' : '#6b7280' // Red for HIGH, gray for LOW
    if (direction === 'OUT') return state === 'HIGH' ? '#10b981' : '#6b7280' // Green for HIGH, gray for LOW
    return '#6b7280'
  }

  return (
    <div className="hardware-diagram" style={{ position: 'relative', minHeight: '800px', padding: '20px' }}>
      {/* SVG for connection lines */}
      <svg
        style={{
          position: 'absolute',
          top: 0,
          left: 0,
          width: '100%',
          height: '100%',
          pointerEvents: 'none',
          zIndex: 1
        }}
      >
        <defs>
          <marker id="arrowhead" markerWidth="10" markerHeight="7"
                  refX="9" refY="3.5" orient="auto">
            <polygon points="0 0, 10 3.5, 0 7" fill="#6b7280" />
          </marker>
        </defs>

        {/* Connection lines */}
        {PIN_CONNECTIONS.map((connection, index) => {
          const pinState = hardwareState.pins[connection.signal]
          const color = getConnectionColor(connection.direction, pinState?.state)
          const yOffset = 200 + (index * 45) // Vertical spacing for connections

          return (
            <g key={connection.signal}>
              {/* Connection line */}
              <line
                x1="450" // Arduino side
                y1={yOffset}
                x2="750" // ATmega side
                y2={yOffset}
                stroke={color}
                strokeWidth="3"
                markerEnd="url(#arrowhead)"
                opacity="0.8"
              />

              {/* Signal label */}
              <text
                x="600" // Center of line
                y={yOffset - 8}
                textAnchor="middle"
                fontSize="11"
                fill="#374151"
                fontWeight="600"
              >
                {connection.signal}
              </text>

              {/* State indicator */}
              <circle
                cx="600"
                cy={yOffset + 15}
                r="8"
                fill={color}
                opacity="0.9"
              />
              <text
                x="600"
                y={yOffset + 19}
                textAnchor="middle"
                fontSize="8"
                fill="white"
                fontWeight="bold"
              >
                {pinState ? (typeof pinState.state === 'number' ? 'A' : pinState.state.charAt(0)) : '?'}
              </text>
            </g>
          )
        })}
      </svg>

      <div style={{
        display: 'flex',
        alignItems: 'flex-start',
        justifyContent: 'space-between',
        gap: '100px',
        padding: '40px',
        maxWidth: '1400px',
        margin: '0 auto',
        position: 'relative',
        zIndex: 2
      }}>

        {/* Arduino Test Wrapper */}
        <div style={{ textAlign: 'center', flex: '0 0 400px' }}>
          <h3 className="mb-6 font-bold text-lg text-blue-800">Arduino Test Wrapper</h3>
          <div style={{
            width: '400px',
            height: '300px',
            background: 'linear-gradient(135deg, #f8fafc 0%, #e2e8f0 100%)',
            border: '3px solid #1e40af',
            borderRadius: '12px',
            position: 'relative',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            overflow: 'hidden',
            boxShadow: '0 10px 25px rgba(30, 64, 175, 0.2)'
          }}>
            <img
              src="/arduino-uno-r3-icon.png"
              alt="Arduino Uno R3"
              style={{
                maxWidth: '85%',
                maxHeight: '85%',
                objectFit: 'contain',
                filter: 'drop-shadow(0 4px 8px rgba(0,0,0,0.1))'
              }}
              onError={(e) => {
                // Fallback to text if image fails to load
                const target = e.target as HTMLImageElement;
                target.style.display = 'none';
                const parent = target.parentElement;
                if (parent) {
                  parent.innerHTML = '<div style="color: #1e40af; font-weight: 700; font-size: 24px;">Arduino Uno R3<br/><span style="font-size: 16px;">HIL Test Wrapper</span></div>';
                }
              }}
            />
            <div style={{
              position: 'absolute',
              top: '8px',
              left: '8px',
              right: '8px',
              height: '24px',
              background: 'rgba(30, 64, 175, 0.95)',
              borderRadius: '6px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              fontSize: '12px',
              color: 'white',
              fontWeight: '700',
              letterSpacing: '0.5px'
            }}>
              HIL TEST WRAPPER
            </div>
          </div>

        {/* Connection Lines and Pin States */}
        <div style={{ 
          display: 'flex', 
          flexDirection: 'column',
          gap: '8px',
          minWidth: '300px'
        }}>
          <h4 className="text-center font-semibold mb-2">Pin Connections</h4>
          
          {Object.entries(hardwareState.pins).map(([signal, pinState]) => (
            <div 
              key={signal}
              className={`pin-state ${getPinStateClass(pinState.state)}`}
              onClick={() => handlePinClick(signal, pinState.state)}
              style={{ cursor: 'pointer' }}
              title={`${pinState.description || ''}\nClick to ${pinState.direction === 'IN' ? 'toggle' : 'read'}`}
            >
              <div className="flex items-center justify-between">
                <div className="flex items-center gap-2">
                  <span className="font-mono text-xs">{pinState.pin}</span>
                  <span className="text-xs">→</span>
                  <span className="font-mono text-xs">{signal}</span>
                </div>
                <div className="flex items-center gap-2">
                  <span className={`text-xs px-1 rounded ${
                    pinState.direction === 'IN' ? 'bg-blue-100 text-blue-800' :
                    pinState.direction === 'OUT' ? 'bg-green-100 text-green-800' :
                    'bg-purple-100 text-purple-800'
                  }`}>
                    {pinState.direction}
                  </span>
                  <span className="font-mono text-xs font-semibold">
                    {formatPinValue(pinState.state)}
                  </span>
                </div>
              </div>
            </div>
          ))}
        </div>

        {/* ATmega32A DUT */}
        <div style={{ textAlign: 'center' }}>
          <h3 className="mb-4 font-semibold">ATmega32A (DUT)</h3>
          <div style={{
            width: '300px',
            height: '200px',
            background: '#fef2f2',
            border: '2px solid #dc2626',
            borderRadius: '8px',
            position: 'relative',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            overflow: 'hidden'
          }}>
            <img
              src="/atmega-32-a-icon.png"
              alt="ATmega32A"
              style={{
                maxWidth: '90%',
                maxHeight: '90%',
                objectFit: 'contain'
              }}
              onError={(e) => {
                // Fallback to text if image fails to load
                const target = e.target as HTMLImageElement;
                target.style.display = 'none';
                const parent = target.parentElement;
                if (parent) {
                  parent.innerHTML = '<div style="color: #dc2626; font-weight: 600;">ATmega32A</div>';
                }
              }}
            />
            <div style={{
              position: 'absolute',
              top: '5px',
              left: '5px',
              right: '5px',
              height: '16px',
              background: 'rgba(220, 38, 38, 0.9)',
              borderRadius: '4px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              fontSize: '10px',
              color: 'white',
              fontWeight: '600'
            }}>
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
        position: 'absolute',
        bottom: '20px',
        left: '20px',
        right: '20px',
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
