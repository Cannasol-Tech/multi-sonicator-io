import { useState } from 'react'
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
