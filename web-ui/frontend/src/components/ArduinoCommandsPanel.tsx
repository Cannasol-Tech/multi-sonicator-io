import { useState, useEffect } from 'react'
import { HardwareState } from '../types'

interface ArduinoCommandsPanelProps {
  hardwareState: HardwareState
  connected: boolean
}

interface PingHistoryEntry {
  timestamp: Date
  responseTime: number
  success: boolean
  error?: string
}

export default function ArduinoCommandsPanel({ hardwareState, connected }: ArduinoCommandsPanelProps) {
  const [pingStatus, setPingStatus] = useState<'idle' | 'loading' | 'success' | 'error'>('idle')
  const [pingResponse, setPingResponse] = useState<any>(null)
  const [lastPingTime, setLastPingTime] = useState<Date | null>(null)
  const [pingHistory, setPingHistory] = useState<PingHistoryEntry[]>([])

  const handlePing = async () => {
    setPingStatus('loading')
    setPingResponse(null)
    const startTime = Date.now()

    try {
      const response = await fetch('http://localhost:3001/api/ping', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
      })

      const data = await response.json()
      const clientResponseTime = Date.now() - startTime

      setPingResponse(data)
      setPingStatus('success')
      setLastPingTime(new Date())

      // Add to ping history (keep last 10 entries)
      const historyEntry: PingHistoryEntry = {
        timestamp: new Date(),
        responseTime: data.responseTime || clientResponseTime,
        success: data.success || response.ok,
        error: data.error
      }

      setPingHistory(prev => [historyEntry, ...prev.slice(0, 9)])

    } catch (error) {
      console.error('Ping failed:', error)
      const clientResponseTime = Date.now() - startTime

      setPingStatus('error')
      const errorResponse = { error: 'Connection failed', responseTime: clientResponseTime }
      setPingResponse(errorResponse)

      // Add failed ping to history
      const historyEntry: PingHistoryEntry = {
        timestamp: new Date(),
        responseTime: clientResponseTime,
        success: false,
        error: 'Connection failed'
      }

      setPingHistory(prev => [historyEntry, ...prev.slice(0, 9)])
    }
  }

  const formatResponseTime = (responseTime?: number) => {
    if (!responseTime) return 'N/A'

    // Add performance indicators
    let indicator = ''
    if (responseTime < 50) {
      indicator = ' 🟢' // Excellent
    } else if (responseTime < 100) {
      indicator = ' 🟡' // Good
    } else if (responseTime < 500) {
      indicator = ' 🟠' // Fair
    } else {
      indicator = ' 🔴' // Slow
    }

    return `${responseTime}ms${indicator}`
  }

  const getStatusIcon = () => {
    switch (pingStatus) {
      case 'loading': return '⏳'
      case 'success': return '✅'
      case 'error': return '❌'
      default: return '🔍'
    }
  }

  const getConnectionStatusColor = () => {
    if (!connected) return '#ef4444' // red
    return hardwareState.connection.status === 'connected' ? '#10b981' : '#f59e0b' // green or amber
  }

  return (
    <div className="arduino-commands-panel">
      <div className="commands-header">
        <h3>🔧 Arduino Test Wrapper Commands</h3>
        <p>Real-time communication with HIL Test Harness</p>
      </div>

      <div className="commands-grid">
        {/* PING Test Section - Primary */}
        <div className="ping-test-primary">
          <h4>Hardware Communication Test</h4>
          <div className="ping-controls-large">
            <button
              className={`ping-button-large ${pingStatus === 'loading' ? 'loading' : ''}`}
              onClick={handlePing}
              disabled={pingStatus === 'loading'}
            >
              {getStatusIcon()} {pingStatus === 'loading' ? 'Testing...' : 'PING Hardware'}
            </button>
            
            {pingResponse && (
              <div className={`ping-response-large ${pingStatus}`}>
                <div className="response-header">
                  <span className="response-status">
                    {pingStatus === 'success' ? '✅ SUCCESS' : '❌ FAILED'}
                  </span>
                  {lastPingTime && (
                    <span className="response-time">
                      {lastPingTime.toLocaleTimeString()}
                    </span>
                  )}
                </div>
                
                {pingStatus === 'success' && pingResponse.success && (
                  <div className="response-details">
                    <div className="detail-item">
                      <span className="detail-label">Response Time:</span>
                      <span className="detail-value">{formatResponseTime(pingResponse.responseTime)}</span>
                    </div>
                    <div className="detail-item">
                      <span className="detail-label">Hardware Response:</span>
                      <span className="detail-value">{pingResponse.result?.data || 'PONG'}</span>
                    </div>
                    <div className="detail-item">
                      <span className="detail-label">Status:</span>
                      <span className="detail-value">{pingResponse.success ? 'Connected' : 'Failed'}</span>
                    </div>
                  </div>
                )}
                
                {pingStatus === 'error' && (
                  <div className="error-details">
                    <div className="detail-item">
                      <span className="detail-label">Error:</span>
                      <span className="detail-value error-text">{pingResponse.error}</span>
                    </div>
                    {pingResponse.responseTime && (
                      <div className="detail-item">
                        <span className="detail-label">Response Time:</span>
                        <span className="detail-value">{formatResponseTime(pingResponse.responseTime)}</span>
                      </div>
                    )}
                  </div>
                )}
              </div>
            )}

            {/* Ping History */}
            {pingHistory.length > 0 && (
              <div className="ping-history">
                <h5>📊 Recent Communication Tests</h5>
                <div className="history-legend">
                  <span className="legend-item">
                    <span className="legend-icon">🟢</span>
                    <span className="legend-text">Fast Response (&lt;50ms)</span>
                  </span>
                  <span className="legend-item">
                    <span className="legend-icon">✅❌</span>
                    <span className="legend-text">Test Result</span>
                  </span>
                </div>
                <div className="history-list">
                  {pingHistory.slice(0, 5).map((entry, index) => (
                    <div
                      key={index}
                      className={`history-item ${entry.success ? 'success' : 'error'}`}
                      title={`${entry.success ? 'Communication Test PASSED' : 'Communication Test FAILED'}
Response Time: ${entry.responseTime}ms (${entry.responseTime < 50 ? 'Excellent' : entry.responseTime < 100 ? 'Good' : entry.responseTime < 500 ? 'Fair' : 'Slow'})
${entry.error ? 'Error: ' + entry.error : ''}
Time: ${entry.timestamp.toLocaleString()}`}
                    >
                      <span className="history-time">
                        {entry.timestamp.toLocaleTimeString()}
                      </span>
                      <span className="history-response-time">
                        {formatResponseTime(entry.responseTime)}
                      </span>
                      <span className="history-status">
                        {entry.success ? '✅' : '❌'}
                      </span>
                    </div>
                  ))}
                </div>
                {pingHistory.length > 0 && (
                  <div className="history-stats">
                    <span className="stat-item">
                      Avg: {Math.round(pingHistory.slice(0, 5).reduce((sum, entry) => sum + entry.responseTime, 0) / Math.min(5, pingHistory.length))}ms
                    </span>
                    <span className="stat-item">
                      Success: {Math.round((pingHistory.slice(0, 10).filter(entry => entry.success).length / Math.min(10, pingHistory.length)) * 100)}%
                    </span>
                  </div>
                )}
              </div>
            )}
          </div>
        </div>

        {/* System Status Section */}
        <div className="system-status-section">
          <h4>System Status</h4>
          <div className="status-indicators">
            <div className="status-item">
              <div 
                className="status-dot" 
                style={{ backgroundColor: getConnectionStatusColor() }}
              ></div>
              <span className="status-label">WebSocket</span>
              <span className="status-value">
                {connected ? 'Connected' : 'Disconnected'}
              </span>
            </div>
            
            <div className="status-item">
              <div 
                className="status-dot" 
                style={{ backgroundColor: hardwareState.connection.status === 'connected' ? '#10b981' : '#ef4444' }}
              ></div>
              <span className="status-label">Hardware</span>
              <span className="status-value">
                {hardwareState.connection.status === 'connected' ? 'Ready' : 'Not Ready'}
              </span>
            </div>
            
            <div className="status-item">
              <div className="status-dot" style={{ backgroundColor: '#3b82f6' }}></div>
              <span className="status-label">Port</span>
              <span className="status-value">
                {hardwareState.connection.port || 'Mock'}
              </span>
            </div>
          </div>
        </div>

        {/* Quick Help Section */}
        <div className="quick-help-section">
          <h4>Quick Help</h4>
          <div className="help-items">
            <div className="help-item">
              <span className="help-icon">🔍</span>
              <div className="help-content">
                <span className="help-title">PING Test</span>
                <span className="help-description">Verify hardware communication</span>
              </div>
            </div>
            
            <div className="help-item">
              <span className="help-icon">⚡</span>
              <div className="help-content">
                <span className="help-title">Pin Control</span>
                <span className="help-description">Use Hardware Control tab</span>
              </div>
            </div>
            
            <div className="help-item">
              <span className="help-icon">🧪</span>
              <div className="help-content">
                <span className="help-title">Test Automation</span>
                <span className="help-description">Use Test Automation tab</span>
              </div>
            </div>
            
            <div className="help-item">
              <span className="help-icon">⌨️</span>
              <div className="help-content">
                <span className="help-title">Keyboard Shortcuts</span>
                <span className="help-description">Press ? for help</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
