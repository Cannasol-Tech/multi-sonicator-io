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
  connection: { connected: true, lastUpdate: Date.now() },
  pins: {
    'FREQ_DIV10_4': { state: 'LOW', lastUpdated: '12:31:45 AM' },
    'FREQ_LOCK_4': { state: 'HIGH', lastUpdated: '12:31:45 AM' },
    'RESET_4': { state: 'HIGH', lastUpdated: '12:31:45 AM' },
    'POWER_SENSE_4': { state: 512, lastUpdated: '12:31:45 AM' },
    'ENABLE_4': { state: 'HIGH', lastUpdated: '12:31:45 AM' },
    'AMPLITUDE_ALL': { state: 128, lastUpdated: '12:31:45 AM' },
    'UART_RXD': { state: 'LOW', lastUpdated: '12:31:45 AM' },
    'UART_TXD': { state: 'LOW', lastUpdated: '12:31:45 AM' },
    'STATUS_LED': { state: 'LOW', lastUpdated: '12:31:45 AM' }
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
