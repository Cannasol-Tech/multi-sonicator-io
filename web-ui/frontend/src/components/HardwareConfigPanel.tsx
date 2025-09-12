/**
 * Hardware Configuration Panel Component
 * Displays and manages hardware configuration for Multi-Sonicator-IO
 */

import { useState, useEffect } from 'react'

interface HardwareConfigSummary {
  project: string
  version: string
  dut: string
  harness: string
  sonicators: {
    total: number
    connected: number
    channels: number[]
  }
  capabilities: {
    total: number
    supported: number
    names: string[]
  }
  simulation_mode: boolean
  limitations: string
}

interface SonicatorChannel {
  id: number
  name: string
  db9_connector: string
  power_range: [number, number]
  frequency_range: [number, number]
  amplitude_range: [number, number]
  status: 'connected' | 'not_connected' | 'error'
}

interface TestCapability {
  name: string
  description: string
  pins: string[]
  supported: boolean
  note?: string
}

export default function HardwareConfigPanel() {
  const [configSummary, setConfigSummary] = useState<HardwareConfigSummary | null>(null)
  const [sonicators, setSonicators] = useState<SonicatorChannel[]>([])
  const [capabilities, setCapabilities] = useState<TestCapability[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [simulationMode, setSimulationMode] = useState(false)

  // Load configuration data
  useEffect(() => {
    loadConfigurationData()
  }, [])

  const loadConfigurationData = async () => {
    try {
      setLoading(true)
      setError(null)

      // Load configuration summary
      const summaryResponse = await fetch('/api/hardware-config/summary')
      if (!summaryResponse.ok) throw new Error('Failed to load configuration summary')
      const summaryData = await summaryResponse.json()
      setConfigSummary(summaryData.data)
      setSimulationMode(summaryData.data.simulation_mode)

      // Load sonicator configuration
      const sonicatorsResponse = await fetch('/api/hardware-config/sonicators')
      if (!sonicatorsResponse.ok) throw new Error('Failed to load sonicator configuration')
      const sonicatorsData = await sonicatorsResponse.json()
      setSonicators(sonicatorsData.data.channels)

      // Load test capabilities
      const capabilitiesResponse = await fetch('/api/hardware-config/capabilities')
      if (!capabilitiesResponse.ok) throw new Error('Failed to load test capabilities')
      const capabilitiesData = await capabilitiesResponse.json()
      setCapabilities(capabilitiesData.data.all)

    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unknown error')
      console.error('Failed to load configuration:', err)
    } finally {
      setLoading(false)
    }
  }

  const toggleSimulationMode = async () => {
    try {
      const response = await fetch('/api/hardware-config/simulation', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ enabled: !simulationMode })
      })

      if (!response.ok) throw new Error('Failed to toggle simulation mode')
      
      const data = await response.json()
      setSimulationMode(data.data.simulation_mode)
      
      // Reload configuration to reflect changes
      await loadConfigurationData()
    } catch (err) {
      console.error('Failed to toggle simulation mode:', err)
      setError(err instanceof Error ? err.message : 'Failed to toggle simulation mode')
    }
  }

  const reloadConfiguration = async () => {
    try {
      const response = await fetch('/api/hardware-config/reload', { method: 'POST' })
      if (!response.ok) throw new Error('Failed to reload configuration')
      
      await loadConfigurationData()
    } catch (err) {
      console.error('Failed to reload configuration:', err)
      setError(err instanceof Error ? err.message : 'Failed to reload configuration')
    }
  }

  const validateConfiguration = async () => {
    try {
      const response = await fetch('/api/hardware-config/validate')
      if (!response.ok) throw new Error('Failed to validate configuration')
      
      const data = await response.json()
      if (data.data.valid) {
        alert('✅ Configuration validation passed!')
      } else {
        alert(`❌ Configuration validation failed:\n${data.data.errors.join('\n')}`)
      }
    } catch (err) {
      console.error('Failed to validate configuration:', err)
      alert(`❌ Configuration validation error: ${err instanceof Error ? err.message : 'Unknown error'}`)
    }
  }

  if (loading) {
    return (
      <div className="hardware-config-panel">
        <div className="loading">
          <div className="spinner"></div>
          <p>Loading hardware configuration...</p>
        </div>
      </div>
    )
  }

  if (error) {
    return (
      <div className="hardware-config-panel">
        <div className="error">
          <h3>❌ Configuration Error</h3>
          <p>{error}</p>
          <button onClick={loadConfigurationData}>Retry</button>
        </div>
      </div>
    )
  }

  return (
    <div className="hardware-config-panel">
      <div className="config-header">
        <h2>🔧 Hardware Configuration</h2>
        <div className="config-actions">
          <button onClick={validateConfiguration} className="btn-secondary">
            Validate Config
          </button>
          <button onClick={reloadConfiguration} className="btn-secondary">
            Reload Config
          </button>
          <button 
            onClick={toggleSimulationMode} 
            className={`btn-toggle ${simulationMode ? 'active' : ''}`}
          >
            {simulationMode ? '🔧 Simulation Mode' : '🔌 Hardware Mode'}
          </button>
        </div>
      </div>

      {configSummary && (
        <div className="config-summary">
          <div className="summary-grid">
            <div className="summary-item">
              <h4>Project</h4>
              <p>{configSummary.project} v{configSummary.version}</p>
            </div>
            <div className="summary-item">
              <h4>Device Under Test</h4>
              <p>{configSummary.dut}</p>
            </div>
            <div className="summary-item">
              <h4>Test Harness</h4>
              <p>{configSummary.harness}</p>
            </div>
            <div className="summary-item">
              <h4>Mode</h4>
              <p className={simulationMode ? 'simulation' : 'hardware'}>
                {simulationMode ? '🔧 Simulation' : '🔌 Hardware'}
              </p>
            </div>
          </div>
        </div>
      )}

      <div className="config-sections">
        {/* Sonicator Channels */}
        <div className="config-section">
          <h3>🔊 Sonicator Channels</h3>
          <div className="sonicator-grid">
            {sonicators.map(sonicator => (
              <div key={sonicator.id} className={`sonicator-card ${sonicator.status}`}>
                <div className="sonicator-header">
                  <h4>{sonicator.name}</h4>
                  <span className={`status-badge ${sonicator.status}`}>
                    {sonicator.status === 'connected' ? '✅' : 
                     sonicator.status === 'not_connected' ? '⚪' : '❌'}
                    {sonicator.status.replace('_', ' ')}
                  </span>
                </div>
                <div className="sonicator-details">
                  <p><strong>Connector:</strong> {sonicator.db9_connector}</p>
                  <p><strong>Power:</strong> {sonicator.power_range[0]}-{sonicator.power_range[1]}W</p>
                  <p><strong>Frequency:</strong> {(sonicator.frequency_range[0]/1000).toFixed(2)}-{(sonicator.frequency_range[1]/1000).toFixed(2)}kHz</p>
                  <p><strong>Amplitude:</strong> {sonicator.amplitude_range[0]}-{sonicator.amplitude_range[1]}%</p>
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Test Capabilities */}
        <div className="config-section">
          <h3>🧪 Test Capabilities</h3>
          <div className="capabilities-list">
            {capabilities.map(capability => (
              <div key={capability.name} className={`capability-item ${capability.supported ? 'supported' : 'unsupported'}`}>
                <div className="capability-header">
                  <h4>
                    {capability.supported ? '✅' : '❌'} {capability.name.replace(/_/g, ' ')}
                  </h4>
                  <span className="pin-count">{capability.pins.length} pins</span>
                </div>
                <p className="capability-description">{capability.description}</p>
                {capability.note && (
                  <p className="capability-note">📝 {capability.note}</p>
                )}
                <div className="capability-pins">
                  <strong>Pins:</strong> {capability.pins.join(', ')}
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Current Limitations */}
        {configSummary && (
          <div className="config-section">
            <h3>⚠️ Current Limitations</h3>
            <div className="limitations-info">
              <p>{configSummary.limitations}</p>
              <div className="limitation-details">
                <p><strong>Connected Channels:</strong> {configSummary.sonicators.channels.join(', ')}</p>
                <p><strong>Supported Capabilities:</strong> {configSummary.capabilities.supported}/{configSummary.capabilities.total}</p>
              </div>
            </div>
          </div>
        )}
      </div>

      <style>{`
        .hardware-config-panel {
          padding: var(--spacing-lg);
          max-width: 1200px;
          margin: 0 auto;
          color: var(--text-primary);
        }

        .config-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: var(--spacing-lg);
          padding-bottom: var(--spacing-md);
          border-bottom: 2px solid var(--border-color);
        }

        .config-actions {
          display: flex;
          gap: var(--spacing-sm);
        }

        .btn-secondary, .btn-toggle {
          padding: 8px 16px;
          border: 1px solid var(--border-color);
          background: var(--bg-tertiary);
          color: var(--text-primary);
          border-radius: var(--border-radius);
          cursor: pointer;
          transition: all 0.2s;
        }

        .btn-secondary:hover, .btn-toggle:hover {
          border-color: var(--border-color-hover);
          background: var(--bg-secondary);
        }

        .btn-toggle.active {
          background: var(--color-success);
          color: var(--text-primary);
          border-color: var(--color-success);
        }

        .config-summary {
          background: var(--bg-secondary);
          padding: var(--spacing-md);
          border: 1px solid var(--border-color);
          border-radius: 8px;
          margin-bottom: var(--spacing-lg);
        }

        .summary-grid {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
          gap: var(--spacing-md);
        }

        .summary-item h4 {
          margin: 0 0 5px 0;
          color: var(--text-tertiary);
          font-size: 0.9em;
        }

        .summary-item p {
          margin: 0;
          font-weight: bold;
          color: var(--text-primary);
        }

        .summary-item p.simulation {
          color: var(--color-warning);
        }

        .summary-item p.hardware {
          color: var(--color-success);
        }

        .config-sections {
          display: flex;
          flex-direction: column;
          gap: 30px;
        }

        .config-section h3 {
          margin-bottom: var(--spacing-md);
          color: var(--text-primary);
        }

        .sonicator-grid {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
          gap: var(--spacing-md);
        }

        .sonicator-card {
          border: 2px solid var(--border-color);
          border-radius: 8px;
          padding: var(--spacing-md);
          background: var(--bg-primary);
          transition: background-color 0.3s ease, border-color 0.3s ease;
        }

        .sonicator-card.connected {
          border-color: var(--color-success);
          background: rgba(16, 185, 129, 0.06);
        }

        .sonicator-card.not_connected {
          border-color: var(--border-color);
          background: var(--bg-secondary);
        }

        .sonicator-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: var(--spacing-sm);
        }

        .status-badge {
          padding: 4px 8px;
          border-radius: 12px;
          font-size: 0.8em;
          font-weight: bold;
          background: var(--bg-tertiary);
          color: var(--text-secondary);
          border: 1px solid var(--border-color);
        }

        .status-badge.connected {
          background: rgba(16, 185, 129, 0.15);
          color: var(--color-success);
          border-color: rgba(16, 185, 129, 0.3);
        }

        .status-badge.not_connected {
          background: var(--bg-tertiary);
          color: var(--text-tertiary);
        }

        .capabilities-list {
          display: flex;
          flex-direction: column;
          gap: var(--spacing-md);
        }

        .capability-item {
          border: 1px solid var(--border-color);
          border-radius: 8px;
          padding: var(--spacing-md);
          background: var(--bg-primary);
          transition: background-color 0.3s ease, border-color 0.3s ease;
        }

        .capability-item.supported {
          border-color: var(--color-success);
          background: rgba(16, 185, 129, 0.06);
        }

        .capability-item.unsupported {
          border-color: var(--border-color);
          background: var(--bg-secondary);
        }

        .capability-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 8px;
        }

        .pin-count {
          background: var(--bg-tertiary);
          padding: 2px 8px;
          border-radius: 12px;
          font-size: 0.8em;
          color: var(--text-tertiary);
          border: 1px solid var(--border-color);
        }

        .capability-description {
          margin: 8px 0;
          color: var(--text-secondary);
        }

        .capability-note {
          margin: 8px 0;
          color: var(--color-warning);
          font-style: italic;
        }

        .capability-pins {
          margin-top: 10px;
          font-size: 0.9em;
          color: var(--text-tertiary);
        }

        .limitations-info {
          background: var(--bg-secondary);
          border: 1px solid var(--border-color);
          border-radius: 8px;
          padding: var(--spacing-md);
        }

        .limitation-details {
          margin-top: var(--spacing-sm);
          padding-top: var(--spacing-sm);
          border-top: 1px solid var(--border-color);
        }

        .loading, .error {
          text-align: center;
          padding: 40px;
          color: var(--text-primary);
        }

        .spinner {
          border: 4px solid var(--border-color);
          border-top: 4px solid var(--color-primary);
          border-radius: 50%;
          width: 40px;
          height: 40px;
          animation: spin 2s linear infinite;
          margin: 0 auto 20px;
        }

        @keyframes spin {
          0% { transform: rotate(0deg); }
          100% { transform: rotate(360deg); }
        }
      `}</style>
    </div>
  )
}
