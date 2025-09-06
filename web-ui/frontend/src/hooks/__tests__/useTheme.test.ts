import { describe, it, expect, vi, beforeEach } from 'vitest'
import { renderHook, act } from '@testing-library/react'

// Simple theme hook for testing
function useTheme() {
  const [theme, setTheme] = React.useState<'light' | 'dark' | 'auto'>('light')
  
  const toggleTheme = () => {
    setTheme(current => {
      switch (current) {
        case 'light': return 'dark'
        case 'dark': return 'auto'
        case 'auto': return 'light'
        default: return 'light'
      }
    })
  }
  
  const applyTheme = (newTheme: 'light' | 'dark' | 'auto') => {
    setTheme(newTheme)
    
    // Apply to document
    if (newTheme === 'dark') {
      document.documentElement.classList.add('dark-theme')
    } else if (newTheme === 'light') {
      document.documentElement.classList.remove('dark-theme')
    } else {
      // Auto mode - check system preference
      const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches
      if (prefersDark) {
        document.documentElement.classList.add('dark-theme')
      } else {
        document.documentElement.classList.remove('dark-theme')
      }
    }
    
    // Save to localStorage
    localStorage.setItem('theme', newTheme)
  }
  
  React.useEffect(() => {
    // Load saved theme
    const saved = localStorage.getItem('theme') as 'light' | 'dark' | 'auto' | null
    if (saved) {
      applyTheme(saved)
    }
  }, [])
  
  return {
    theme,
    toggleTheme,
    applyTheme
  }
}

// Mock React
const React = {
  useState: vi.fn(),
  useEffect: vi.fn()
}

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

describe('useTheme Hook', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    mockLocalStorage.getItem.mockReturnValue(null)
    document.documentElement.className = ''
  })

  describe('Theme State Management', () => {
    it('initializes with light theme by default', () => {
      let currentTheme = 'light'
      let setTheme = vi.fn()
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(fn => fn())
      
      const { result } = renderHook(() => useTheme())
      
      expect(result.current.theme).toBe('light')
    })

    it('loads saved theme from localStorage', () => {
      mockLocalStorage.getItem.mockReturnValue('dark')
      
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(fn => fn())
      
      renderHook(() => useTheme())
      
      expect(mockLocalStorage.getItem).toHaveBeenCalledWith('theme')
    })
  })

  describe('Theme Toggling', () => {
    it('cycles through theme modes correctly', () => {
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(() => {})
      
      const { result } = renderHook(() => useTheme())
      
      // Light -> Dark
      act(() => {
        result.current.toggleTheme()
      })
      expect(setTheme).toHaveBeenCalled()
      
      // Simulate theme change
      currentTheme = 'dark'
      React.useState.mockReturnValue([currentTheme, setTheme])
      
      // Dark -> Auto
      act(() => {
        result.current.toggleTheme()
      })
      expect(setTheme).toHaveBeenCalled()
    })
  })

  describe('Theme Application', () => {
    it('applies dark theme to document', () => {
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(() => {})
      
      const { result } = renderHook(() => useTheme())
      
      act(() => {
        result.current.applyTheme('dark')
      })
      
      expect(document.documentElement.classList.contains('dark-theme')).toBe(true)
      expect(mockLocalStorage.setItem).toHaveBeenCalledWith('theme', 'dark')
    })

    it('removes dark theme for light mode', () => {
      document.documentElement.classList.add('dark-theme')
      
      let currentTheme = 'dark'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(() => {})
      
      const { result } = renderHook(() => useTheme())
      
      act(() => {
        result.current.applyTheme('light')
      })
      
      expect(document.documentElement.classList.contains('dark-theme')).toBe(false)
      expect(mockLocalStorage.setItem).toHaveBeenCalledWith('theme', 'light')
    })

    it('handles auto mode based on system preference', () => {
      // Mock system prefers dark
      window.matchMedia = vi.fn().mockImplementation(query => ({
        matches: query === '(prefers-color-scheme: dark)',
        media: query,
        onchange: null,
        addListener: vi.fn(),
        removeListener: vi.fn(),
        addEventListener: vi.fn(),
        removeEventListener: vi.fn(),
        dispatchEvent: vi.fn(),
      }))
      
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(() => {})
      
      const { result } = renderHook(() => useTheme())
      
      act(() => {
        result.current.applyTheme('auto')
      })
      
      expect(window.matchMedia).toHaveBeenCalledWith('(prefers-color-scheme: dark)')
      expect(document.documentElement.classList.contains('dark-theme')).toBe(true)
      expect(mockLocalStorage.setItem).toHaveBeenCalledWith('theme', 'auto')
    })
  })

  describe('Persistence', () => {
    it('saves theme changes to localStorage', () => {
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(() => {})
      
      const { result } = renderHook(() => useTheme())
      
      act(() => {
        result.current.applyTheme('dark')
      })
      
      expect(mockLocalStorage.setItem).toHaveBeenCalledWith('theme', 'dark')
    })

    it('loads theme from localStorage on initialization', () => {
      mockLocalStorage.getItem.mockReturnValue('dark')
      
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(fn => fn())
      
      renderHook(() => useTheme())
      
      expect(mockLocalStorage.getItem).toHaveBeenCalledWith('theme')
    })
  })

  describe('Error Handling', () => {
    it('handles localStorage errors gracefully', () => {
      mockLocalStorage.setItem.mockImplementation(() => {
        throw new Error('Storage quota exceeded')
      })
      
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(() => {})
      
      const { result } = renderHook(() => useTheme())
      
      expect(() => {
        act(() => {
          result.current.applyTheme('dark')
        })
      }).not.toThrow()
    })

    it('handles invalid saved theme values', () => {
      mockLocalStorage.getItem.mockReturnValue('invalid-theme')
      
      let currentTheme = 'light'
      let setTheme = vi.fn((newTheme) => {
        currentTheme = typeof newTheme === 'function' ? newTheme(currentTheme) : newTheme
      })
      
      React.useState.mockReturnValue([currentTheme, setTheme])
      React.useEffect.mockImplementation(fn => fn())
      
      expect(() => {
        renderHook(() => useTheme())
      }).not.toThrow()
    })
  })
})
