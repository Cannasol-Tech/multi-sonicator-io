import React from 'react'
import { render, screen } from '@testing-library/react'
import { vi, describe, it, expect, beforeEach } from 'vitest'
import ArduinoCommandHistory from '../ArduinoCommandHistory'
import { ArduinoCommand } from '../../hooks/useArduinoCommandLog'

// Create mock command pairs for testing
const createMockCommandPair = (sentOverrides: Partial<ArduinoCommand> = {}, receivedOverrides: Partial<ArduinoCommand> = {}) => {
  const id = `pair-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`
  return {
    sent: {
      direction: 'sent' as const,
      data: 'SET_PIN_HIGH D2',
      timestamp: Date.now(),
      type: 'command' as const,
      id,
      status: 'success' as const,
      responseTime: 25,
      ...sentOverrides
    },
    received: {
      direction: 'received' as const,
      data: 'OK - Pin D2 set to HIGH',
      timestamp: Date.now() + 25,
      type: 'response' as const,
      id,
      status: 'success' as const,
      responseTime: 25,
      ...receivedOverrides
    }
  }
}

describe('Arduino Command History Integration Tests', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  describe('Component Integration', () => {
    it('renders with command pairs and displays them correctly', () => {
      const commandPairs = [
        createMockCommandPair(
          { data: 'ENABLE_ALL HIGH' },
          { data: 'OK - ENABLE_ALL set to HIGH' }
        ),
        createMockCommandPair(
          { data: 'FREQUENCY_ALL 1000', status: 'error', error: 'Invalid frequency' },
          { data: 'ERROR - Invalid frequency value', status: 'error', error: 'Invalid frequency' }
        )
      ]

      render(
        <ArduinoCommandHistory
          commandPairs={commandPairs}
          connected={true}
          maxEntries={5}
        />
      )

      // Should render header
      expect(screen.getByText('📡 Arduino Communication Log')).toBeInTheDocument()
      expect(screen.getByText('Last 5 Commands & Responses')).toBeInTheDocument()

      // Should show connection status
      expect(screen.getByText('Live')).toBeInTheDocument()

      // Should display both command pairs
      expect(screen.getAllByText('📤 SENT')).toHaveLength(2)
      expect(screen.getAllByText('📥 RECEIVED')).toHaveLength(2)

      // Should display command data
      expect(screen.getByText('ENABLE_ALL HIGH')).toBeInTheDocument()
      expect(screen.getByText('OK - ENABLE_ALL set to HIGH')).toBeInTheDocument()
      expect(screen.getByText('FREQUENCY_ALL 1000')).toBeInTheDocument()
      expect(screen.getByText('ERROR - Invalid frequency value')).toBeInTheDocument()

      // Should display error message
      expect(screen.getByText('⚠️ Invalid frequency')).toBeInTheDocument()

      // Should display statistics
      expect(screen.getByText('📊 Total: 2')).toBeInTheDocument()
      expect(screen.getByText('✅ Success: 1')).toBeInTheDocument()
      expect(screen.getByText('❌ Errors: 1')).toBeInTheDocument()
    })

    it('handles different response times and displays appropriate indicators', () => {
      const commandPairs = [
        createMockCommandPair({ responseTime: 5 }, { responseTime: 5 }),
        createMockCommandPair({ responseTime: 50 }, { responseTime: 50 }),
        createMockCommandPair({ responseTime: 200 }, { responseTime: 200 }),
        createMockCommandPair({ responseTime: 600 }, { responseTime: 600 })
      ]

      render(
        <ArduinoCommandHistory
          commandPairs={commandPairs}
          connected={true}
          maxEntries={10}
        />
      )

      // Should display different response time indicators
      expect(screen.getByText('Response: 5ms 🟢')).toBeInTheDocument()
      expect(screen.getByText('Response: 50ms 🟡')).toBeInTheDocument()
      expect(screen.getByText('Response: 200ms 🟠')).toBeInTheDocument()
      expect(screen.getByText('Response: 600ms 🔴')).toBeInTheDocument()
    })

    it('respects maxEntries limit', () => {
      const commandPairs = Array.from({ length: 10 }, (_, i) =>
        createMockCommandPair({ data: `Command ${i}` }, { data: `Response ${i}` })
      )

      render(
        <ArduinoCommandHistory
          commandPairs={commandPairs}
          connected={true}
          maxEntries={3}
        />
      )

      // Should only display first 3 commands
      expect(screen.getByText('Command 0')).toBeInTheDocument()
      expect(screen.getByText('Command 1')).toBeInTheDocument()
      expect(screen.getByText('Command 2')).toBeInTheDocument()
      expect(screen.queryByText('Command 3')).not.toBeInTheDocument()

      // Statistics should reflect all commands passed in (not just displayed)
      expect(screen.getByText('📊 Total: 10')).toBeInTheDocument()
    })

    it('handles missing received commands gracefully', () => {
      const commandPairs = [
        {
          sent: {
            direction: 'sent' as const,
            data: 'PING',
            timestamp: Date.now(),
            type: 'command' as const,
            id: 'test-1',
            status: 'pending' as const
          },
          received: undefined
        }
      ]

      render(
        <ArduinoCommandHistory
          commandPairs={commandPairs}
          connected={true}
          maxEntries={5}
        />
      )

      // Should show sent command
      expect(screen.getByText('📤 SENT')).toBeInTheDocument()
      expect(screen.getByText('PING')).toBeInTheDocument()

      // Should show no response message
      expect(screen.getByText('No response received')).toBeInTheDocument()
    })

    it('displays connection status correctly', () => {
      const commandPairs = [createMockCommandPair()]

      // Test connected state
      const { rerender } = render(
        <ArduinoCommandHistory
          commandPairs={commandPairs}
          connected={true}
          maxEntries={5}
        />
      )

      expect(screen.getByText('Live')).toBeInTheDocument()
      expect(screen.getByText('Live').closest('.connection-indicator')).toHaveClass('connected')

      // Test disconnected state
      rerender(
        <ArduinoCommandHistory
          commandPairs={commandPairs}
          connected={false}
          maxEntries={5}
        />
      )

      expect(screen.getByText('Offline')).toBeInTheDocument()
      expect(screen.getByText('Offline').closest('.connection-indicator')).toHaveClass('disconnected')
    })

    it('handles click events when onCommandClick is provided', () => {
      const onCommandClick = vi.fn()
      const commandPairs = [createMockCommandPair()]

      render(
        <ArduinoCommandHistory
          commandPairs={commandPairs}
          connected={true}
          maxEntries={5}
          onCommandClick={onCommandClick}
        />
      )

      // Should have clickable class
      const sentCommand = screen.getByText('SET_PIN_HIGH D2').closest('.command-entry')
      expect(sentCommand).toHaveClass('clickable')

      // Should call onClick when clicked
      sentCommand?.click()
      expect(onCommandClick).toHaveBeenCalledWith(commandPairs[0].sent)
    })
  })
})
