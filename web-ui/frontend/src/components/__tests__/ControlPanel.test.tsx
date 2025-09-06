import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import React from 'react'
import ControlPanel from '../ControlPanel'

// Mock WebSocket hook
const mockSendMessage = vi.fn()
vi.mock('../../hooks/useWebSocket', () => ({
  default: () => ({
    connected: true,
    sendMessage: mockSendMessage,
    lastMessage: null
  })
}))

describe('ControlPanel Component', () => {
  const mockProps = {
    connected: true,
    onSendCommand: vi.fn(),
  }

  beforeEach(() => {
    vi.clearAllMocks()
  })

  describe('Rendering', () => {
    it('renders the control panel container', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByText(/Control Panel/i)).toBeInTheDocument()
    })

    it('renders pin control sections', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByText(/Digital Pins/i)).toBeInTheDocument()
      expect(screen.getByText(/Analog Pins/i)).toBeInTheDocument()
      expect(screen.getByText(/PWM Pins/i)).toBeInTheDocument()
    })

    it('renders frequency control section', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByText(/Frequency Control/i)).toBeInTheDocument()
    })

    it('shows connection status', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByText(/Connected/i)).toBeInTheDocument()
    })
  })

  describe('Digital Pin Controls', () => {
    it('renders digital pin buttons', () => {
      render(<ControlPanel {...mockProps} />)
      
      // Should have buttons for common digital pins
      expect(screen.getByText('D8')).toBeInTheDocument()
      expect(screen.getByText('D9')).toBeInTheDocument()
      expect(screen.getByText('D10')).toBeInTheDocument()
    })

    it('sends HIGH command when HIGH button is clicked', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const d8HighButton = screen.getByText('D8').closest('.pin-control')?.querySelector('.btn-high')
      fireEvent.click(d8HighButton!)
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith({
          type: 'pin_write',
          pin: 'D8',
          value: 'HIGH'
        })
      })
    })

    it('sends LOW command when LOW button is clicked', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const d8LowButton = screen.getByText('D8').closest('.pin-control')?.querySelector('.btn-low')
      fireEvent.click(d8LowButton!)
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith({
          type: 'pin_write',
          pin: 'D8',
          value: 'LOW'
        })
      })
    })

    it('disables pin controls when disconnected', () => {
      render(<ControlPanel {...mockProps} connected={false} />)
      
      const d8HighButton = screen.getByText('D8').closest('.pin-control')?.querySelector('.btn-high')
      expect(d8HighButton).toBeDisabled()
    })
  })

  describe('PWM Pin Controls', () => {
    it('renders PWM sliders', () => {
      render(<ControlPanel {...mockProps} />)
      
      // Should have sliders for PWM pins
      const pwmSliders = screen.getAllByRole('slider')
      expect(pwmSliders.length).toBeGreaterThan(0)
    })

    it('sends PWM command when slider value changes', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const pwmSlider = screen.getAllByRole('slider')[0]
      fireEvent.change(pwmSlider, { target: { value: '128' } })
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith(
          expect.objectContaining({
            type: 'pin_write',
            value: 'PWM=128'
          })
        )
      })
    })

    it('displays current PWM value', () => {
      render(<ControlPanel {...mockProps} />)
      
      const pwmSlider = screen.getAllByRole('slider')[0]
      fireEvent.change(pwmSlider, { target: { value: '200' } })
      
      expect(screen.getByDisplayValue('200')).toBeInTheDocument()
    })

    it('constrains PWM values to valid range', () => {
      render(<ControlPanel {...mockProps} />)
      
      const pwmSlider = screen.getAllByRole('slider')[0]
      
      // Test minimum value
      fireEvent.change(pwmSlider, { target: { value: '-10' } })
      expect(pwmSlider).toHaveValue('0')
      
      // Test maximum value
      fireEvent.change(pwmSlider, { target: { value: '300' } })
      expect(pwmSlider).toHaveValue('255')
    })
  })

  describe('Analog Pin Controls', () => {
    it('renders analog pin read buttons', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByText('A0')).toBeInTheDocument()
      expect(screen.getByText('A1')).toBeInTheDocument()
    })

    it('sends analog read command when button is clicked', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const a0Button = screen.getByText('A0').closest('.pin-control')?.querySelector('.btn-read')
      fireEvent.click(a0Button!)
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith({
          type: 'pin_read',
          pin: 'A0'
        })
      })
    })
  })

  describe('Frequency Control', () => {
    it('renders frequency input field', () => {
      render(<ControlPanel {...mockProps} />)
      
      const frequencyInput = screen.getByLabelText(/frequency/i)
      expect(frequencyInput).toBeInTheDocument()
    })

    it('renders frequency control buttons', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByText(/Start/i)).toBeInTheDocument()
      expect(screen.getByText(/Stop/i)).toBeInTheDocument()
    })

    it('sends frequency start command with specified frequency', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const frequencyInput = screen.getByLabelText(/frequency/i)
      const startButton = screen.getByText(/Start/i)
      
      fireEvent.change(frequencyInput, { target: { value: '1000' } })
      fireEvent.click(startButton)
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith({
          type: 'frequency_start',
          frequency: 1000
        })
      })
    })

    it('sends frequency stop command', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const stopButton = screen.getByText(/Stop/i)
      fireEvent.click(stopButton)
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith({
          type: 'frequency_stop'
        })
      })
    })

    it('validates frequency input range', () => {
      render(<ControlPanel {...mockProps} />)
      
      const frequencyInput = screen.getByLabelText(/frequency/i)
      
      // Test invalid frequency
      fireEvent.change(frequencyInput, { target: { value: '-100' } })
      fireEvent.blur(frequencyInput)
      
      expect(frequencyInput).toHaveValue('0')
    })
  })

  describe('Bulk Operations', () => {
    it('renders bulk control section', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByText(/Bulk Operations/i)).toBeInTheDocument()
    })

    it('provides reset all pins functionality', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const resetButton = screen.getByText(/Reset All/i)
      fireEvent.click(resetButton)
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith({
          type: 'reset_all'
        })
      })
    })

    it('provides set all pins high functionality', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const setAllHighButton = screen.getByText(/Set All High/i)
      fireEvent.click(setAllHighButton)
      
      await waitFor(() => {
        expect(mockProps.onSendCommand).toHaveBeenCalledWith({
          type: 'set_all_high'
        })
      })
    })
  })

  describe('Connection Status Handling', () => {
    it('shows disconnected state when not connected', () => {
      render(<ControlPanel {...mockProps} connected={false} />)
      
      expect(screen.getByText(/Disconnected/i)).toBeInTheDocument()
    })

    it('disables all controls when disconnected', () => {
      render(<ControlPanel {...mockProps} connected={false} />)
      
      const buttons = screen.getAllByRole('button')
      const sliders = screen.getAllByRole('slider')
      const inputs = screen.getAllByRole('textbox')
      
      buttons.forEach(button => {
        expect(button).toBeDisabled()
      })
      
      sliders.forEach(slider => {
        expect(slider).toBeDisabled()
      })
      
      inputs.forEach(input => {
        expect(input).toBeDisabled()
      })
    })

    it('enables controls when connected', () => {
      render(<ControlPanel {...mockProps} connected={true} />)
      
      const buttons = screen.getAllByRole('button')
      const sliders = screen.getAllByRole('slider')
      
      buttons.forEach(button => {
        expect(button).not.toBeDisabled()
      })
      
      sliders.forEach(slider => {
        expect(slider).not.toBeDisabled()
      })
    })
  })

  describe('Error Handling', () => {
    it('handles command send failures gracefully', async () => {
      const mockOnSendCommand = vi.fn().mockRejectedValue(new Error('Send failed'))
      
      render(<ControlPanel {...mockProps} onSendCommand={mockOnSendCommand} />)
      
      const d8HighButton = screen.getByText('D8').closest('.pin-control')?.querySelector('.btn-high')
      
      expect(() => {
        fireEvent.click(d8HighButton!)
      }).not.toThrow()
    })

    it('validates input values before sending commands', async () => {
      render(<ControlPanel {...mockProps} />)
      
      const frequencyInput = screen.getByLabelText(/frequency/i)
      const startButton = screen.getByText(/Start/i)
      
      // Try to send invalid frequency
      fireEvent.change(frequencyInput, { target: { value: 'invalid' } })
      fireEvent.click(startButton)
      
      // Should not send command with invalid frequency
      expect(mockProps.onSendCommand).not.toHaveBeenCalledWith(
        expect.objectContaining({
          frequency: 'invalid'
        })
      )
    })
  })

  describe('Accessibility', () => {
    it('has proper labels for all controls', () => {
      render(<ControlPanel {...mockProps} />)
      
      expect(screen.getByLabelText(/frequency/i)).toBeInTheDocument()
      
      // PWM sliders should have labels
      const sliders = screen.getAllByRole('slider')
      sliders.forEach(slider => {
        expect(slider).toHaveAccessibleName()
      })
    })

    it('supports keyboard navigation', () => {
      render(<ControlPanel {...mockProps} />)
      
      const firstButton = screen.getAllByRole('button')[0]
      firstButton.focus()
      
      expect(document.activeElement).toBe(firstButton)
    })

    it('provides appropriate ARIA states for disabled controls', () => {
      render(<ControlPanel {...mockProps} connected={false} />)
      
      const buttons = screen.getAllByRole('button')
      buttons.forEach(button => {
        expect(button).toHaveAttribute('aria-disabled', 'true')
      })
    })
  })
})
