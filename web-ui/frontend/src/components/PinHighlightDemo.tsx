import { useState } from 'react'

interface PinHighlightDemoProps {
  className?: string
}

export default function PinHighlightDemo({ className = '' }: PinHighlightDemoProps) {
  const [highlightedPins, setHighlightedPins] = useState<string[]>([])

  // Mock pin connections for demo
  const mockConnections = [
    { arduino: 'D7', atmega: 'PB0', signal: 'FREQ_DIV10_4', description: 'Frequency ÷10 output' },
    { arduino: 'D8', atmega: 'PB4', signal: 'FREQ_LOCK_4', description: 'Frequency lock input' },
    { arduino: 'A2', atmega: 'PD3', signal: 'OVERLOAD_4', description: 'Overload input' },
    { arduino: 'A3', atmega: 'PC0', signal: 'START_4', description: 'Start output' },
    { arduino: 'A4', atmega: 'PC1', signal: 'RESET_4', description: 'Reset output' }
  ]

  const handlePinClick = (signal: string) => {
    const currentlyHighlighted = highlightedPins.includes(signal)
    if (currentlyHighlighted) {
      // Remove highlighting
      setHighlightedPins(highlightedPins.filter(pin => pin !== signal))
    } else {
      // Add highlighting (keeping existing ones)
      setHighlightedPins([...highlightedPins, signal])
    }
  }

  const clearAllHighlights = () => {
    setHighlightedPins([])
  }

  return (
    <div className={`pin-highlight-demo ${className}`} style={{ 
      padding: '20px', 
      background: 'white', 
      borderRadius: '8px', 
      boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
      margin: '20px'
    }}>
      <h3 style={{ marginBottom: '20px', color: '#374151' }}>
        🔌 Enhanced Pin Connection Highlighting Demo
      </h3>
      
      <p style={{ fontSize: '14px', color: '#6b7280', marginBottom: '20px' }}>
        Click on any part of a pin connection (left pin, center connection, or right pin) to highlight all three components. 
        Click again to remove highlighting.
      </p>

      <div style={{ marginBottom: '15px' }}>
        <button
          onClick={clearAllHighlights}
          style={{
            padding: '8px 16px',
            background: '#6b7280',
            color: 'white',
            border: 'none',
            borderRadius: '6px',
            cursor: 'pointer',
            fontSize: '14px',
            fontWeight: '500'
          }}
        >
          🧹 Clear All Highlights
        </button>
        <span style={{ marginLeft: '15px', fontSize: '12px', color: '#6b7280' }}>
          Currently highlighted: {highlightedPins.length} connection{highlightedPins.length !== 1 ? 's' : ''}
        </span>
      </div>

      {/* Pin Connection Demo Layout */}
      <div style={{ 
        display: 'grid', 
        gridTemplateColumns: '1fr 2fr 1fr', 
        gap: '20px', 
        alignItems: 'center',
        background: '#f8f9fa',
        padding: '20px',
        borderRadius: '8px'
      }}>
        
        {/* Left Side - Arduino Pins */}
        <div>
          <h4 style={{ fontSize: '14px', fontWeight: '600', marginBottom: '10px', textAlign: 'center' }}>
            Arduino Pins
          </h4>
          <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
            {mockConnections.map(conn => {
              const isHighlighted = highlightedPins.includes(conn.signal)
              return (
                <div
                  key={`arduino-${conn.signal}`}
                  onClick={() => handlePinClick(conn.signal)}
                  style={{
                    padding: '8px 12px',
                    background: isHighlighted ? '#fef3c7' : '#dbeafe',
                    border: isHighlighted ? '2px solid #f59e0b' : '2px solid #3b82f6',
                    borderRadius: '6px',
                    cursor: 'pointer',
                    textAlign: 'center',
                    fontSize: '12px',
                    fontWeight: '600',
                    color: isHighlighted ? '#92400e' : '#1e40af',
                    transition: 'all 0.2s ease',
                    transform: isHighlighted ? 'scale(1.05)' : 'scale(1)'
                  }}
                  title={`Click to ${isHighlighted ? 'remove' : 'add'} highlighting`}
                >
                  {conn.arduino}
                </div>
              )
            })}
          </div>
        </div>

        {/* Center - Connection Bars */}
        <div>
          <h4 style={{ fontSize: '14px', fontWeight: '600', marginBottom: '10px', textAlign: 'center' }}>
            Pin Connections
          </h4>
          <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
            {mockConnections.map(conn => {
              const isHighlighted = highlightedPins.includes(conn.signal)
              return (
                <div
                  key={`connection-${conn.signal}`}
                  onClick={() => handlePinClick(conn.signal)}
                  style={{
                    padding: '8px 12px',
                    background: isHighlighted ? '#fef3c7' : '#f3f4f6',
                    border: isHighlighted ? '2px solid #f59e0b' : '2px solid #d1d5db',
                    borderRadius: '6px',
                    cursor: 'pointer',
                    fontSize: '11px',
                    color: isHighlighted ? '#92400e' : '#374151',
                    transition: 'all 0.2s ease',
                    transform: isHighlighted ? 'scale(1.02)' : 'scale(1)',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between'
                  }}
                  title={`${conn.description}\nClick to ${isHighlighted ? 'remove' : 'add'} highlighting`}
                >
                  <span style={{ fontWeight: '600' }}>{conn.arduino}</span>
                  <span style={{ color: '#6b7280' }}>→</span>
                  <span style={{ fontWeight: '600' }}>{conn.atmega}</span>
                </div>
              )
            })}
          </div>
        </div>

        {/* Right Side - ATmega Pins */}
        <div>
          <h4 style={{ fontSize: '14px', fontWeight: '600', marginBottom: '10px', textAlign: 'center' }}>
            ATmega Pins
          </h4>
          <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
            {mockConnections.map(conn => {
              const isHighlighted = highlightedPins.includes(conn.signal)
              return (
                <div
                  key={`atmega-${conn.signal}`}
                  onClick={() => handlePinClick(conn.signal)}
                  style={{
                    padding: '8px 12px',
                    background: isHighlighted ? '#fef3c7' : '#fecaca',
                    border: isHighlighted ? '2px solid #f59e0b' : '2px solid #dc2626',
                    borderRadius: '6px',
                    cursor: 'pointer',
                    textAlign: 'center',
                    fontSize: '12px',
                    fontWeight: '600',
                    color: isHighlighted ? '#92400e' : '#dc2626',
                    transition: 'all 0.2s ease',
                    transform: isHighlighted ? 'scale(1.05)' : 'scale(1)'
                  }}
                  title={`Click to ${isHighlighted ? 'remove' : 'add'} highlighting`}
                >
                  {conn.atmega}
                </div>
              )
            })}
          </div>
        </div>
      </div>

      {/* Status Display */}
      <div style={{ 
        marginTop: '20px', 
        padding: '12px', 
        background: highlightedPins.length > 0 ? '#dbeafe' : '#f8f9fa',
        color: highlightedPins.length > 0 ? '#1e40af' : '#6c757d',
        borderRadius: '6px',
        fontSize: '12px',
        textAlign: 'center'
      }}>
        {highlightedPins.length === 0 ? 
          '⏸️ No connections highlighted. Click on any pin or connection to highlight all three components.' :
          `🔗 Highlighted connections: ${highlightedPins.map(pin => {
            const conn = mockConnections.find(c => c.signal === pin)
            return conn ? `${conn.arduino}↔${conn.atmega}` : pin
          }).join(', ')}`
        }
      </div>

      {/* Instructions */}
      <div style={{ 
        marginTop: '15px', 
        padding: '10px', 
        background: '#e0f2fe',
        borderLeft: '4px solid #0284c7',
        fontSize: '12px',
        color: '#0c4a6e'
      }}>
        <strong>✨ New Feature:</strong> When you click on any part of a pin connection (left pin, center connection bar, or right pin), 
        all three components stay highlighted together. This makes it easy to see the complete connection path at a glance!
      </div>
    </div>
  )
}
