import React from 'react'
import { render, screen, fireEvent } from '@testing-library/react'
import { vi, describe, it, expect, beforeEach } from 'vitest'
import ArduinoCommandHistory from '../ArduinoCommandHistory'
import { ArduinoCommand } from '../../hooks/useArduinoCommandLog'

// Mock data for testing
const createMockCommand = (overrides: Partial<ArduinoCommand> = {}): ArduinoCommand => ({
  direction: 'sent',
  data: 'TEST_COMMAND',
  timestamp: Date.now(),
  type: 'command',
  id: 'test-id-123',
  status: 'success',
  responseTime: 50,
  ...overrides
})

const createMockCommandPair = (sentOverrides: Partial<ArduinoCommand> = {}, receivedOverrides: Partial<ArduinoCommand> = {}) => {
  const id = `pair-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`
  return {
    sent: createMockCommand({ 
      direction: 'sent' as const, 
      id, 
      data: 'SET_PIN_HIGH D2',
      ...sentOverrides 
    }),
    received: createMockCommand({ 
      direction: 'received' as const, 
      id, 
      data: 'OK - Pin D2 set to HIGH',
      ...receivedOverrides 
    })
  }
}

describe('ArduinoCommandHistory Component', () => {
  const defaultProps = {
    commandPairs: [],
    connected: true,
    maxEntries: 5
  }

  beforeEach(() => {
    vi.clearAllMocks()
  })

  describe('Rendering', () => {
    it('renders the component with correct header', () => {
      render(<ArduinoCommandHistory {...defaultProps} />)
      
      expect(screen.getByText('📡 Arduino Communication Log')).toBeInTheDocument()
      expect(screen.getByText('Last 5 Commands & Responses')).toBeInTheDocument()
    })

    it('shows connection status when connected', () => {
      render(<ArduinoCommandHistory {...defaultProps} connected={true} />)
      
      const connectionIndicator = screen.getByText('Live')
      expect(connectionIndicator).toBeInTheDocument()
      expect(connectionIndicator.closest('.connection-indicator')).toHaveClass('connected')
    })

    it('shows connection status when disconnected', () => {
      render(<ArduinoCommandHistory {...defaultProps} connected={false} />)
      
      const connectionIndicator = screen.getByText('Offline')
      expect(connectionIndicator).toBeInTheDocument()
      expect(connectionIndicator.closest('.connection-indicator')).toHaveClass('disconnected')
    })

    it('displays no commands message when empty', () => {
      render(<ArduinoCommandHistory {...defaultProps} />)
      
      expect(screen.getByText('No commands sent yet')).toBeInTheDocument()
      expect(screen.getByText('Arduino commands will appear here in real-time')).toBeInTheDocument()
    })

    it('displays command pairs when provided', () => {
      const commandPairs = [createMockCommandPair()]
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)
      
      expect(screen.getByText('📤 SENT')).toBeInTheDocument()
      expect(screen.getByText('📥 RECEIVED')).toBeInTheDocument()
      expect(screen.getByText('SET_PIN_HIGH D2')).toBeInTheDocument()
      expect(screen.getByText('OK - Pin D2 set to HIGH')).toBeInTheDocument()
    })
  })

  describe('Command Display', () => {
    it('displays command timestamps correctly', () => {
      const timestamp = new Date('2023-12-01T10:30:45').getTime()
      const commandPairs = [createMockCommandPair(
        { timestamp },
        { timestamp: timestamp + 50 }
      )]

      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)

      // Should find both timestamps (sent and received)
      const timestamps = screen.getAllByText('10:30:45')
      expect(timestamps).toHaveLength(2)
    })

    it('displays response time with correct formatting', () => {
      const commandPairs = [createMockCommandPair(
        { responseTime: 5 }, // Use 5ms to ensure green color
        { responseTime: 5 }
      )]

      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)

      expect(screen.getByText('Response: 5ms 🟢')).toBeInTheDocument()
    })

    it('displays different response time colors based on speed', () => {
      const fastPair = createMockCommandPair({ responseTime: 5 }, { responseTime: 5 })
      const mediumPair = createMockCommandPair({ responseTime: 50 }, { responseTime: 50 })
      const slowPair = createMockCommandPair({ responseTime: 200 }, { responseTime: 200 })
      const verySlowPair = createMockCommandPair({ responseTime: 600 }, { responseTime: 600 })
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={[fastPair, mediumPair, slowPair, verySlowPair]} />)
      
      expect(screen.getByText('Response: 5ms 🟢')).toBeInTheDocument()
      expect(screen.getByText('Response: 50ms 🟡')).toBeInTheDocument()
      expect(screen.getByText('Response: 200ms 🟠')).toBeInTheDocument()
      expect(screen.getByText('Response: 600ms 🔴')).toBeInTheDocument()
    })

    it('displays status icons correctly', () => {
      const successPair = createMockCommandPair({ status: 'success' }, { status: 'success' })
      const errorPair = createMockCommandPair({ status: 'error' }, { status: 'error' })
      const pendingPair = createMockCommandPair({ status: 'pending' }, { status: 'pending' })
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={[successPair, errorPair, pendingPair]} />)
      
      // Should have success icons (✅)
      expect(screen.getAllByText('✅')).toHaveLength(2) // One for sent, one for received
      // Should have error icons (❌)
      expect(screen.getAllByText('❌')).toHaveLength(2)
      // Should have pending icons (⏳)
      expect(screen.getAllByText('⏳')).toHaveLength(2)
    })

    it('truncates long command data', () => {
      const longCommand = 'A'.repeat(100)
      const commandPairs = [createMockCommandPair({ data: longCommand })]
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)
      
      // Should be truncated to 50 characters + "..."
      expect(screen.getByText('A'.repeat(50) + '...')).toBeInTheDocument()
    })

    it('displays error messages when present', () => {
      const errorPair = createMockCommandPair(
        { status: 'error', error: 'Connection timeout' },
        { status: 'error', error: 'Connection timeout' }
      )
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={[errorPair]} />)
      
      expect(screen.getByText('⚠️ Connection timeout')).toBeInTheDocument()
    })

    it('shows no response when received command is missing', () => {
      const pairWithoutResponse = {
        sent: createMockCommand({ data: 'PING' }),
        received: undefined
      }
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={[pairWithoutResponse]} />)
      
      expect(screen.getByText('No response received')).toBeInTheDocument()
    })
  })

  describe('User Interactions', () => {
    it('calls onCommandClick when command is clicked and callback is provided', () => {
      const onCommandClick = vi.fn()
      const commandPairs = [createMockCommandPair()]
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} onCommandClick={onCommandClick} />)
      
      const sentCommand = screen.getByText('SET_PIN_HIGH D2').closest('.command-entry')
      fireEvent.click(sentCommand!)
      
      expect(onCommandClick).toHaveBeenCalledWith(commandPairs[0].sent)
    })

    it('does not show clickable cursor when onCommandClick is not provided', () => {
      const commandPairs = [createMockCommandPair()]
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)
      
      const sentCommand = screen.getByText('SET_PIN_HIGH D2').closest('.command-entry')
      expect(sentCommand).not.toHaveClass('clickable')
    })

    it('shows clickable cursor when onCommandClick is provided', () => {
      const onCommandClick = vi.fn()
      const commandPairs = [createMockCommandPair()]
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} onCommandClick={onCommandClick} />)
      
      const sentCommand = screen.getByText('SET_PIN_HIGH D2').closest('.command-entry')
      expect(sentCommand).toHaveClass('clickable')
    })
  })

  describe('Footer Statistics', () => {
    it('displays correct statistics', () => {
      const successPair = createMockCommandPair({ status: 'success' }, { status: 'success' })
      const errorPair = createMockCommandPair({ status: 'error' }, { status: 'error' })
      const anotherSuccessPair = createMockCommandPair({ status: 'success' }, { status: 'success' })
      
      const commandPairs = [successPair, errorPair, anotherSuccessPair]
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)
      
      expect(screen.getByText('📊 Total: 3')).toBeInTheDocument()
      expect(screen.getByText('✅ Success: 2')).toBeInTheDocument()
      expect(screen.getByText('❌ Errors: 1')).toBeInTheDocument()
    })

    it('handles zero statistics correctly', () => {
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={[]} />)
      
      expect(screen.getByText('📊 Total: 0')).toBeInTheDocument()
      expect(screen.getByText('✅ Success: 0')).toBeInTheDocument()
      expect(screen.getByText('❌ Errors: 0')).toBeInTheDocument()
    })
  })

  describe('Props Handling', () => {
    it('respects maxEntries prop', () => {
      const commandPairs = Array.from({ length: 10 }, (_, i) => 
        createMockCommandPair({ data: `Command ${i}` })
      )
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} maxEntries={3} />)
      
      // Should only show first 3 commands
      expect(screen.getByText('Command 0')).toBeInTheDocument()
      expect(screen.getByText('Command 1')).toBeInTheDocument()
      expect(screen.getByText('Command 2')).toBeInTheDocument()
      expect(screen.queryByText('Command 3')).not.toBeInTheDocument()
    })

    it('uses default maxEntries when not provided', () => {
      const commandPairs = Array.from({ length: 10 }, (_, i) => 
        createMockCommandPair({ data: `Command ${i}` })
      )
      
      const { maxEntries, ...propsWithoutMaxEntries } = defaultProps
      render(<ArduinoCommandHistory {...propsWithoutMaxEntries} commandPairs={commandPairs} />)
      
      // Should show default 5 commands
      expect(screen.getByText('Command 0')).toBeInTheDocument()
      expect(screen.getByText('Command 4')).toBeInTheDocument()
      expect(screen.queryByText('Command 5')).not.toBeInTheDocument()
    })
  })

  describe('Accessibility', () => {
    it('has proper semantic structure', () => {
      const commandPairs = [createMockCommandPair()]
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)
      
      expect(screen.getByRole('heading', { name: /Arduino Communication Log/ })).toBeInTheDocument()
    })

    it('uses proper code elements for command data', () => {
      const commandPairs = [createMockCommandPair()]
      
      render(<ArduinoCommandHistory {...defaultProps} commandPairs={commandPairs} />)
      
      const codeElements = screen.getAllByText(/SET_PIN_HIGH D2|OK - Pin D2 set to HIGH/)
      codeElements.forEach(element => {
        expect(element.tagName).toBe('CODE')
      })
    })
  })
})
