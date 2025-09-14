import React from 'react'
import { render, fireEvent, screen, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { vi } from 'vitest'
import AdvancedControlPanel from '../AdvancedControlPanel'

describe('AdvancedControlPanel', () => {
  // Mock hardwareState
  const mockHardwareState = {
    pins: {
      'START_4': { state: 'LOW' },
      'RESET_4': { state: 'LOW' },
      'POWER_SENSE_4': { state: 512 }, // ~50% power
      'FREQ_DIV10_4': { state: 'LOW' },
      'FREQ_LOCK_4': { state: 'HIGH' },
      'OVERLOAD_4': { state: 'LOW' }
    }
  }

  // Mock handlers
  const mockOnPinClick = vi.fn()
  const mockOnBatchOperation = vi.fn()

  // Mock fetch
  const mockFetch = vi.fn(() => Promise.resolve({ ok: true }))
  global.fetch = mockFetch as any

  // Mock localStorage
  const mockLocalStorage = {
    getItem: vi.fn(),
    setItem: vi.fn(),
    clear: vi.fn()
  }
  Object.defineProperty(global, 'localStorage', { value: mockLocalStorage })

  beforeEach(() => {
    vi.clearAllMocks()
    mockLocalStorage.getItem.mockReturnValue(null)
  })

  // Clean up timers
  afterEach(() => {
    vi.clearAllTimers()
  })

  describe('Rendering', () => {
    it('renders without crashing', () => {
      const { container } = render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          onBatchOperation={mockOnBatchOperation}
        />
      )
      expect(container).toBeTruthy()
    })

    it('renders all tab buttons', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )
      expect(screen.getByText('📋 Presets')).toBeInTheDocument()
      expect(screen.getByText('⚡ Batch Ops')).toBeInTheDocument()
      expect(screen.getByText('🛠️ Custom')).toBeInTheDocument()
    })

    it('renders power control section', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )
      expect(screen.getByText('🔋 Sonicator #4 Power Control')).toBeInTheDocument()
      expect(screen.getByRole('slider')).toBeInTheDocument()
      expect(screen.getAllByText('50%')).toHaveLength(2) // Power display and preset button // Based on mockHardwareState
    })
  })

  describe('Presets', () => {
    it('renders default presets', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )
      expect(screen.getByText('All Pins LOW')).toBeInTheDocument()
      expect(screen.getByText('Start Sequence')).toBeInTheDocument()
      expect(screen.getByText('Reset Sequence')).toBeInTheDocument()
      expect(screen.getByText('Calibration Mode')).toBeInTheDocument()
    })

    it('loads saved presets from localStorage', () => {
      const savedPresets = [{
        id: 'custom-123',
        name: 'Saved Preset',
        description: 'Test preset',
        pinStates: { 'START_4': 'HIGH' },
        category: 'custom'
      }]
      mockLocalStorage.getItem.mockReturnValue(JSON.stringify(savedPresets))

      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )
      expect(screen.getByText('Saved Preset')).toBeInTheDocument()
    })

    it('applies preset when clicked', async () => {
      vi.useFakeTimers()
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      const applyButton = screen.getAllByText('▶️ Apply')[0]
      fireEvent.click(applyButton)

      // Check that both pins were set (order may vary)
      expect(mockOnPinClick).toHaveBeenCalledWith('START_4', 'set', 'LOW')
      expect(mockOnPinClick).toHaveBeenCalledWith('RESET_4', 'set', 'LOW')

      // Advance timers to verify button state changes
      await vi.advanceTimersByTimeAsync(100)
      expect(screen.getByText('✅ Applied')).toBeInTheDocument()

      await vi.advanceTimersByTimeAsync(2000)
      expect(applyButton).toHaveTextContent('▶️ Apply')

      vi.useRealTimers()
    })
  })

  describe('Batch Operations', () => {
    it('renders batch operations tab content', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )
      fireEvent.click(screen.getByText('⚡ Batch Ops'))
      expect(screen.getByText('Power Cycle Test')).toBeInTheDocument()
      expect(screen.getByText('Signal Sweep Test')).toBeInTheDocument()
    })

    it('executes batch operation steps in sequence', async () => {
      vi.useFakeTimers()
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
          onBatchOperation={mockOnBatchOperation}
        />
      )
      
      fireEvent.click(screen.getByText('⚡ Batch Ops'))
      fireEvent.click(screen.getAllByText('▶️ Execute')[0])

      // First step
      expect(mockOnPinClick).toHaveBeenCalledWith('START_4', 'set', 'LOW')
      
      // Advance timers for the operation steps
      for (let i = 0; i < 8; i++) {
        await vi.advanceTimersByTimeAsync(1100) // More than the step delay
      }

      // Verify all steps were executed
      expect(mockOnPinClick).toHaveBeenCalledTimes(6) // Number of pin operations in power cycle test
      expect(mockFetch).toHaveBeenCalledWith('/api/ping', { method: 'POST' })
      expect(mockOnBatchOperation).toHaveBeenCalled()

      vi.useRealTimers()
    })

    it('shows progress during batch operation', async () => {
      vi.useFakeTimers()
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      fireEvent.click(screen.getByText('⚡ Batch Ops'))
      fireEvent.click(screen.getAllByText('▶️ Execute')[0])

      expect(screen.getByText('Executing batch operation...')).toBeInTheDocument()
      expect(screen.getByText('1/9')).toBeInTheDocument() // First step of power cycle test

      await vi.advanceTimersByTimeAsync(5000)
      expect(screen.queryByText('Executing batch operation...')).not.toBeInTheDocument()

      vi.useRealTimers()
    })
  })

  describe('Custom Presets', () => {
    it('creates custom preset from current state', async () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      fireEvent.click(screen.getByText('🛠️ Custom'))
      
      const nameInput = screen.getByPlaceholderText('Enter preset name...')
      const descInput = screen.getByPlaceholderText('Describe what this preset does...')
      
      await userEvent.type(nameInput, 'Test Preset')
      await userEvent.type(descInput, 'Test Description')
      
      fireEvent.click(screen.getByText('💾 Save as Preset'))

      expect(mockLocalStorage.setItem).toHaveBeenCalled()
      const savedData = JSON.parse(mockLocalStorage.setItem.mock.calls[0][1])
      expect(savedData[0].name).toBe('Test Preset')
      expect(savedData[0].description).toBe('Test Description')
    })

    it('validates custom preset form', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      fireEvent.click(screen.getByText('🛠️ Custom'))
      expect(screen.getByText('💾 Save as Preset')).toBeDisabled()

      const nameInput = screen.getByPlaceholderText('Enter preset name...')
      fireEvent.change(nameInput, { target: { value: 'Test' } })
      expect(screen.getByText('💾 Save as Preset')).toBeEnabled()
    })

    it('allows deletion of custom presets', () => {
      const savedPresets = [{
        id: 'custom-123',
        name: 'Custom Preset',
        description: 'Test preset',
        pinStates: { 'START_4': 'HIGH' },
        category: 'custom'
      }]
      mockLocalStorage.getItem.mockReturnValue(JSON.stringify(savedPresets))

      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      const deleteButton = screen.getByTitle('Delete preset')
      fireEvent.click(deleteButton)

      expect(mockLocalStorage.setItem).toHaveBeenCalledWith(
        'multi-sonicator-presets',
        '[]'
      )
    })
  })

  describe('Power Control', () => {
    it('updates power level via slider', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      const slider = screen.getByRole('slider')
      fireEvent.change(slider, { target: { value: '75' } })

      expect(mockOnPinClick).toHaveBeenCalledWith('POWER_SENSE_4', '768') // 75% of 1023
    })

    it('uses power level preset buttons', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      fireEvent.click(screen.getByText('75%'))
      expect(mockOnPinClick).toHaveBeenCalledWith('POWER_SENSE_4', '768')

      fireEvent.click(screen.getByText('OFF'))
      expect(mockOnPinClick).toHaveBeenCalledWith('POWER_SENSE_4', '0')
    })

    it('displays correct power status text', () => {
      const states = [
        { power: 0, status: 'OFF' },
        { power: 200, status: 'LOW' }, // ~20%
        { power: 450, status: 'MEDIUM' }, // ~44%
        { power: 700, status: 'HIGH' }, // ~68%
        { power: 900, status: 'MAXIMUM' } // ~88%
      ]

      states.forEach(({ power, status }) => {
        const hardwareState = {
          pins: {
            ...mockHardwareState.pins,
            'POWER_SENSE_4': { state: power }
          }
        }

        const { rerender } = render(
          <AdvancedControlPanel
            hardwareState={hardwareState}
            onPinClick={mockOnPinClick}
          />
        )

        expect(screen.getByText(status)).toBeInTheDocument()
        rerender(<></>)
      })
    })
  })

  describe('Accessibility', () => {
    it('has labeled controls', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )
      
      expect(screen.getByLabelText('Power Level:')).toBeInTheDocument()
    })

    it('supports keyboard navigation', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      const firstTab = screen.getByText('📋 Presets')
      const slider = screen.getByRole('slider')

      firstTab.focus()
      expect(document.activeElement).toBe(firstTab)

      userEvent.tab()
      expect(document.activeElement?.textContent).toContain('⚡ ⚡ Batch Ops')

      userEvent.tab()
      expect(document.activeElement?.textContent).toContain('🛠️ 🛠️ Custom')
    })

    it('has appropriate ARIA attributes', () => {
      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      const slider = screen.getByRole('slider')
      // HTML range input attributes
      expect(slider).toHaveAttribute('min', '0')
      expect(slider).toHaveAttribute('max', '100')
      expect(slider).toHaveAttribute('value', '50')
    })
  })

  describe('Error Handling', () => {
    it('handles localStorage errors gracefully', () => {
      const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {})
      mockLocalStorage.getItem.mockImplementation(() => { throw new Error('Storage error') })

      // The component should render despite localStorage errors
      expect(() => {
        render(
          <AdvancedControlPanel
            hardwareState={mockHardwareState}
            onPinClick={mockOnPinClick}
          />
        )
      }).not.toThrow()

      expect(consoleSpy).toHaveBeenCalledWith('Failed to load saved presets:', expect.any(Error))
      consoleSpy.mockRestore()
    })

    it('handles batch operation errors', async () => {
      const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {})
      mockFetch.mockRejectedValueOnce(new Error('Network error'))

      render(
        <AdvancedControlPanel
          hardwareState={mockHardwareState}
          onPinClick={mockOnPinClick}
        />
      )

      fireEvent.click(screen.getByText('⚡ Batch Ops'))
      fireEvent.click(screen.getAllByText('▶️ Execute')[0])

      await waitFor(() => {
        expect(consoleSpy).toHaveBeenCalledWith('Batch operation failed:', expect.any(Error))
      })

      consoleSpy.mockRestore()
    })
  })
})
