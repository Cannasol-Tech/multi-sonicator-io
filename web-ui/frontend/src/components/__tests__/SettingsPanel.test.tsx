import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import React from 'react'
import SettingsPanel from '../SettingsPanel'

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

describe('SettingsPanel Component', () => {
  const mockProps = {
    onPreferencesChange: vi.fn(),
  }

  beforeEach(() => {
    vi.clearAllMocks()
    mockLocalStorage.getItem.mockReturnValue(null)
  })

  describe('Rendering', () => {
    it('renders the settings panel', () => {
      render(<SettingsPanel {...mockProps} />)
      
      expect(screen.getByText(/Settings/i)).toBeInTheDocument()
    })

    it('renders all settings sections', () => {
      render(<SettingsPanel {...mockProps} />)

      expect(screen.getByText(/🎨 Appearance/i)).toBeInTheDocument()
      expect(screen.getByText(/⚡ Behavior/i)).toBeInTheDocument()
      expect(screen.getByText(/📊 Data/i)).toBeInTheDocument()
      expect(screen.getByText(/⌨️ Shortcuts/i)).toBeInTheDocument()
    })

    it('renders navigation menu', () => {
      render(<SettingsPanel {...mockProps} />)

      const navItems = screen.getAllByRole('button')
      const settingsNavItems = navItems.filter(item =>
        item.textContent?.includes('🎨 Appearance') ||
        item.textContent?.includes('⚡ Behavior') ||
        item.textContent?.includes('📊 Data') ||
        item.textContent?.includes('⌨️ Shortcuts')
      )

      expect(settingsNavItems.length).toBeGreaterThan(0)
    })
  })

  describe('Navigation', () => {
    it('switches between settings sections', () => {
      render(<SettingsPanel {...mockProps} />)

      const behaviorNav = screen.getByText('⚡ Behavior')
      fireEvent.click(behaviorNav)

      expect(behaviorNav.closest('button')).toHaveClass('active')
    })

    it('shows correct content for each section', () => {
      render(<SettingsPanel {...mockProps} />)

      // Test Appearance section (default)
      expect(screen.getByText(/Animations/i)).toBeInTheDocument()

      // Switch to Behavior section
      const behaviorNav = screen.getByText('⚡ Behavior')
      fireEvent.click(behaviorNav)

      expect(screen.getByText(/Auto Refresh/i)).toBeInTheDocument()
    })
  })

  describe('Appearance Settings', () => {
    it('renders animations toggle', () => {
      render(<SettingsPanel {...mockProps} />)

      expect(screen.getByLabelText(/Animations/i)).toBeInTheDocument()
    })

    it('renders compact mode toggle', () => {
      render(<SettingsPanel {...mockProps} />)

      expect(screen.getByLabelText(/Compact Mode/i)).toBeInTheDocument()
    })

    it('saves animation preference', async () => {
      render(<SettingsPanel {...mockProps} />)

      const animationsToggle = screen.getByLabelText(/Animations/i)
      fireEvent.click(animationsToggle)

      await waitFor(() => {
        expect(mockLocalStorage.setItem).toHaveBeenCalledWith('theme', 'dark')
      })
    })

    it('loads saved preferences', () => {
      mockLocalStorage.getItem.mockImplementation((key) => {
        if (key === 'theme') return 'dark'
        if (key === 'autoSave') return 'true'
        return null
      })
      
      render(<SettingsPanel {...mockProps} />)
      
      const themeSelect = screen.getByLabelText(/Theme/i) as HTMLSelectElement
      expect(themeSelect.value).toBe('dark')
    })
  })

  describe('Connection Settings', () => {
    it('renders WebSocket URL input', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const connectionNav = screen.getByText('Connection')
      fireEvent.click(connectionNav)
      
      expect(screen.getByLabelText(/WebSocket URL/i)).toBeInTheDocument()
    })

    it('renders connection timeout setting', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const connectionNav = screen.getByText('Connection')
      fireEvent.click(connectionNav)
      
      expect(screen.getByLabelText(/Timeout/i)).toBeInTheDocument()
    })

    it('renders auto-reconnect option', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const connectionNav = screen.getByText('Connection')
      fireEvent.click(connectionNav)
      
      expect(screen.getByLabelText(/Auto-reconnect/i)).toBeInTheDocument()
    })

    it('validates WebSocket URL format', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const connectionNav = screen.getByText('Connection')
      fireEvent.click(connectionNav)
      
      const urlInput = screen.getByLabelText(/WebSocket URL/i)
      fireEvent.change(urlInput, { target: { value: 'invalid-url' } })
      fireEvent.blur(urlInput)
      
      await waitFor(() => {
        expect(screen.getByText(/Invalid URL format/i)).toBeInTheDocument()
      })
    })

    it('saves connection settings', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const connectionNav = screen.getByText('Connection')
      fireEvent.click(connectionNav)
      
      const urlInput = screen.getByLabelText(/WebSocket URL/i)
      fireEvent.change(urlInput, { target: { value: 'ws://localhost:8080' } })
      
      const saveButton = screen.getByText(/Save/i)
      fireEvent.click(saveButton)
      
      await waitFor(() => {
        expect(mockLocalStorage.setItem).toHaveBeenCalledWith(
          'connectionSettings',
          expect.stringContaining('ws://localhost:8080')
        )
      })
    })
  })

  describe('Test Automation Settings', () => {
    it('renders test execution settings', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const testNav = screen.getByText('Test Automation')
      fireEvent.click(testNav)
      
      expect(screen.getByLabelText(/Default timeout/i)).toBeInTheDocument()
    })

    it('renders parallel execution option', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const testNav = screen.getByText('Test Automation')
      fireEvent.click(testNav)
      
      expect(screen.getByLabelText(/Parallel execution/i)).toBeInTheDocument()
    })

    it('renders result retention setting', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const testNav = screen.getByText('Test Automation')
      fireEvent.click(testNav)
      
      expect(screen.getByLabelText(/Keep results/i)).toBeInTheDocument()
    })

    it('validates timeout values', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const testNav = screen.getByText('Test Automation')
      fireEvent.click(testNav)
      
      const timeoutInput = screen.getByLabelText(/Default timeout/i)
      fireEvent.change(timeoutInput, { target: { value: '-1' } })
      fireEvent.blur(timeoutInput)
      
      await waitFor(() => {
        expect(screen.getByText(/Timeout must be positive/i)).toBeInTheDocument()
      })
    })
  })

  describe('Keyboard Shortcuts Settings', () => {
    it('renders keyboard shortcuts section', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const shortcutsNav = screen.getByText('Keyboard Shortcuts')
      fireEvent.click(shortcutsNav)
      
      expect(screen.getByText(/Enable keyboard shortcuts/i)).toBeInTheDocument()
    })

    it('displays available shortcuts', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const shortcutsNav = screen.getByText('Keyboard Shortcuts')
      fireEvent.click(shortcutsNav)
      
      expect(screen.getByText(/Ctrl\+R/i)).toBeInTheDocument() // Run tests
      expect(screen.getByText(/Ctrl\+S/i)).toBeInTheDocument() // Save settings
    })

    it('toggles keyboard shortcuts', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const shortcutsNav = screen.getByText('Keyboard Shortcuts')
      fireEvent.click(shortcutsNav)
      
      const enableCheckbox = screen.getByLabelText(/Enable keyboard shortcuts/i)
      fireEvent.click(enableCheckbox)
      
      await waitFor(() => {
        expect(mockProps.onPreferencesChange).toHaveBeenCalledWith(
          expect.objectContaining({
            keyboardShortcuts: expect.any(Boolean)
          })
        )
      })
    })
  })

  describe('Save and Reset Functionality', () => {
    it('shows save button when settings are modified', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const themeSelect = screen.getByLabelText(/Theme/i)
      fireEvent.change(themeSelect, { target: { value: 'dark' } })
      
      await waitFor(() => {
        const saveButton = screen.getByText(/Save/i)
        expect(saveButton).not.toBeDisabled()
      })
    })

    it('saves all settings when save button is clicked', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const themeSelect = screen.getByLabelText(/Theme/i)
      fireEvent.change(themeSelect, { target: { value: 'dark' } })
      
      const saveButton = screen.getByText(/Save/i)
      fireEvent.click(saveButton)
      
      await waitFor(() => {
        expect(mockLocalStorage.setItem).toHaveBeenCalled()
        expect(mockProps.onPreferencesChange).toHaveBeenCalled()
      })
    })

    it('resets settings to defaults', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const resetButton = screen.getByText(/Reset to Defaults/i)
      fireEvent.click(resetButton)
      
      await waitFor(() => {
        const themeSelect = screen.getByLabelText(/Theme/i) as HTMLSelectElement
        expect(themeSelect.value).toBe('light')
      })
    })

    it('shows confirmation dialog before reset', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const resetButton = screen.getByText(/Reset to Defaults/i)
      fireEvent.click(resetButton)
      
      expect(screen.getByText(/Are you sure/i)).toBeInTheDocument()
    })
  })

  describe('Import/Export Settings', () => {
    it('provides export functionality', () => {
      render(<SettingsPanel {...mockProps} />)
      
      expect(screen.getByText(/📤 Export/i)).toBeInTheDocument()
    })

    it('provides import functionality', () => {
      render(<SettingsPanel {...mockProps} />)
      
      expect(screen.getByText(/📥 Import/i)).toBeInTheDocument()
    })

    it('exports settings as JSON', async () => {
      // Mock URL.createObjectURL
      global.URL.createObjectURL = vi.fn(() => 'blob:mock-url')
      
      render(<SettingsPanel {...mockProps} />)
      
      const exportButton = screen.getByText(/📤 Export/i)
      fireEvent.click(exportButton)
      
      expect(global.URL.createObjectURL).toHaveBeenCalled()
    })
  })

  describe('Validation and Error Handling', () => {
    it('shows validation errors for invalid inputs', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const connectionNav = screen.getByText('Connection')
      fireEvent.click(connectionNav)
      
      const timeoutInput = screen.getByLabelText(/Timeout/i)
      fireEvent.change(timeoutInput, { target: { value: 'invalid' } })
      fireEvent.blur(timeoutInput)
      
      await waitFor(() => {
        expect(screen.getByText(/Invalid number/i)).toBeInTheDocument()
      })
    })

    it('prevents saving invalid settings', async () => {
      render(<SettingsPanel {...mockProps} />)
      
      const connectionNav = screen.getByText('Connection')
      fireEvent.click(connectionNav)
      
      const urlInput = screen.getByLabelText(/WebSocket URL/i)
      fireEvent.change(urlInput, { target: { value: 'invalid-url' } })
      
      const saveButton = screen.getByText(/Save/i)
      fireEvent.click(saveButton)
      
      await waitFor(() => {
        expect(mockLocalStorage.setItem).not.toHaveBeenCalled()
      })
    })

    it('handles localStorage errors gracefully', async () => {
      mockLocalStorage.setItem.mockImplementation(() => {
        throw new Error('Storage quota exceeded')
      })
      
      render(<SettingsPanel {...mockProps} />)
      
      const themeSelect = screen.getByLabelText(/Theme/i)
      fireEvent.change(themeSelect, { target: { value: 'dark' } })
      
      const saveButton = screen.getByText(/Save/i)
      
      expect(() => {
        fireEvent.click(saveButton)
      }).not.toThrow()
    })
  })

  describe('Accessibility', () => {
    it('has proper labels for all form controls', () => {
      render(<SettingsPanel {...mockProps} />)
      
      expect(screen.getByLabelText(/Theme/i)).toBeInTheDocument()
      expect(screen.getByLabelText(/Auto-save/i)).toBeInTheDocument()
    })

    it('supports keyboard navigation', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const firstNavItem = screen.getByText('General')
      firstNavItem.focus()
      
      expect(document.activeElement).toBe(firstNavItem)
    })

    it('provides appropriate ARIA attributes', () => {
      render(<SettingsPanel {...mockProps} />)
      
      const activeNavItem = screen.getByText('General').closest('button')
      expect(activeNavItem).toHaveAttribute('aria-selected', 'true')
    })
  })
})
