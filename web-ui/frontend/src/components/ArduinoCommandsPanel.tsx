import { useState, useEffect } from 'react'
import { HardwareState } from '../types'

interface ArduinoCommandsPanelProps {
  hardwareState: HardwareState
  connected: boolean
}

export default function ArduinoCommandsPanel({ hardwareState, connected }: ArduinoCommandsPanelProps) {
  const [pingStatus, setPingStatus] = useState<'idle' | 'loading' | 'success' | 'error'>('idle')
  const [pingResponse, setPingResponse] = useState<any>(null)
  const [lastPingTime, setLastPingTime] = useState<Date | null>(null)

  const handlePing = async () => {
    setPingStatus('loading')
    setPingResponse(null)
    
    try {
      const response = await fetch('http://localhost:3001/api/ping', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
      })
      
      const data = await response.json()
      setPingResponse(data)
      setPingStatus('success')
      setLastPingTime(new Date())
    } catch (error) {
      console.error('Ping failed:', error)
      setPingStatus('error')
      setPingResponse({ error: 'Connection failed' })
    }
  }

  const formatResponseTime = (responseTime?: number) => {
    if (!responseTime) return 'N/A'
    return `${responseTime}ms`
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
                
                {pingStatus === 'success' && pingResponse.hardware && (
                  <div className="response-details">
                    <div className="detail-item">
                      <span className="detail-label">Response Time:</span>
                      <span className="detail-value">{formatResponseTime(pingResponse.responseTime)}</span>
                    </div>
                    <div className="detail-item">
                      <span className="detail-label">Hardware Port:</span>
                      <span className="detail-value">{pingResponse.hardware.port || 'Mock'}</span>
                    </div>
                    <div className="detail-item">
                      <span className="detail-label">Status:</span>
                      <span className="detail-value">{pingResponse.status}</span>
                    </div>
                  </div>
                )}
                
                {pingStatus === 'error' && (
                  <div className="error-details">
                    <span className="error-message">{pingResponse.error}</span>
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
