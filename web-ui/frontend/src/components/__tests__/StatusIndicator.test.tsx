import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import React from 'react'

// Simple StatusIndicator component for testing
interface StatusIndicatorProps {
  connected: boolean
  label?: string
  className?: string
}

const StatusIndicator: React.FC<StatusIndicatorProps> = ({ 
  connected, 
  label = 'Status',
  className = ''
}) => {
  return (
    <div className={`status-indicator ${connected ? 'connected' : 'disconnected'} ${className}`}>
      <div className="status-dot" />
      <span className="status-label">{label}</span>
      <span className="status-text">{connected ? 'Connected' : 'Disconnected'}</span>
    </div>
  )
}

// Simple PinStateDisplay component for testing
interface PinStateDisplayProps {
  signal: string
  state: string
  direction: 'IN' | 'OUT'
  timestamp?: number
}

const PinStateDisplay: React.FC<PinStateDisplayProps> = ({
  signal,
  state,
  direction,
  timestamp
}) => {
  const formatState = (state: string) => {
    if (state.startsWith('PWM=')) {
      return state
    }
    return state.toUpperCase()
  }

  const formatTimestamp = (ts?: number) => {
    if (!ts) return 'Never'
    return new Date(ts).toLocaleTimeString()
  }

  return (
    <div className={`pin-state-display ${direction.toLowerCase()}`}>
      <div className="pin-signal">{signal}</div>
      <div className="pin-state">{formatState(state)}</div>
      <div className="pin-direction">{direction}</div>
      <div className="pin-timestamp">{formatTimestamp(timestamp)}</div>
    </div>
  )
}

// Simple CommandEntry component for testing
interface CommandEntryProps {
  direction: 'sent' | 'received'
  data: string
  timestamp: number
  type: string
}

const CommandEntry: React.FC<CommandEntryProps> = ({
  direction,
  data,
  timestamp,
  type
}) => {
  const formatTimestamp = (ts: number) => {
    return new Date(ts).toLocaleTimeString('en-US', {
      hour12: false,
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit'
    })
  }

  const formatData = (data: string) => {
    try {
      const parsed = JSON.parse(data)
      return JSON.stringify(parsed, null, 2)
    } catch {
      return data
    }
  }

  return (
    <div className={`command-entry ${direction}`}>
      <div className="command-header">
        <span className="command-timestamp">{formatTimestamp(timestamp)}</span>
        <span className={`command-direction ${direction}`}>
          {direction === 'sent' ? '→ SENT' : '← RECEIVED'}
        </span>
        <span className="command-type">{type}</span>
      </div>
      <div className="command-data">
        <pre>{formatData(data)}</pre>
      </div>
    </div>
  )
}

describe('StatusIndicator Component', () => {
  it('renders connected status correctly', () => {
    render(<StatusIndicator connected={true} />)
    
    expect(screen.getByText('Connected')).toBeInTheDocument()
    expect(screen.getByText('Status')).toBeInTheDocument()
  })

  it('renders disconnected status correctly', () => {
    render(<StatusIndicator connected={false} />)
    
    expect(screen.getByText('Disconnected')).toBeInTheDocument()
    expect(screen.getByText('Status')).toBeInTheDocument()
  })

  it('renders custom label', () => {
    render(<StatusIndicator connected={true} label="Hardware Connection" />)
    
    expect(screen.getByText('Hardware Connection')).toBeInTheDocument()
  })

  it('applies correct CSS classes', () => {
    const { container } = render(<StatusIndicator connected={true} className="custom-class" />)
    
    const indicator = container.querySelector('.status-indicator')
    expect(indicator).toHaveClass('connected')
    expect(indicator).toHaveClass('custom-class')
  })

  it('applies disconnected class when not connected', () => {
    const { container } = render(<StatusIndicator connected={false} />)
    
    const indicator = container.querySelector('.status-indicator')
    expect(indicator).toHaveClass('disconnected')
    expect(indicator).not.toHaveClass('connected')
  })
})

describe('PinStateDisplay Component', () => {
  it('renders pin information correctly', () => {
    render(
      <PinStateDisplay 
        signal="START_STOP_4" 
        state="HIGH" 
        direction="IN" 
        timestamp={Date.now()}
      />
    )
    
    expect(screen.getByText('START_STOP_4')).toBeInTheDocument()
    expect(screen.getByText('HIGH')).toBeInTheDocument()
    expect(screen.getByText('IN')).toBeInTheDocument()
  })

  it('formats PWM states correctly', () => {
    render(
      <PinStateDisplay 
        signal="AMPLITUDE_ALL" 
        state="PWM=75%" 
        direction="OUT" 
      />
    )
    
    expect(screen.getByText('PWM=75%')).toBeInTheDocument()
  })

  it('formats digital states to uppercase', () => {
    render(
      <PinStateDisplay 
        signal="TEST_PIN" 
        state="low" 
        direction="IN" 
      />
    )
    
    expect(screen.getByText('LOW')).toBeInTheDocument()
  })

  it('handles missing timestamp', () => {
    render(
      <PinStateDisplay 
        signal="TEST_PIN" 
        state="HIGH" 
        direction="IN" 
      />
    )
    
    expect(screen.getByText('Never')).toBeInTheDocument()
  })

  it('applies correct CSS classes for direction', () => {
    const { container } = render(
      <PinStateDisplay 
        signal="TEST_PIN" 
        state="HIGH" 
        direction="OUT" 
      />
    )
    
    const display = container.querySelector('.pin-state-display')
    expect(display).toHaveClass('out')
  })
})

describe('CommandEntry Component', () => {
  it('renders command information correctly', () => {
    const timestamp = Date.now()
    render(
      <CommandEntry 
        direction="sent"
        data='{"type":"pin_read","pin":"D7"}'
        timestamp={timestamp}
        type="pin_read"
      />
    )
    
    expect(screen.getByText('→ SENT')).toBeInTheDocument()
    expect(screen.getByText('pin_read')).toBeInTheDocument()
    expect(screen.getAllByText(/pin_read/)).toHaveLength(2) // One in header, one in JSON
  })

  it('renders received commands correctly', () => {
    render(
      <CommandEntry 
        direction="received"
        data="HIGH"
        timestamp={Date.now()}
        type="response"
      />
    )
    
    expect(screen.getByText('← RECEIVED')).toBeInTheDocument()
    expect(screen.getByText('response')).toBeInTheDocument()
    expect(screen.getByText('HIGH')).toBeInTheDocument()
  })

  it('formats JSON data with proper indentation', () => {
    render(
      <CommandEntry 
        direction="sent"
        data='{"type":"pin_write","pin":"D8","value":"LOW"}'
        timestamp={Date.now()}
        type="pin_write"
      />
    )
    
    // Should format JSON with indentation
    expect(screen.getAllByText(/pin_write/)).toHaveLength(2) // One in header, one in JSON
    expect(screen.getByText(/D8/)).toBeInTheDocument()
    expect(screen.getAllByText(/LOW/)).toHaveLength(2) // One in JSON value, one in JSON string
  })

  it('handles non-JSON data correctly', () => {
    render(
      <CommandEntry 
        direction="received"
        data="Simple response"
        timestamp={Date.now()}
        type="response"
      />
    )
    
    expect(screen.getByText('Simple response')).toBeInTheDocument()
  })

  it('formats timestamp correctly', () => {
    const timestamp = new Date('2023-01-01T12:30:45').getTime()
    render(
      <CommandEntry 
        direction="sent"
        data="test"
        timestamp={timestamp}
        type="test"
      />
    )
    
    // Should display time in HH:MM:SS format
    expect(screen.getByText('12:30:45')).toBeInTheDocument()
  })

  it('applies correct CSS classes for direction', () => {
    const { container } = render(
      <CommandEntry 
        direction="received"
        data="test"
        timestamp={Date.now()}
        type="test"
      />
    )
    
    const entry = container.querySelector('.command-entry')
    expect(entry).toHaveClass('received')
    
    const direction = container.querySelector('.command-direction')
    expect(direction).toHaveClass('received')
  })
})
