import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import React from 'react'
import Header from '../Header'

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

// Mock matchMedia for theme detection
Object.defineProperty(window, 'matchMedia', {
  writable: true,
  value: vi.fn().mockImplementation(query => ({
    matches: query === '(prefers-color-scheme: dark)',
    media: query,
    onchange: null,
    addListener: vi.fn(),
    removeListener: vi.fn(),
    addEventListener: vi.fn(),
    removeEventListener: vi.fn(),
    dispatchEvent: vi.fn(),
  })),
})

describe('Header Component', () => {
  const mockProps = {
    connectionStatus: {
      connected: false,
      port: 'COM3',
      lastSeen: Date.now(),
      error: null
    },
    onShowHelp: vi.fn(),
  }

  beforeEach(() => {
    vi.clearAllMocks()
    mockLocalStorage.getItem.mockReturnValue(null)
  })

  describe('Rendering', () => {
    it('renders the header with title', () => {
      render(<Header {...mockProps} />)

      expect(screen.getByText('Multi-Sonicator-IO Test Harness')).toBeInTheDocument()
      expect(screen.getByText('Arduino Test Wrapper ↔ ATmega32A DUT')).toBeInTheDocument()
    })



    it('renders connection status indicator', () => {
      render(<Header {...mockProps} />)
      
      expect(screen.getByText('Disconnected')).toBeInTheDocument()
    })

    it('renders theme toggle button', () => {
      render(<Header {...mockProps} />)

      const themeButton = screen.getByTitle(/Current theme:/)
      expect(themeButton).toBeInTheDocument()
      expect(themeButton).toHaveClass('theme-toggle')
    })

    it('renders help button', () => {
      render(<Header {...mockProps} />)
      
      const helpButton = screen.getByRole('button', { name: /help/i })
      expect(helpButton).toBeInTheDocument()
    })
  })



  describe('Connection Status', () => {
    it('shows connected status when connected', () => {
      const connectedProps = {
        ...mockProps,
        connectionStatus: {
          ...mockProps.connectionStatus,
          connected: true
        }
      }
      render(<Header {...connectedProps} />)

      expect(screen.getByText('Connected')).toBeInTheDocument()
      expect(screen.queryByText('Disconnected')).not.toBeInTheDocument()
    })

    it('shows disconnected status when not connected', () => {
      render(<Header {...mockProps} connected={false} />)
      
      expect(screen.getByText('Disconnected')).toBeInTheDocument()
      expect(screen.queryByText('Connected')).not.toBeInTheDocument()
    })

    it('displays last update time when provided', () => {
      const lastUpdate = Date.now()
      render(<Header {...mockProps} lastUpdate={lastUpdate} />)
      
      // Should show some form of timestamp
      expect(screen.getByText(/\d{1,2}:\d{2}:\d{2}/)).toBeInTheDocument()
    })

    it('handles null lastUpdate gracefully', () => {
      render(<Header {...mockProps} lastUpdate={null} />)
      
      // Should not crash and should show some default state
      expect(screen.getByText('Disconnected')).toBeInTheDocument()
    })
  })

  describe('Theme Toggle', () => {
    it('initializes with light theme by default', () => {
      render(<Header {...mockProps} />)
      
      expect(document.documentElement).not.toHaveClass('dark-theme')
    })

    it('cycles through theme modes when clicked', async () => {
      render(<Header {...mockProps} />)
      
      const themeButton = screen.getByRole('button', { name: /theme/i })
      
      // Click to go to dark mode
      fireEvent.click(themeButton)
      await waitFor(() => {
        expect(document.documentElement).toHaveClass('dark-theme')
      })
      
      // Click to go to auto mode
      fireEvent.click(themeButton)
      // Auto mode behavior depends on system preference
      
      // Click to go back to light mode
      fireEvent.click(themeButton)
      await waitFor(() => {
        expect(document.documentElement).not.toHaveClass('dark-theme')
      })
    })

    it('saves theme preference to localStorage', async () => {
      render(<Header {...mockProps} />)
      
      const themeButton = screen.getByRole('button', { name: /theme/i })
      fireEvent.click(themeButton)
      
      await waitFor(() => {
        expect(mockLocalStorage.setItem).toHaveBeenCalledWith('theme', 'dark')
      })
    })

    it('loads theme preference from localStorage', () => {
      mockLocalStorage.getItem.mockReturnValue('dark')
      
      render(<Header {...mockProps} />)
      
      expect(document.documentElement).toHaveClass('dark-theme')
    })
  })

  describe('Help System', () => {
    it('calls onHelpToggle when help button is clicked', () => {
      render(<Header {...mockProps} />)
      
      const helpButton = screen.getByRole('button', { name: /help/i })
      fireEvent.click(helpButton)
      
      expect(mockProps.onHelpToggle).toHaveBeenCalled()
    })
  })

  describe('Accessibility', () => {
    it('has proper ARIA labels for buttons', () => {
      render(<Header {...mockProps} />)
      
      expect(screen.getByRole('button', { name: /theme/i })).toBeInTheDocument()
      expect(screen.getByRole('button', { name: /help/i })).toBeInTheDocument()
    })

    it('has proper tab navigation structure', () => {
      render(<Header {...mockProps} />)
      
      const tabs = screen.getAllByRole('button')
      const navigationTabs = tabs.filter(tab => 
        tab.textContent?.includes('Hardware Control') ||
        tab.textContent?.includes('Test Automation') ||
        tab.textContent?.includes('Arduino Commands') ||
        tab.textContent?.includes('Settings')
      )
      
      expect(navigationTabs).toHaveLength(4)
    })
  })

  describe('Responsive Behavior', () => {
    it('maintains functionality on smaller screens', () => {
      // Mock smaller screen size
      Object.defineProperty(window, 'innerWidth', {
        writable: true,
        configurable: true,
        value: 768,
      })
      
      render(<Header {...mockProps} />)
      
      // All main elements should still be present
      expect(screen.getByText('Multi-Sonicator IO')).toBeInTheDocument()
      expect(screen.getByText('Hardware Control')).toBeInTheDocument()
      expect(screen.getByRole('button', { name: /theme/i })).toBeInTheDocument()
    })
  })
})
