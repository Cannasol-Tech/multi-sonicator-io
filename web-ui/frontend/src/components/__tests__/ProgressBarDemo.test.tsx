import React from 'react'
import { render, screen, fireEvent, act, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { vi } from 'vitest'
import ProgressBarDemo from '../ProgressBarDemo'

describe('ProgressBarDemo', () => {
  beforeEach(() => {
    vi.useFakeTimers()
  })

  afterEach(() => {
    vi.clearAllTimers()
    vi.useRealTimers()
  })

  describe('Rendering', () => {
    it('renders without crashing', () => {
      const { container } = render(<ProgressBarDemo />)
      expect(container).toBeTruthy()
    })

    it('renders the demo title', () => {
      render(<ProgressBarDemo />)
      expect(screen.getByText('🌈 Iridescent Progress Bar Demo')).toBeInTheDocument()
    })

    it('displays initial progress at 0%', () => {
      render(<ProgressBarDemo />)
      expect(screen.getByText('0%')).toBeInTheDocument()
      expect(screen.getByText('0 / 25 scenarios (0%)')).toBeInTheDocument()
    })

    it('shows initial ready status', () => {
      render(<ProgressBarDemo />)
      expect(screen.getByText('⏸️ Ready to demonstrate the new progress design')).toBeInTheDocument()
    })

    it('applies custom className', () => {
      const { container } = render(<ProgressBarDemo className="custom-demo" />)
      expect(container.querySelector('.progress-demo')).toHaveClass('custom-demo')
    })
  })

  describe('Control Buttons', () => {
    it('renders start and reset buttons', () => {
      render(<ProgressBarDemo />)
      expect(screen.getByText('▶️ Start Test')).toBeInTheDocument()
      expect(screen.getByText('🔄 Reset')).toBeInTheDocument()
    })

    it('enables start button initially', () => {
      render(<ProgressBarDemo />)
      const startButton = screen.getByText('▶️ Start Test')
      expect(startButton).toBeEnabled()
    })

    it('disables start button when running', () => {
      render(<ProgressBarDemo />)
      const startButton = screen.getByText('▶️ Start Test')
      
      fireEvent.click(startButton)
      expect(screen.getByText('🔄 Running...')).toBeInTheDocument()
      expect(screen.getByText('🔄 Running...')).toBeDisabled()
    })

    it('always enables reset button', () => {
      render(<ProgressBarDemo />)
      const resetButton = screen.getByText('🔄 Reset')
      expect(resetButton).toBeEnabled()
      
      // Even when running
      fireEvent.click(screen.getByText('▶️ Start Test'))
      expect(resetButton).toBeEnabled()
    })
  })

  describe('Progress Animation', () => {
    it('starts progress when start button is clicked', async () => {
      render(<ProgressBarDemo />)
      const startButton = screen.getByText('▶️ Start Test')
      
      fireEvent.click(startButton)
      expect(screen.getByText('🔄 Test execution in progress...')).toBeInTheDocument()
      
      // Advance timers to see progress
      act(() => {
        vi.advanceTimersByTime(500) // 5 intervals * 100ms = 10% progress (5 * 2%)
      })
      
      expect(screen.getByText('10%')).toBeInTheDocument()
      expect(screen.getByText('3 / 25 scenarios (10%)')).toBeInTheDocument()
    })

    it('progresses by 2% every 100ms', async () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // Test specific increments
      act(() => {
        vi.advanceTimersByTime(100) // 1 interval = 2%
      })
      expect(screen.getByText('2%')).toBeInTheDocument()
      
      act(() => {
        vi.advanceTimersByTime(400) // 4 more intervals = 8% more = 10% total
      })
      expect(screen.getByText('10%')).toBeInTheDocument()
    })

    it('stops at 100% and shows completion', async () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // Advance to completion (100% = 50 intervals * 100ms)
      act(() => {
        vi.advanceTimersByTime(5000)
      })
      
      expect(screen.getByText('100%')).toBeInTheDocument()
      expect(screen.getByText('⚡ Test execution complete!')).toBeInTheDocument()
      expect(screen.getByText('▶️ Start Test')).toBeInTheDocument() // Button is re-enabled
    })

    it('calculates scenarios correctly based on progress', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      act(() => {
        vi.advanceTimersByTime(2000) // 20 intervals = 40%
      })
      
      // 40% of 25 = 10
      expect(screen.getByText('10 / 25 scenarios (40%)')).toBeInTheDocument()
    })
  })

  describe('Progress Bar Styling', () => {
    it('applies correct CSS classes during different states', () => {
      const { container } = render(<ProgressBarDemo />)
      
      // Initial state
      let progressFill = container.querySelector('.progress-fill')
      expect(progressFill).toHaveClass('progress-fill')
      expect(progressFill).not.toHaveClass('iridescent')
      
      // Running state
      fireEvent.click(screen.getByText('▶️ Start Test'))
      progressFill = container.querySelector('.progress-fill')
      expect(progressFill).toHaveClass('progress-fill', 'iridescent')
      
      // Complete state
      act(() => {
        vi.advanceTimersByTime(5000) // Complete
      })
      progressFill = container.querySelector('.progress-fill')
      expect(progressFill).toHaveClass('progress-fill', 'metallic-blue')
    })

    it('sets correct width based on progress', () => {
      const { container } = render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      act(() => {
        vi.advanceTimersByTime(1500) // 30%
      })
      
      const progressFill = container.querySelector('.progress-fill') as HTMLElement
      expect(progressFill.style.width).toBe('30%')
    })

    it('applies transition styles for smooth animation', () => {
      const { container } = render(<ProgressBarDemo />)
      const progressFill = container.querySelector('.progress-fill') as HTMLElement
      expect(progressFill.style.transition).toBe('width 0.3s ease')
    })
  })

  describe('Step Progress', () => {
    it('does not show step progress when idle', () => {
      render(<ProgressBarDemo />)
      expect(screen.queryByText(/Step \d+ \/ \d+/)).not.toBeInTheDocument()
    })

    it('shows step progress when running', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      expect(screen.getByText('Step 1 / 10')).toBeInTheDocument()
    })

    it('updates step progress correctly', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // At 15% progress, should be on step 2
      act(() => {
        vi.advanceTimersByTime(750) // 15%
      })
      expect(screen.getByText('Step 2 / 10')).toBeInTheDocument()
      
      // At 25% progress, should be on step 3
      act(() => {
        vi.advanceTimersByTime(500) // 25% total
      })
      expect(screen.getByText('Step 3 / 10')).toBeInTheDocument()
    })

    it('calculates step progress width correctly', () => {
      const { container } = render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // At 15% progress (5% into step 2)
      act(() => {
        vi.advanceTimersByTime(750)
      })
      
      const stepProgressFill = container.querySelector('.step-progress-fill') as HTMLElement
      expect(stepProgressFill.style.width).toBe('50%') // 5% of 10% = 50% of step
    })

    it('caps step number at 10', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // At 95% progress, should still show step 10
      act(() => {
        vi.advanceTimersByTime(4750) // 95%
      })
      expect(screen.getByText('Step 10 / 10')).toBeInTheDocument()
    })
  })

  describe('Reset Functionality', () => {
    it('resets progress to 0 when reset button is clicked', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // Let it progress
      act(() => {
        vi.advanceTimersByTime(1000) // 20%
      })
      expect(screen.getByText('20%')).toBeInTheDocument()
      
      // Reset
      fireEvent.click(screen.getByText('🔄 Reset'))
      expect(screen.getByText('0%')).toBeInTheDocument()
      expect(screen.getByText('⏸️ Ready to demonstrate the new progress design')).toBeInTheDocument()
    })

    it('stops the running animation when reset', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // Let it start running
      act(() => {
        vi.advanceTimersByTime(200) // 4%
      })
      expect(screen.getByText('4%')).toBeInTheDocument()
      
      // Reset while running
      fireEvent.click(screen.getByText('🔄 Reset'))
      expect(screen.getByText('0%')).toBeInTheDocument()
      expect(screen.getByText('▶️ Start Test')).toBeInTheDocument() // Button re-enabled
      
      // Verify it doesn't continue progressing
      act(() => {
        vi.advanceTimersByTime(1000)
      })
      expect(screen.getByText('0%')).toBeInTheDocument() // Still at 0
    })

    it('resets from completed state', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // Complete the progress
      act(() => {
        vi.advanceTimersByTime(5000)
      })
      expect(screen.getByText('100%')).toBeInTheDocument()
      expect(screen.getByText('⚡ Test execution complete!')).toBeInTheDocument()
      
      // Reset
      fireEvent.click(screen.getByText('🔄 Reset'))
      expect(screen.getByText('0%')).toBeInTheDocument()
      expect(screen.getByText('⏸️ Ready to demonstrate the new progress design')).toBeInTheDocument()
    })
  })

  describe('Status Messages', () => {
    it('shows correct status for each state', () => {
      render(<ProgressBarDemo />)
      
      // Initial state
      expect(screen.getByText('⏸️ Ready to demonstrate the new progress design')).toBeInTheDocument()
      
      // Running state
      fireEvent.click(screen.getByText('▶️ Start Test'))
      expect(screen.getByText('🔄 Test execution in progress...')).toBeInTheDocument()
      
      // Completed state
      act(() => {
        vi.advanceTimersByTime(5000)
      })
      expect(screen.getByText('⚡ Test execution complete!')).toBeInTheDocument()
    })

    it('applies correct styling to status messages', () => {
      const { container } = render(<ProgressBarDemo />)
      
      // Check initial status styling
      let statusDiv = container.querySelector('[style*="background: #f8f9fa"]')
      expect(statusDiv).toBeInTheDocument()
      
      // Check running status styling
      fireEvent.click(screen.getByText('▶️ Start Test'))
      statusDiv = container.querySelector('[style*="background: #dbeafe"]')
      expect(statusDiv).toBeInTheDocument()
      
      // Check completed status styling
      act(() => {
        vi.advanceTimersByTime(5000)
      })
      statusDiv = container.querySelector('[style*="background: #d4edda"]')
      expect(statusDiv).toBeInTheDocument()
    })
  })

  describe('Edge Cases', () => {
    it('handles rapid start/reset clicks', () => {
      render(<ProgressBarDemo />)
      
      // Rapid clicks
      fireEvent.click(screen.getByText('▶️ Start Test'))
      fireEvent.click(screen.getByText('🔄 Reset'))
      fireEvent.click(screen.getByText('▶️ Start Test'))
      fireEvent.click(screen.getByText('🔄 Reset'))
      
      expect(screen.getByText('0%')).toBeInTheDocument()
      expect(screen.getByText('▶️ Start Test')).toBeInTheDocument()
    })

    it('handles timer cleanup on unmount', () => {
      const { unmount } = render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      // Should not throw error when unmounting with active timer
      expect(() => unmount()).not.toThrow()
    })
  })

  describe('Accessibility', () => {
    it('provides accessible button labels', () => {
      render(<ProgressBarDemo />)
      
      const startButton = screen.getByRole('button', { name: /Start Test/i })
      const resetButton = screen.getByRole('button', { name: /Reset/i })
      
      expect(startButton).toBeInTheDocument()
      expect(resetButton).toBeInTheDocument()
    })

    it('indicates button state changes for screen readers', () => {
      render(<ProgressBarDemo />)
      
      const startButton = screen.getByRole('button', { name: /Start Test/i })
      expect(startButton).toBeEnabled()
      
      fireEvent.click(startButton)
      const runningButton = screen.getByRole('button', { name: /Running.../i })
      expect(runningButton).toBeDisabled()
    })

    it('supports keyboard navigation', async () => {
      const user = userEvent.setup({ advanceTimers: vi.advanceTimersByTime })
      render(<ProgressBarDemo />)
      
      const startButton = screen.getByRole('button', { name: /Start Test/i })
      const resetButton = screen.getByRole('button', { name: /Reset/i })
      
      await user.tab()
      expect(startButton).toHaveFocus()
      
      await user.tab()
      expect(resetButton).toHaveFocus()
    })

    it('provides meaningful progress information', () => {
      render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      act(() => {
        vi.advanceTimersByTime(1000) // 20%
      })
      
      // Progress information should be accessible
      expect(screen.getByText('20%')).toBeInTheDocument()
      expect(screen.getByText('5 / 25 scenarios (20%)')).toBeInTheDocument()
      expect(screen.getByText('Step 3 / 10')).toBeInTheDocument()
    })
  })

  describe('Visual Regression', () => {
    it('matches snapshot for initial state', () => {
      const { container } = render(<ProgressBarDemo />)
      expect(container.firstChild).toMatchSnapshot()
    })

    it('matches snapshot for running state', () => {
      const { container } = render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      act(() => {
        vi.advanceTimersByTime(1500) // 30%
      })
      
      expect(container.firstChild).toMatchSnapshot()
    })

    it('matches snapshot for completed state', () => {
      const { container } = render(<ProgressBarDemo />)
      fireEvent.click(screen.getByText('▶️ Start Test'))
      
      act(() => {
        vi.advanceTimersByTime(5000) // 100%
      })
      
      expect(container.firstChild).toMatchSnapshot()
    })
  })
})
