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
    mockGetRecentPairs.mockReturnValue([])
  })

  describe('HardwareDiagram Integration', () => {
    it('renders ArduinoCommandHistory component within HardwareDiagram', () => {
      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Should render the Arduino Command History section
      expect(screen.getByText('📡 Arduino Communication Log')).toBeInTheDocument()
      expect(screen.getByText('Last 5 Commands & Responses')).toBeInTheDocument()
    })

    it('displays connection status correctly in command history', () => {
      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Should show connected status
      expect(screen.getByText('Live')).toBeInTheDocument()
      const connectionIndicator = screen.getByText('Live').closest('.connection-indicator')
      expect(connectionIndicator).toHaveClass('connected')
    })

    it('displays disconnected status when hardware is offline', () => {
      const mockHardwareState = createMockHardwareState()
      mockHardwareState.connection.connected = false
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Should show offline status
      expect(screen.getByText('Offline')).toBeInTheDocument()
      const connectionIndicator = screen.getByText('Offline').closest('.connection-indicator')
      expect(connectionIndicator).toHaveClass('disconnected')
    })

    it('calls useArduinoCommandLog hook with correct parameters', () => {
      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Verify the hook was called
      expect(mockUseArduinoCommandLog).toHaveBeenCalledWith({ maxEntries: 50 })
    })

    it('displays command pairs when provided by the hook', () => {
      const mockCommandPairs = [
        {
          sent: {
            direction: 'sent' as const,
            data: 'SET_PIN_HIGH ENABLE_ALL',
            timestamp: Date.now(),
            type: 'command' as const,
            id: 'test-1',
            status: 'success' as const,
            responseTime: 25
          },
          received: {
            direction: 'received' as const,
            data: 'OK - Pin ENABLE_ALL set to HIGH',
            timestamp: Date.now() + 25,
            type: 'response' as const,
            id: 'test-1',
            status: 'success' as const,
            responseTime: 25
          }
        }
      ]

      mockGetRecentPairs.mockReturnValue(mockCommandPairs)

      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Should display the command pair
      expect(screen.getByText('📤 SENT')).toBeInTheDocument()
      expect(screen.getByText('📥 RECEIVED')).toBeInTheDocument()
      expect(screen.getByText('SET_PIN_HIGH ENABLE_ALL')).toBeInTheDocument()
      expect(screen.getByText('OK - Pin ENABLE_ALL set to HIGH')).toBeInTheDocument()
    })

    it('shows no commands message when history is empty', () => {
      mockGetRecentPairs.mockReturnValue([])

      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Should show empty state
      expect(screen.getByText('No commands sent yet')).toBeInTheDocument()
      expect(screen.getByText('Arduino commands will appear here in real-time')).toBeInTheDocument()
    })

    it('passes correct maxEntries to ArduinoCommandHistory', () => {
      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Verify getRecentPairs was called with maxEntries of 5
      expect(mockGetRecentPairs).toHaveBeenCalledWith(5)
    })
  })

  describe('Command Logging Integration', () => {
    it('integrates with command logging hook correctly', () => {
      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      render(
        <HardwareDiagram
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          highlightedPins={[]}
        />
      )

      // Verify the hook integration
      expect(mockUseArduinoCommandLog).toHaveBeenCalled()
      expect(mockGetRecentPairs).toHaveBeenCalled()
    })

    it('handles hook returning empty array gracefully', () => {
      mockGetRecentPairs.mockReturnValue([])

      const mockHardwareState = createMockHardwareState()
      const mockOnPinClick = vi.fn()

      expect(() => {
        render(
          <HardwareDiagram
            hardwareState={mockHardwareState}
            onPinClick={mockOnPinClick}
            highlightedPins={[]}
          />
        )
      }).not.toThrow()

      // Should still render the component
      expect(screen.getByText('📡 Arduino Communication Log')).toBeInTheDocument()
      // Should show empty state
      expect(screen.getByText('No commands sent yet')).toBeInTheDocument()
    })
  })
})
