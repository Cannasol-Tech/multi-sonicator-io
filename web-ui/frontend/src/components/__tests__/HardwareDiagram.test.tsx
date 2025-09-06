import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import React from 'react'
import HardwareDiagram from '../HardwareDiagram'

// Mock hardware state
const mockHardwareState = {
  connected: true,
  pins: {
    'RESET_4': { state: 'HIGH', direction: 'OUT' as const, timestamp: Date.now() },
    'D8': { state: 'LOW', direction: 'OUT' as const, timestamp: Date.now() },
    'D9': { state: 'PWM=128', direction: 'OUT' as const, timestamp: Date.now() },
    'A0': { state: 'HIGH', direction: 'IN' as const, timestamp: Date.now() },
  },
  lastUpdate: Date.now()
}

describe('HardwareDiagram Component', () => {
  const mockProps = {
    hardwareState: mockHardwareState,
    highlightedPins: [] as string[],
    onPinHighlight: vi.fn(),
  }

  beforeEach(() => {
    vi.clearAllMocks()
  })

  describe('Rendering', () => {
    it('renders the hardware diagram container', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const diagram = screen.getByTestId('hardware-diagram')
      expect(diagram).toBeInTheDocument()
    })

    it('renders Arduino board representation', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      // Should show Arduino board elements
      expect(screen.getByText(/Arduino/i)).toBeInTheDocument()
    })

    it('renders pin labels and states', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      // Check for pin labels
      expect(screen.getByText('RESET_4')).toBeInTheDocument()
      expect(screen.getByText('D8')).toBeInTheDocument()
      expect(screen.getByText('D9')).toBeInTheDocument()
      expect(screen.getByText('A0')).toBeInTheDocument()
    })

    it('displays pin states correctly', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      // Check for pin states
      expect(screen.getByText('HIGH')).toBeInTheDocument()
      expect(screen.getByText('LOW')).toBeInTheDocument()
      expect(screen.getByText('PWM=128')).toBeInTheDocument()
    })

    it('shows pin directions', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      // Should show input/output indicators
      const outPins = screen.getAllByText('OUT')
      const inPins = screen.getAllByText('IN')
      
      expect(outPins).toHaveLength(3) // RESET_4, D8, D9
      expect(inPins).toHaveLength(1) // A0
    })
  })

  describe('Pin Interactions', () => {
    it('calls onPinHighlight when pin is clicked', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const resetPin = screen.getByText('RESET_4').closest('.pin')
      fireEvent.click(resetPin!)
      
      expect(mockProps.onPinHighlight).toHaveBeenCalledWith('RESET_4')
    })

    it('highlights pins when specified in highlightedPins prop', () => {
      const propsWithHighlight = {
        ...mockProps,
        highlightedPins: ['D8', 'D9']
      }
      
      render(<HardwareDiagram {...propsWithHighlight} />)
      
      const d8Pin = screen.getByText('D8').closest('.pin')
      const d9Pin = screen.getByText('D9').closest('.pin')
      
      expect(d8Pin).toHaveClass('highlighted')
      expect(d9Pin).toHaveClass('highlighted')
    })

    it('removes highlight when pin is not in highlightedPins', () => {
      const { rerender } = render(<HardwareDiagram {...mockProps} highlightedPins={['D8']} />)
      
      let d8Pin = screen.getByText('D8').closest('.pin')
      expect(d8Pin).toHaveClass('highlighted')
      
      rerender(<HardwareDiagram {...mockProps} highlightedPins={[]} />)
      
      d8Pin = screen.getByText('D8').closest('.pin')
      expect(d8Pin).not.toHaveClass('highlighted')
    })

    it('handles multiple pin highlights correctly', () => {
      const propsWithMultipleHighlights = {
        ...mockProps,
        highlightedPins: ['RESET_4', 'D8', 'A0']
      }
      
      render(<HardwareDiagram {...propsWithMultipleHighlights} />)
      
      const resetPin = screen.getByText('RESET_4').closest('.pin')
      const d8Pin = screen.getByText('D8').closest('.pin')
      const a0Pin = screen.getByText('A0').closest('.pin')
      
      expect(resetPin).toHaveClass('highlighted')
      expect(d8Pin).toHaveClass('highlighted')
      expect(a0Pin).toHaveClass('highlighted')
    })
  })

  describe('Pin State Display', () => {
    it('applies correct CSS classes for pin states', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const highPin = screen.getByText('RESET_4').closest('.pin')
      const lowPin = screen.getByText('D8').closest('.pin')
      const pwmPin = screen.getByText('D9').closest('.pin')
      
      expect(highPin).toHaveClass('pin-high')
      expect(lowPin).toHaveClass('pin-low')
      expect(pwmPin).toHaveClass('pin-pwm')
    })

    it('applies correct CSS classes for pin directions', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const outputPin = screen.getByText('D8').closest('.pin')
      const inputPin = screen.getByText('A0').closest('.pin')
      
      expect(outputPin).toHaveClass('pin-output')
      expect(inputPin).toHaveClass('pin-input')
    })

    it('handles PWM values correctly', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      expect(screen.getByText('PWM=128')).toBeInTheDocument()
      
      const pwmPin = screen.getByText('D9').closest('.pin')
      expect(pwmPin).toHaveClass('pin-pwm')
    })

    it('updates pin states when hardware state changes', () => {
      const { rerender } = render(<HardwareDiagram {...mockProps} />)
      
      expect(screen.getByText('HIGH')).toBeInTheDocument()
      
      const updatedState = {
        ...mockHardwareState,
        pins: {
          ...mockHardwareState.pins,
          'RESET_4': { state: 'LOW', direction: 'OUT' as const, timestamp: Date.now() }
        }
      }
      
      rerender(<HardwareDiagram {...mockProps} hardwareState={updatedState} />)
      
      // Should now show LOW instead of HIGH for RESET_4
      const lowStates = screen.getAllByText('LOW')
      expect(lowStates.length).toBeGreaterThan(1) // D8 and RESET_4
    })
  })

  describe('Connection Status', () => {
    it('shows disconnected state when not connected', () => {
      const disconnectedState = {
        ...mockHardwareState,
        connected: false
      }
      
      render(<HardwareDiagram {...mockProps} hardwareState={disconnectedState} />)
      
      const diagram = screen.getByTestId('hardware-diagram')
      expect(diagram).toHaveClass('disconnected')
    })

    it('shows connected state when connected', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const diagram = screen.getByTestId('hardware-diagram')
      expect(diagram).toHaveClass('connected')
    })

    it('disables pin interactions when disconnected', () => {
      const disconnectedState = {
        ...mockHardwareState,
        connected: false
      }
      
      render(<HardwareDiagram {...mockProps} hardwareState={disconnectedState} />)
      
      const resetPin = screen.getByText('RESET_4').closest('.pin')
      fireEvent.click(resetPin!)
      
      // Should not call onPinHighlight when disconnected
      expect(mockProps.onPinHighlight).not.toHaveBeenCalled()
    })
  })

  describe('Visual Feedback', () => {
    it('shows hover effects on pins', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const resetPin = screen.getByText('RESET_4').closest('.pin')
      
      fireEvent.mouseEnter(resetPin!)
      expect(resetPin).toHaveClass('pin-hover')
      
      fireEvent.mouseLeave(resetPin!)
      expect(resetPin).not.toHaveClass('pin-hover')
    })

    it('shows animation when pin state changes', async () => {
      const { rerender } = render(<HardwareDiagram {...mockProps} />)
      
      const updatedState = {
        ...mockHardwareState,
        pins: {
          ...mockHardwareState.pins,
          'D8': { state: 'HIGH', direction: 'OUT' as const, timestamp: Date.now() }
        }
      }
      
      rerender(<HardwareDiagram {...mockProps} hardwareState={updatedState} />)
      
      const d8Pin = screen.getByText('D8').closest('.pin')
      
      await waitFor(() => {
        expect(d8Pin).toHaveClass('pin-updated')
      })
    })
  })

  describe('Accessibility', () => {
    it('has proper ARIA labels for pins', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const resetPin = screen.getByLabelText(/RESET_4.*HIGH.*OUT/i)
      expect(resetPin).toBeInTheDocument()
    })

    it('supports keyboard navigation', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      const resetPin = screen.getByText('RESET_4').closest('.pin')
      
      // Should be focusable
      resetPin?.focus()
      expect(document.activeElement).toBe(resetPin)
      
      // Should respond to Enter key
      fireEvent.keyDown(resetPin!, { key: 'Enter' })
      expect(mockProps.onPinHighlight).toHaveBeenCalledWith('RESET_4')
    })

    it('provides screen reader friendly descriptions', () => {
      render(<HardwareDiagram {...mockProps} />)
      
      // Should have descriptive text for screen readers
      expect(screen.getByText(/Arduino hardware diagram/i)).toBeInTheDocument()
    })
  })

  describe('Error Handling', () => {
    it('handles missing pin data gracefully', () => {
      const stateWithMissingPins = {
        connected: true,
        pins: {},
        lastUpdate: Date.now()
      }
      
      expect(() => {
        render(<HardwareDiagram {...mockProps} hardwareState={stateWithMissingPins} />)
      }).not.toThrow()
    })

    it('handles invalid pin states gracefully', () => {
      const stateWithInvalidPin = {
        ...mockHardwareState,
        pins: {
          'D8': { state: 'INVALID', direction: 'OUT' as const, timestamp: Date.now() }
        }
      }
      
      expect(() => {
        render(<HardwareDiagram {...mockProps} hardwareState={stateWithInvalidPin} />)
      }).not.toThrow()
    })
  })
})
