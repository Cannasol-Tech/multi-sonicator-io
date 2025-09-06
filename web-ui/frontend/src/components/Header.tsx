import { useState, useEffect } from 'react'
import { ConnectionStatus } from '../types'
import { api } from '../services/api'

interface HeaderProps {
  connectionStatus: ConnectionStatus
  onReconnect?: () => void
  onShowHelp?: () => void
}

export default function Header({ connectionStatus, onShowHelp }: HeaderProps) {
  const [reconnecting, setReconnecting] = useState(false)
  const [retrying, setRetrying] = useState(false)
  const [theme, setTheme] = useState<'light' | 'dark' | 'auto'>('auto')

  // Load theme from localStorage on mount
  useEffect(() => {
    const savedTheme = localStorage.getItem('multi-sonicator-theme') as 'light' | 'dark' | 'auto'
    if (savedTheme) {
      setTheme(savedTheme)
      applyTheme(savedTheme)
    } else {
      applyTheme('auto')
    }
  }, [])

  const applyTheme = (newTheme: 'light' | 'dark' | 'auto') => {
    const root = document.documentElement

    if (newTheme === 'dark') {
      root.classList.add('dark-theme')
    } else if (newTheme === 'light') {
      root.classList.remove('dark-theme')
    } else {
      // Auto theme based on system preference
      const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)')
      if (mediaQuery.matches) {
        root.classList.add('dark-theme')
      } else {
        root.classList.remove('dark-theme')
      }
    }
  }

  const toggleTheme = () => {
    const themes: ('light' | 'dark' | 'auto')[] = ['light', 'dark', 'auto']
    const currentIndex = themes.indexOf(theme)
    const nextTheme = themes[(currentIndex + 1) % themes.length]

    setTheme(nextTheme)
    applyTheme(nextTheme)
    localStorage.setItem('multi-sonicator-theme', nextTheme)
  }

  const getThemeIcon = () => {
    switch (theme) {
      case 'light': return '☀️'
      case 'dark': return '🌙'
      case 'auto': return '🌓'
      default: return '🌓'
    }
  }

  const getConnectionText = () => {
    if (reconnecting) return 'Reconnecting...'
    if (retrying) return 'Retrying connection...'
    if (connectionStatus.connected) {
      return `Connected${connectionStatus.port ? ` to ${connectionStatus.port}` : ''}`
    }
    return connectionStatus.error || 'Disconnected'
  }

  const getConnectionClass = () => {
    if (reconnecting || retrying) return 'connecting'
    if (connectionStatus.connected) return 'connected'
    return 'disconnected'
  }

  const handleReconnect = async () => {
    setReconnecting(true)
    try {
      await api.reconnect()
      // The WebSocket will handle the connection status update
    } catch (error) {
      console.error('Reconnection failed:', error)
    } finally {
      setReconnecting(false)
    }
  }

  const handleRetryConnection = async () => {
    setRetrying(true)
    try {
      await api.retryConnection(3, 2000)
      // The WebSocket will handle the connection status update
    } catch (error) {
      console.error('Connection retry failed:', error)
    } finally {
      setRetrying(false)
    }
  }

  return (
    <header className="app-header">
      <div className="flex items-center gap-2">
        <h1 className="text-base font-semibold">Multi-Sonicator-IO Test Harness</h1>
        <span className="text-xs text-gray-500">
          Arduino Test Wrapper ↔ ATmega32A DUT
        </span>
      </div>
      
      <div className="flex items-center gap-4">
        <div className="connection-status">
          <div className={`connection-indicator ${getConnectionClass()}`}></div>
          <span>{getConnectionText()}</span>
        </div>
        
        {!connectionStatus.connected && (
          <div className="flex gap-2">
            <button
              className="btn primary"
              onClick={handleReconnect}
              disabled={reconnecting || retrying}
              title="Quick reconnection attempt"
            >
              {reconnecting ? 'Reconnecting...' : 'Reconnect'}
            </button>
            <button
              className="btn"
              onClick={handleRetryConnection}
              disabled={reconnecting || retrying}
              title="Retry connection with multiple attempts"
            >
              {retrying ? 'Retrying...' : 'Retry (3x)'}
            </button>
          </div>
        )}
        
        <button
          className="btn theme-toggle"
          onClick={toggleTheme}
          title={`Current theme: ${theme}. Click to cycle through light/dark/auto`}
        >
          {getThemeIcon()}
        </button>

        <button
          className="btn"
          onClick={onShowHelp}
          title="Show comprehensive help system"
        >
          Help
        </button>
      </div>
    </header>
  )
}
