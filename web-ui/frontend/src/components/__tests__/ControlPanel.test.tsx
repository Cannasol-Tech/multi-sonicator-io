import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import React from 'react'
import ControlPanel from '../ControlPanel'
import { HardwareState, PinState } from '../../types'

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
  const mockPinState: PinState = {
    pin: 'D7',
    signal: 'FREQ_DIV10_4',
    direction: 'OUT',
    state: 'LOW',
    timestamp: Date.now(),
    description: 'Test pin'
  }

  const mockHardwareState: HardwareState = {
    connection: {
      connected: true,
      port: '/dev/ttyUSB0'
    },
    pins: {
      'FREQ_DIV10_4': mockPinState,
      'FREQ_LOCK_4': { ...mockPinState, signal: 'FREQ_LOCK_4', pin: 'D8' },
      'OVERLOAD_4': { ...mockPinState, signal: 'OVERLOAD_4', pin: 'A2', direction: 'IN' },
      'START_4': { ...mockPinState, signal: 'START_4', pin: 'A3' },
      'RESET_4': { ...mockPinState, signal: 'RESET_4', pin: 'A4' },
      'POWER_SENSE_4': { ...mockPinState, signal: 'POWER_SENSE_4', pin: 'A1', direction: 'ANALOG', state: 512 },
      'AMPLITUDE_ALL': { ...mockPinState, signal: 'AMPLITUDE_ALL', pin: 'D9' },
      'UART_RXD': { ...mockPinState, signal: 'UART_RXD', pin: 'D10', direction: 'IN' },
      'UART_TXD': { ...mockPinState, signal: 'UART_TXD', pin: 'D11' },
      'STATUS_LED': { ...mockPinState, signal: 'STATUS_LED', pin: 'D12' }
    },
    lastUpdate: Date.now()
  }

  const mockProps = {
    hardwareState: mockHardwareState,
    onPinControl: vi.fn(),
    connected: true
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

  describe('Tabbed Interface', () => {
    it('renders sub-tab navigation with correct buttons', () => {
      render(<ControlPanel {...mockProps} />)

      expect(screen.getByText('🔧 Configurable Parameters')).toBeInTheDocument()
      expect(screen.getByText('📊 Live DUT Monitoring')).toBeInTheDocument()
    })

    it('shows parameters tab content by default', () => {
      render(<ControlPanel {...mockProps} />)

      expect(screen.getByText('Configurable Parameters')).toBeInTheDocument()
      expect(screen.getByText('Click to toggle HIGH/LOW states')).toBeInTheDocument()
    })

    it('switches to monitoring tab when clicked', () => {
      render(<ControlPanel {...mockProps} />)

      const monitoringTab = screen.getByText('📊 Live DUT Monitoring')
      fireEvent.click(monitoringTab)

      expect(screen.getByText('Live DUT Monitoring')).toBeInTheDocument()
      expect(screen.getByText('Real-time signals from ATmega32A')).toBeInTheDocument()
    })

    it('applies active class to selected tab', () => {
      render(<ControlPanel {...mockProps} />)

      const parametersTab = screen.getByText('🔧 Configurable Parameters')
      const monitoringTab = screen.getByText('📊 Live DUT Monitoring')

      expect(parametersTab).toHaveClass('active')
      expect(monitoringTab).not.toHaveClass('active')

      fireEvent.click(monitoringTab)

      expect(parametersTab).not.toHaveClass('active')
      expect(monitoringTab).toHaveClass('active')
    })
  })

  describe('Compact Parameter Cards', () => {
    it('renders parameter descriptions as bold headers', () => {
      render(<ControlPanel {...mockProps} />)

      // Should show the description as the main header, not the signal name
      expect(screen.getByText('Sonicator #4 Overload Protection Signal')).toBeInTheDocument()
      expect(screen.getByText('Sonicator #4 Power Sensing (5.44 mV/W scaling)')).toBeInTheDocument()
    })

    it('shows signal names and connection info in compact row', () => {
      render(<ControlPanel {...mockProps} />)

      // Signal names should still be visible but in the info row
      expect(screen.getByText('OVERLOAD_4')).toBeInTheDocument()
      expect(screen.getByText('POWER_SENSE_4')).toBeInTheDocument()

      // Connection info should be in compact format
      expect(screen.getByText('A2 → PD3, Pin 17')).toBeInTheDocument()
      expect(screen.getByText('A1 → PA7, Pin 33')).toBeInTheDocument()
    })

    it('uses compact parameter card styling', () => {
      render(<ControlPanel {...mockProps} />)

      const parameterCards = document.querySelectorAll('.parameter-card-compact')
      expect(parameterCards.length).toBeGreaterThan(0)
    })
  })
})
