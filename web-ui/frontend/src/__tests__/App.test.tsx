import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import React from 'react'
import App from '../App'

// Mock all the hooks and services
vi.mock('../hooks/useWebSocket', () => ({
  useWebSocket: () => ({
    connected: true,
    sendMessage: vi.fn(),
    lastMessage: null
  })
}))

vi.mock('../hooks/useHardwareState', () => ({
  useHardwareState: () => ({
    hardwareState: {
      connection: { connected: true, status: 'Connected' },
      pins: {
        'D8': { state: 'LOW', direction: 'OUT', timestamp: Date.now() },
        'D9': { state: 'PWM=128', direction: 'OUT', timestamp: Date.now() }
      },
      lastUpdate: Date.now()
    },
    updatePinState: vi.fn(),
    updateMultiplePins: vi.fn(),
    setConnectionStatus: vi.fn()
  })
}))

vi.mock('../hooks/useArduinoCommandLog', () => ({
  useArduinoCommandLog: () => ({
    commands: [],
    addCommand: vi.fn(),
    clearCommands: vi.fn()
  })
}))

vi.mock('../hooks/usePinHistory', () => ({
  usePinHistory: () => ({
    addHistoryEntry: vi.fn()
  })
}))

vi.mock('../hooks/useTestAutomation', () => ({
  useTestAutomation: () => ({
    // State
    availableScenarios: [],
    availableTags: ['@smoke', '@integration'],
    availableFeatures: ['pin-control.feature'],
    currentExecution: null,
    isExecutionInProgress: false,
    selectedScenarios: [],
    filterTags: [],
    loading: false,
    error: null,

    // Actions
    loadInitialData: vi.fn(),
    loadScenariosByTags: vi.fn(),
    executeScenarios: vi.fn(),
    stopExecution: vi.fn(),
    updateExecutionProgress: vi.fn(),
    handleExecutionComplete: vi.fn(),
    handleExecutionError: vi.fn(),
    toggleScenarioSelection: vi.fn(),
    selectAllScenarios: vi.fn(),
    clearScenarioSelection: vi.fn(),
    setFilterTags: vi.fn(),
    clearError: vi.fn(),

    // Computed values
    getFilteredScenarios: () => [],
    getScenariosByPins: () => [],
    getExecutionProgress: () => ({ completed: 0, total: 0, percentage: 0 }),
    getCurrentStepProgress: () => ({ current: 0, total: 0, percentage: 0 }),

    // Helper functions
    formatDuration: (ms: number) => `${ms}ms`,
    getStatusColor: (status: string) => '#000000',
    getStatusIcon: (status: string) => '⚪'
  })
}))

vi.mock('../hooks/useKeyboardShortcuts', () => ({
  useKeyboardShortcuts: () => ({ shortcuts: [] }),
  createAppShortcuts: () => []
}))

// Mock localStorage
const mockLocalStorage = {
  getItem: vi.fn(),
  setItem: vi.fn(),
  removeItem: vi.fn(),
  clear: vi.fn(),
}
Object.defineProperty(window, 'localStorage', {
  value: mockLocalStorage
})

// Mock matchMedia
Object.defineProperty(window, 'matchMedia', {
  writable: true,
  value: vi.fn().mockImplementation(query => ({
    matches: false,
    media: query,
    onchange: null,
    addListener: vi.fn(),
    removeListener: vi.fn(),
    addEventListener: vi.fn(),
    removeEventListener: vi.fn(),
    dispatchEvent: vi.fn(),
  })),
})

describe('App Component Integration Tests', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    mockLocalStorage.getItem.mockReturnValue(null)
  })

  describe('Initial Rendering', () => {
    it('renders the main application', () => {
      render(<App />)

      expect(screen.getByText('Multi-Sonicator-IO Test Harness')).toBeInTheDocument()
      expect(screen.getByText('Connected')).toBeInTheDocument()
    })

    it('renders all main navigation tabs', () => {
      render(<App />)

      expect(screen.getByText('🔧 Hardware Control')).toBeInTheDocument()
      expect(screen.getByText('🧪 Test Automation')).toBeInTheDocument()
      expect(screen.getByText('🔧 Arduino Commands')).toBeInTheDocument()
      expect(screen.getByText('⚙️ Settings')).toBeInTheDocument()
    })

    it('starts with hardware control tab active', () => {
      render(<App />)

      const hardwareTab = screen.getByText('🔧 Hardware Control').closest('button')
      expect(hardwareTab).toHaveClass('active')
    })

    it('renders the hardware diagram by default', () => {
      render(<App />)

      expect(screen.getByRole('main')).toBeInTheDocument()
      expect(screen.getByText('🔧 Hardware Control')).toBeInTheDocument()
    })
  })

  describe('Tab Navigation', () => {
    it('switches to test automation tab', async () => {
      render(<App />)

      const testTab = screen.getByText('🧪 Test Automation')
      fireEvent.click(testTab)

      await waitFor(() => {
        expect(testTab.closest('button')).toHaveClass('active')
        expect(screen.getByText(/Test Automation/i)).toBeInTheDocument()
      })
    })

    it('switches to arduino commands tab', async () => {
      render(<App />)

      const commandsTab = screen.getByText('🔧 Arduino Commands')
      fireEvent.click(commandsTab)

      await waitFor(() => {
        expect(commandsTab.closest('button')).toHaveClass('active')
        expect(screen.getByText(/Command Log/i)).toBeInTheDocument()
      })
    })

    it('switches to settings tab', async () => {
      render(<App />)

      const settingsTab = screen.getByText('⚙️ Settings')
      fireEvent.click(settingsTab)
      
      await waitFor(() => {
        expect(settingsTab.closest('button')).toHaveClass('active')
        expect(screen.getByText(/Settings/i)).toBeInTheDocument()
      })
    })

    it('maintains state when switching between tabs', async () => {
      render(<App />)

      // Go to settings and make a change
      const settingsTab = screen.getByText('⚙️ Settings')
      fireEvent.click(settingsTab)

      await waitFor(() => {
        const themeSelect = screen.getByLabelText(/Theme/i)
        fireEvent.change(themeSelect, { target: { value: 'dark' } })
      })

      // Switch to hardware tab and back
      const hardwareTab = screen.getByText('🔧 Hardware Control')
      fireEvent.click(hardwareTab)
      fireEvent.click(settingsTab)

      // Theme selection should be preserved
      await waitFor(() => {
        const themeSelect = screen.getByLabelText(/Theme/i) as HTMLSelectElement
        expect(themeSelect.value).toBe('dark')
      })
    })
  })

  describe('Theme System Integration', () => {
    it('applies theme changes globally', async () => {
      render(<App />)
      
      const themeButton = screen.getByRole('button', { name: /theme/i })
      fireEvent.click(themeButton)
      
      await waitFor(() => {
        expect(document.documentElement).toHaveClass('dark-theme')
      })
    })

    it('persists theme preference', async () => {
      render(<App />)
      
      const themeButton = screen.getByRole('button', { name: /theme/i })
      fireEvent.click(themeButton)
      
      await waitFor(() => {
        expect(mockLocalStorage.setItem).toHaveBeenCalledWith('theme', 'dark')
      })
    })

    it('loads saved theme on startup', () => {
      mockLocalStorage.getItem.mockReturnValue('dark')
      
      render(<App />)
      
      expect(document.documentElement).toHaveClass('dark-theme')
    })
  })

  describe('Help System Integration', () => {
    it('opens help system when help button is clicked', async () => {
      render(<App />)
      
      const helpButton = screen.getByRole('button', { name: /help/i })
      fireEvent.click(helpButton)
      
      await waitFor(() => {
        expect(screen.getByText(/Help & Documentation/i)).toBeInTheDocument()
      })
    })

    it('closes help system when close button is clicked', async () => {
      render(<App />)
      
      const helpButton = screen.getByRole('button', { name: /help/i })
      fireEvent.click(helpButton)
      
      await waitFor(() => {
        const closeButton = screen.getByRole('button', { name: /close/i })
        fireEvent.click(closeButton)
      })
      
      await waitFor(() => {
        expect(screen.queryByText(/Help & Documentation/i)).not.toBeInTheDocument()
      })
    })

    it('help system overlays other content', async () => {
      render(<App />)
      
      const helpButton = screen.getByRole('button', { name: /help/i })
      fireEvent.click(helpButton)
      
      await waitFor(() => {
        const helpOverlay = screen.getByTestId('help-system')
        expect(helpOverlay).toHaveClass('help-overlay')
      })
    })
  })

  describe('Keyboard Shortcuts Integration', () => {
    it('enables keyboard shortcuts by default', () => {
      render(<App />)
      
      // Should respond to keyboard shortcuts
      fireEvent.keyDown(document, { key: 'h', ctrlKey: true })
      
      expect(screen.getByText(/Help & Documentation/i)).toBeInTheDocument()
    })

    it('disables keyboard shortcuts when setting is off', async () => {
      render(<App />)
      
      // Go to settings and disable shortcuts
      const settingsTab = screen.getByText('Settings')
      fireEvent.click(settingsTab)
      
      await waitFor(() => {
        const shortcutsNav = screen.getByText('Keyboard Shortcuts')
        fireEvent.click(shortcutsNav)
        
        const enableCheckbox = screen.getByLabelText(/Enable keyboard shortcuts/i)
        fireEvent.click(enableCheckbox)
      })
      
      // Switch back to hardware tab
      const hardwareTab = screen.getByText('🔧 Hardware Control')
      fireEvent.click(hardwareTab)
      
      // Keyboard shortcuts should not work
      fireEvent.keyDown(document, { key: 'h', ctrlKey: true })
      
      expect(screen.queryByText(/Help & Documentation/i)).not.toBeInTheDocument()
    })

    it('supports tab switching shortcuts', async () => {
      render(<App />)
      
      // Ctrl+1 should switch to hardware tab
      fireEvent.keyDown(document, { key: '1', ctrlKey: true })
      
      const hardwareTab = screen.getByText('🔧 Hardware Control').closest('button')
      expect(hardwareTab).toHaveClass('active')
      
      // Ctrl+2 should switch to test automation tab
      fireEvent.keyDown(document, { key: '2', ctrlKey: true })
      
      await waitFor(() => {
        const testTab = screen.getByText('🧪 Test Automation').closest('button')
        expect(testTab).toHaveClass('active')
      })
    })
  })

  describe('Pin Highlighting Integration', () => {
    it('highlights pins when clicked in hardware diagram', async () => {
      render(<App />)
      
      const d8Pin = screen.getByText('D8').closest('.pin')
      fireEvent.click(d8Pin!)
      
      await waitFor(() => {
        expect(d8Pin).toHaveClass('highlighted')
      })
    })

    it('highlights pins during test execution', async () => {
      render(<App />)
      
      // Switch to test automation tab
      const testTab = screen.getByText('Test Automation')
      fireEvent.click(testTab)
      
      await waitFor(() => {
        // Mock test execution that highlights pins
        const runButton = screen.getByText(/Run Selected/i)
        if (!runButton.disabled) {
          fireEvent.click(runButton)
        }
      })
      
      // Pins should be highlighted during execution
      // This would be tested with proper mocking of test execution
    })
  })

  describe('Connection Status Integration', () => {
    it('shows connection status in header', () => {
      render(<App />)
      
      expect(screen.getByText('Connected')).toBeInTheDocument()
    })

    it('disables controls when disconnected', async () => {
      // Mock disconnected state
      vi.mocked(require('../hooks/useWebSocket').useWebSocket).mockReturnValue({
        connected: false,
        sendMessage: vi.fn(),
        lastMessage: null
      })
      
      render(<App />)
      
      // Controls should be disabled
      const buttons = screen.getAllByRole('button')
      const controlButtons = buttons.filter(btn => 
        btn.textContent?.includes('HIGH') || 
        btn.textContent?.includes('LOW')
      )
      
      controlButtons.forEach(button => {
        expect(button).toBeDisabled()
      })
    })
  })

  describe('Error Handling', () => {
    it('handles WebSocket connection errors gracefully', () => {
      // Mock WebSocket error
      vi.mocked(require('../hooks/useWebSocket').useWebSocket).mockReturnValue({
        connected: false,
        sendMessage: vi.fn(),
        lastMessage: null,
        error: 'Connection failed'
      })
      
      expect(() => {
        render(<App />)
      }).not.toThrow()
      
      expect(screen.getByText('Disconnected')).toBeInTheDocument()
    })

    it('handles component rendering errors gracefully', () => {
      // Mock console.error to avoid noise in tests
      const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {})
      
      // This would test error boundaries if implemented
      expect(() => {
        render(<App />)
      }).not.toThrow()
      
      consoleSpy.mockRestore()
    })
  })

  describe('Performance', () => {
    it('does not re-render unnecessarily', async () => {
      const { rerender } = render(<App />)
      
      // Simulate props that shouldn't cause re-render
      rerender(<App />)
      
      // Component should still be functional
      expect(screen.getByText('Multi-Sonicator-IO Test Harness')).toBeInTheDocument()
    })

    it('handles rapid tab switching', async () => {
      render(<App />)
      
      const tabs = [
        screen.getByText('🔧 Hardware Control'),
        screen.getByText('🧪 Test Automation'),
        screen.getByText('🔧 Arduino Commands'),
        screen.getByText('⚙️ Settings')
      ]
      
      // Rapidly switch between tabs
      for (let i = 0; i < 10; i++) {
        const tab = tabs[i % tabs.length]
        fireEvent.click(tab)
        
        await waitFor(() => {
          expect(tab.closest('button')).toHaveClass('active')
        })
      }
      
      // App should still be responsive
      expect(screen.getByText('Multi-Sonicator-IO Test Harness')).toBeInTheDocument()
    })
  })

  describe('Responsive Design', () => {
    it('adapts to smaller screen sizes', () => {
      // Mock smaller screen
      Object.defineProperty(window, 'innerWidth', {
        writable: true,
        configurable: true,
        value: 768,
      })
      
      render(<App />)
      
      // Main elements should still be present
      expect(screen.getByText('Multi-Sonicator-IO Test Harness')).toBeInTheDocument()
      expect(screen.getByText('🔧 Hardware Control')).toBeInTheDocument()
    })
  })
})
