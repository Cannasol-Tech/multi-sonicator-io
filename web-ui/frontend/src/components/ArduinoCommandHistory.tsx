import React from 'react'
import { ArduinoCommand } from '../hooks/useArduinoCommandLog'

interface ArduinoCommandHistoryProps {
  /** Array of recent command/response pairs */
  commandPairs: Array<{sent: ArduinoCommand, received?: ArduinoCommand}>
  /** Whether the hardware is currently connected */
  connected: boolean
  /** Optional callback when user clicks on a command entry */
  onCommandClick?: (command: ArduinoCommand) => void
  /** Maximum number of entries to display */
  maxEntries?: number
}

/**
 * ArduinoCommandHistory Component
 * 
 * Displays the last 5 Arduino commands sent to the Test Wrapper and their responses.
 * Shows real-time communication status with timestamps and response times.
 * 
 * @component
 * @example
 * ```tsx
 * <ArduinoCommandHistory 
 *   commandPairs={recentPairs}
 *   connected={true}
 *   maxEntries={5}
 * />
 * ```
 */
export default function ArduinoCommandHistory({ 
  commandPairs, 
  connected, 
  onCommandClick,
  maxEntries = 5 
}: ArduinoCommandHistoryProps) {
  
  const formatTimestamp = (timestamp: number): string => {
    return new Date(timestamp).toLocaleTimeString('en-US', {
      hour12: false,
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit'
    })
  }

  const formatResponseTime = (responseTime?: number): string => {
    if (!responseTime) return 'N/A'
    
    if (responseTime < 10) return `${responseTime}ms 🟢`
    if (responseTime < 100) return `${responseTime}ms 🟡`
    if (responseTime < 500) return `${responseTime}ms 🟠`
    return `${responseTime}ms 🔴`
  }

  const getStatusIcon = (status?: string): string => {
    switch (status) {
      case 'success': return '✅'
      case 'error': return '❌'
      case 'timeout': return '⏱️'
      case 'pending': return '⏳'
      default: return '❓'
    }
  }

  const truncateCommand = (data: string, maxLength: number = 50): string => {
    if (data.length <= maxLength) return data
    return `${data.substring(0, maxLength)}...`
  }

  // Display only the requested number of entries
  const displayPairs = commandPairs.slice(0, maxEntries)

  return (
    <div className="arduino-command-history">
      <div className="command-history-header">
        <h4>📡 Arduino Communication Log</h4>
        <div className="history-subtitle">
          <span>Last {maxEntries} Commands & Responses</span>
          <div className={`connection-indicator ${connected ? 'connected' : 'disconnected'}`}>
            <span className="connection-dot"></span>
            {connected ? 'Live' : 'Offline'}
          </div>
        </div>
      </div>

      <div className="command-history-content">
        {displayPairs.length === 0 ? (
          <div className="no-commands-message">
            <span className="no-commands-icon">📭</span>
            <p>No commands sent yet</p>
            <small>Arduino commands will appear here in real-time</small>
          </div>
        ) : (
          <div className="command-pairs-list">
            {displayPairs.map((pair, index) => (
              <div key={pair.sent.id || index} className="command-pair">
                {/* Sent Command */}
                <div 
                  className={`command-entry sent ${onCommandClick ? 'clickable' : ''}`}
                  onClick={() => onCommandClick?.(pair.sent)}
                >
                  <div className="command-header">
                    <span className="command-direction">📤 SENT</span>
                    <span className="command-timestamp">
                      {formatTimestamp(pair.sent.timestamp)}
                    </span>
                    <span className="command-status">
                      {getStatusIcon(pair.sent.status)}
                    </span>
                  </div>
                  <div className="command-data">
                    <code>{truncateCommand(pair.sent.data)}</code>
                  </div>
                  {pair.sent.responseTime && (
                    <div className="response-time">
                      Response: {formatResponseTime(pair.sent.responseTime)}
                    </div>
                  )}
                </div>

                {/* Received Response */}
                {pair.received ? (
                  <div 
                    className={`command-entry received ${onCommandClick ? 'clickable' : ''}`}
                    onClick={() => onCommandClick?.(pair.received!)}
                  >
                    <div className="command-header">
                      <span className="command-direction">📥 RECEIVED</span>
                      <span className="command-timestamp">
                        {formatTimestamp(pair.received.timestamp)}
                      </span>
                      <span className="command-status">
                        {getStatusIcon(pair.received.status)}
                      </span>
                    </div>
                    <div className="command-data">
                      <code>{truncateCommand(pair.received.data)}</code>
                    </div>
                    {pair.received.error && (
                      <div className="error-message">
                        ⚠️ {pair.received.error}
                      </div>
                    )}
                  </div>
                ) : (
                  <div className="command-entry no-response">
                    <div className="command-header">
                      <span className="command-direction">📥 RESPONSE</span>
                      <span className="no-response-text">No response received</span>
                    </div>
                  </div>
                )}

                {/* Separator line between pairs */}
                {index < displayPairs.length - 1 && (
                  <div className="command-pair-separator"></div>
                )}
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Footer with summary info */}
      <div className="command-history-footer">
        <div className="history-stats">
          <span className="stat-item">
            📊 Total: {commandPairs.length}
          </span>
          <span className="stat-item">
            ✅ Success: {commandPairs.filter(p => p.sent.status === 'success').length}
          </span>
          <span className="stat-item">
            ❌ Errors: {commandPairs.filter(p => p.sent.status === 'error').length}
          </span>
        </div>
      </div>
    </div>
  )
}
