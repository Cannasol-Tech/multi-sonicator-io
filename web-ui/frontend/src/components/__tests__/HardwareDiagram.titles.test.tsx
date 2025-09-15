import { render, screen } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import HardwareDiagram from '../HardwareDiagram'
import { HardwareState } from '../../types'

// Mock the TestAutomationAPI
vi.mock('../../services/testAutomationApi', () => ({
  TestAutomationAPI: {
    getStatusIcon: (status: string) => {
      switch (status) {
        case 'HIGH': return '🟢'
        case 'LOW': return '🔴'
        default: return '⚪'
      }
    }
  }
}))

const mockHardwareState: HardwareState = {
  connection: { connected: true },
  pins: {
    'FREQ_DIV10_4': { pin: 'FREQ_DIV10_4', signal: 'FREQ_DIV10_4', direction: 'IN', state: 'LOW', timestamp: Date.now() },
    'FREQ_LOCK_4': { pin: 'FREQ_LOCK_4', signal: 'FREQ_LOCK_4', direction: 'IN', state: 'HIGH', timestamp: Date.now() },
    'RESET_4': { pin: 'RESET_4', signal: 'RESET_4', direction: 'OUT', state: 'HIGH', timestamp: Date.now() },
    'POWER_SENSE_4': { pin: 'POWER_SENSE_4', signal: 'POWER_SENSE_4', direction: 'ANALOG', state: 512, timestamp: Date.now() },
    'ENABLE_4': { pin: 'ENABLE_4', signal: 'ENABLE_4', direction: 'OUT', state: 'HIGH', timestamp: Date.now() },
    'AMPLITUDE_ALL': { pin: 'AMPLITUDE_ALL', signal: 'AMPLITUDE_ALL', direction: 'OUT', state: 128, timestamp: Date.now() },
    'UART_RXD': { pin: 'UART_RXD', signal: 'UART_RXD', direction: 'IN', state: 'LOW', timestamp: Date.now() },
    'UART_TXD': { pin: 'UART_TXD', signal: 'UART_TXD', direction: 'OUT', state: 'LOW', timestamp: Date.now() },
    'STATUS_LED': { pin: 'STATUS_LED', signal: 'STATUS_LED', direction: 'OUT', state: 'LOW', timestamp: Date.now() }
  }
}

const mockProps = {
  hardwareState: mockHardwareState,
  onPinClick: vi.fn(),
  onConnectionHover: vi.fn(),
  selectedSignal: null,
  hoveredSignal: null
}

describe('HardwareDiagram - Enhanced Titles', () => {
  it('should render Arduino Uno R3 title with enhanced styling', () => {
    render(<HardwareDiagram {...mockProps} />)
    
    const arduinoTitle = screen.getByText('Arduino Uno R3')
    expect(arduinoTitle).toBeInTheDocument()
    expect(arduinoTitle.tagName).toBe('H3')
    expect(arduinoTitle.closest('.device-header')).toBeInTheDocument()
    expect(arduinoTitle.closest('.arduino-container')).toBeInTheDocument()
  })

  it('should render ATmega32A title with enhanced styling', () => {
    render(<HardwareDiagram {...mockProps} />)
    
    const atmegaTitle = screen.getByText('ATmega32A')
    expect(atmegaTitle).toBeInTheDocument()
    expect(atmegaTitle.tagName).toBe('H3')
    expect(atmegaTitle.closest('.device-header')).toBeInTheDocument()
    expect(atmegaTitle.closest('.atmega-container')).toBeInTheDocument()
  })

  it('should render device subtitles correctly', () => {
    render(<HardwareDiagram {...mockProps} />)
    
    expect(screen.getByText('HIL Test Wrapper')).toBeInTheDocument()
    expect(screen.getByText('Device Under Test')).toBeInTheDocument()
  })

  it('should use large device containers for enhanced display', () => {
    const { container } = render(<HardwareDiagram {...mockProps} />)
    
    const arduinoContainer = container.querySelector('.device-container-large.arduino-container')
    const atmegaContainer = container.querySelector('.device-container-large.atmega-container')
    
    expect(arduinoContainer).toBeInTheDocument()
    expect(atmegaContainer).toBeInTheDocument()
  })

  it('should have proper CSS classes for theme support', () => {
    const { container } = render(<HardwareDiagram {...mockProps} />)
    
    // Check that device headers exist
    const deviceHeaders = container.querySelectorAll('.device-header')
    expect(deviceHeaders.length).toBeGreaterThanOrEqual(2)
    
    // Check that titles are h3 elements
    const titleElements = container.querySelectorAll('.device-header h3')
    expect(titleElements.length).toBeGreaterThanOrEqual(2)
  })

  it('should maintain readability in both light and dark themes', () => {
    // Test light theme (default)
    const { container, rerender } = render(<HardwareDiagram {...mockProps} />)
    
    const arduinoTitle = screen.getByText('Arduino Uno R3')
    const atmegaTitle = screen.getByText('ATmega32A')
    
    expect(arduinoTitle).toBeInTheDocument()
    expect(atmegaTitle).toBeInTheDocument()
    
    // Test dark theme
    document.documentElement.classList.add('dark-theme')
    rerender(<HardwareDiagram {...mockProps} />)
    
    // Titles should still be visible
    expect(screen.getByText('Arduino Uno R3')).toBeInTheDocument()
    expect(screen.getByText('ATmega32A')).toBeInTheDocument()
    
    // Clean up
    document.documentElement.classList.remove('dark-theme')
  })

  it('should have enhanced typography for better readability', () => {
    render(<HardwareDiagram {...mockProps} />)
    
    // Check that both hardware component titles are present and readable
    const arduinoTitle = screen.getByText('Arduino Uno R3')
    const atmegaTitle = screen.getByText('ATmega32A')
    
    // Verify they are properly structured
    expect(arduinoTitle.closest('.device-container-large')).toBeInTheDocument()
    expect(atmegaTitle.closest('.device-container-large')).toBeInTheDocument()
    
    // Verify subtitles are present
    expect(screen.getByText('HIL Test Wrapper')).toHaveClass('device-subtitle')
    expect(screen.getByText('Device Under Test')).toHaveClass('device-subtitle')
  })

  it('should display titles prominently for easy identification', () => {
    render(<HardwareDiagram {...mockProps} />)
    
    // Both titles should be easily identifiable
    const titles = screen.getAllByRole('heading', { level: 3 })
    const titleTexts = titles.map(title => title.textContent)
    
    expect(titleTexts).toContain('Arduino Uno R3')
    expect(titleTexts).toContain('ATmega32A')
  })
})
