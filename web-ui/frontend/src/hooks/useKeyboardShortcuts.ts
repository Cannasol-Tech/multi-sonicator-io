import { useEffect, useCallback } from 'react'

interface KeyboardShortcut {
  key: string
  ctrlKey?: boolean
  altKey?: boolean
  shiftKey?: boolean
  action: () => void
  description: string
}

interface UseKeyboardShortcutsProps {
  shortcuts: KeyboardShortcut[]
  enabled?: boolean
}

export const useKeyboardShortcuts = ({ shortcuts, enabled = true }: UseKeyboardShortcutsProps) => {
  const handleKeyDown = useCallback((event: KeyboardEvent) => {
    if (!enabled) return

    // Don't trigger shortcuts when typing in input fields
    const target = event.target as HTMLElement
    if (target.tagName === 'INPUT' || target.tagName === 'TEXTAREA' || target.contentEditable === 'true') {
      return
    }

    const matchingShortcut = shortcuts.find(shortcut => {
      const keyMatch = shortcut.key.toLowerCase() === event.key.toLowerCase()
      const ctrlMatch = !!shortcut.ctrlKey === event.ctrlKey
      const altMatch = !!shortcut.altKey === event.altKey
      const shiftMatch = !!shortcut.shiftKey === event.shiftKey

      return keyMatch && ctrlMatch && altMatch && shiftMatch
    })

    if (matchingShortcut) {
      event.preventDefault()
      matchingShortcut.action()
    }
  }, [shortcuts, enabled])

  useEffect(() => {
    if (enabled) {
      document.addEventListener('keydown', handleKeyDown)
      return () => document.removeEventListener('keydown', handleKeyDown)
    }
  }, [handleKeyDown, enabled])

  return { shortcuts }
}

// Predefined shortcut configurations
export const createAppShortcuts = (actions: {
  toggleHardwareTab: () => void
  toggleTestingTab: () => void
  toggleAnalyticsTab: () => void // Keep for compatibility but will be disabled
  toggleSettingsTab: () => void
  toggleConfigTab: () => void
  pingHardware: () => void
  exportData: () => void
  refreshConnection: () => void
  closeModals: () => void
}): KeyboardShortcut[] => [
  {
    key: 'h',
    ctrlKey: true,
    action: actions.toggleHardwareTab,
    description: 'Toggle Hardware tab'
  },
  {
    key: 't',
    ctrlKey: true,
    action: actions.toggleTestingTab,
    description: 'Toggle Testing tab'
  },
  {
    key: 'a',
    ctrlKey: true,
    action: actions.toggleAnalyticsTab,
    description: 'Toggle Analytics tab'
  },
  {
    key: 's',
    ctrlKey: true,
    action: actions.toggleSettingsTab,
    description: 'Toggle Settings tab'
  },
  {
    key: 'c',
    ctrlKey: true,
    action: actions.toggleConfigTab,
    description: 'Toggle Hardware Config tab'
  },
  {
    key: ' ',
    action: actions.pingHardware,
    description: 'Ping hardware'
  },
  {
    key: 'e',
    ctrlKey: true,
    action: actions.exportData,
    description: 'Export data'
  },
  {
    key: 'r',
    ctrlKey: true,
    action: actions.refreshConnection,
    description: 'Refresh connection'
  },
  {
    key: 'Escape',
    action: actions.closeModals,
    description: 'Close modals'
  }
]
