import React, { useState, useEffect } from 'react'
import { HardwareState } from '../types'

interface PinPreset {
  id: string
  name: string
  description: string
  pinStates: Record<string, 'HIGH' | 'LOW'>
  category: 'test' | 'calibration' | 'diagnostic' | 'custom'
}

interface BatchOperation {
  id: string
  name: string
  steps: Array<{
    action: 'set_pin' | 'wait' | 'read_pin' | 'ping'
    signal?: string
    value?: 'HIGH' | 'LOW'
    duration?: number
  }>
  description: string
}

interface AdvancedControlPanelProps {
  hardwareState: HardwareState
  onPinClick: (signal: string, action: string, value?: any) => void
  onBatchOperation?: (operation: BatchOperation) => void
}

const DEFAULT_PRESETS: PinPreset[] = [
  {
    id: 'all-low',
    name: 'All Pins LOW',
    description: 'Set all controllable pins to LOW state',
    pinStates: {
      'START_4': 'LOW',
      'RESET_4': 'LOW'
    },
    category: 'test'
  },
  {
    id: 'start-sequence',
    name: 'Start Sequence',
    description: 'Typical startup sequence for sonicator',
    pinStates: {
      'RESET_4': 'HIGH',
      'START_4': 'HIGH'
    },
    category: 'test'
  },
  {
    id: 'reset-sequence',
    name: 'Reset Sequence',
    description: 'Reset all systems',
    pinStates: {
      'START_4': 'LOW',
      'RESET_4': 'LOW'
    },
    category: 'diagnostic'
  },
  {
    id: 'calibration-mode',
    name: 'Calibration Mode',
    description: 'Set pins for calibration testing',
    pinStates: {
      'START_4': 'HIGH',
      'RESET_4': 'LOW'
    },
    category: 'calibration'
  }
]

const DEFAULT_BATCH_OPERATIONS: BatchOperation[] = [
  {
    id: 'power-cycle',
    name: 'Power Cycle Test',
    description: 'Complete power cycle sequence with timing',
    steps: [
      { action: 'set_pin', signal: 'START_4', value: 'LOW' },
      { action: 'set_pin', signal: 'RESET_4', value: 'LOW' },
      { action: 'wait', duration: 1000 },
      { action: 'set_pin', signal: 'RESET_4', value: 'HIGH' },
      { action: 'wait', duration: 500 },
      { action: 'set_pin', signal: 'START_4', value: 'HIGH' },
      { action: 'wait', duration: 2000 },
      { action: 'read_pin', signal: 'FREQ_LOCK_4' },
      { action: 'ping' }
    ]
  },
  {
    id: 'signal-sweep',
    name: 'Signal Sweep Test',
    description: 'Test all output signals in sequence',
    steps: [
      { action: 'set_pin', signal: 'START_4', value: 'HIGH' },
      { action: 'wait', duration: 1000 },
      { action: 'read_pin', signal: 'FREQ_DIV10_4' },
      { action: 'wait', duration: 500 },
      { action: 'read_pin', signal: 'FREQ_LOCK_4' },
      { action: 'wait', duration: 500 },
      { action: 'read_pin', signal: 'OVERLOAD_4' },
      { action: 'set_pin', signal: 'START_4', value: 'LOW' }
    ]
  }
]

const AdvancedControlPanel: React.FC<AdvancedControlPanelProps> = ({
  hardwareState,
  onPinClick,
  onBatchOperation
}) => {
  const [presets, setPresets] = useState<PinPreset[]>(DEFAULT_PRESETS)
  const [batchOperations] = useState<BatchOperation[]>(DEFAULT_BATCH_OPERATIONS)
  const [selectedPreset, setSelectedPreset] = useState<string | null>(null)
  const [isExecutingBatch, setIsExecutingBatch] = useState(false)
  const [batchProgress, setBatchProgress] = useState<{ current: number; total: number } | null>(null)
  const [activeTab, setActiveTab] = useState<'presets' | 'batch' | 'custom'>('presets')
  const [customPresetName, setCustomPresetName] = useState('')
  const [customPresetDescription, setCustomPresetDescription] = useState('')

  // Load saved presets from localStorage
  useEffect(() => {
    const savedPresets = localStorage.getItem('multi-sonicator-presets')
    if (savedPresets) {
      try {
        const parsed = JSON.parse(savedPresets)
        setPresets([...DEFAULT_PRESETS, ...parsed])
      } catch (error) {
        console.error('Failed to load saved presets:', error)
      }
    }
  }, [])

  const savePresets = (newPresets: PinPreset[]) => {
    const customPresets = newPresets.filter(preset => preset.category === 'custom')
    localStorage.setItem('multi-sonicator-presets', JSON.stringify(customPresets))
  }

  const applyPreset = async (preset: PinPreset) => {
    setSelectedPreset(preset.id)
    
    for (const [signal, value] of Object.entries(preset.pinStates)) {
      onPinClick(signal, 'set', value)
      await new Promise(resolve => setTimeout(resolve, 100)) // Small delay between commands
    }

    // Clear selection after a delay
    setTimeout(() => setSelectedPreset(null), 2000)
  }

  const executeBatchOperation = async (operation: BatchOperation) => {
    if (isExecutingBatch) return

    setIsExecutingBatch(true)
    setBatchProgress({ current: 0, total: operation.steps.length })

    try {
      for (let i = 0; i < operation.steps.length; i++) {
        const step = operation.steps[i]
        setBatchProgress({ current: i + 1, total: operation.steps.length })

        switch (step.action) {
          case 'set_pin':
            if (step.signal && step.value) {
              onPinClick(step.signal, 'set', step.value)
            }
            break
          case 'read_pin':
            if (step.signal) {
              onPinClick(step.signal, 'read')
            }
            break
          case 'ping':
            // Trigger ping
            fetch('/api/ping', { method: 'POST' })
            break
          case 'wait':
            if (step.duration) {
              await new Promise(resolve => setTimeout(resolve, step.duration))
            }
            break
        }

        // Small delay between steps
        await new Promise(resolve => setTimeout(resolve, 50))
      }

      onBatchOperation?.(operation)
    } catch (error) {
      console.error('Batch operation failed:', error)
    } finally {
      setIsExecutingBatch(false)
      setBatchProgress(null)
    }
  }

  const createCustomPreset = () => {
    if (!customPresetName.trim()) return

    const currentStates: Record<string, 'HIGH' | 'LOW'> = {}
    
    // Get current states of controllable pins (exclude UART communication pins)
    const controllablePins = ['START_4', 'RESET_4', 'POWER_SENSE_4', 'FREQ_DIV10_4'] // UART pins excluded for Modbus communication
    controllablePins.forEach(signal => {
      const pinState = hardwareState.pins[signal]
      if (pinState && (pinState.state === 'HIGH' || pinState.state === 'LOW')) {
        currentStates[signal] = pinState.state
      }
    })

    const newPreset: PinPreset = {
      id: `custom-${Date.now()}`,
      name: customPresetName,
      description: customPresetDescription || 'Custom preset',
      pinStates: currentStates,
      category: 'custom'
    }

    const updatedPresets = [...presets, newPreset]
    setPresets(updatedPresets)
    savePresets(updatedPresets)

    // Clear form
    setCustomPresetName('')
    setCustomPresetDescription('')
  }

  const deletePreset = (presetId: string) => {
    const updatedPresets = presets.filter(preset => preset.id !== presetId)
    setPresets(updatedPresets)
    savePresets(updatedPresets)
  }

  const getPresetsByCategory = (category: string) => {
    return presets.filter(preset => preset.category === category)
  }

  // Power Level Control Functions
  const getCurrentPowerLevel = (): number => {
    const powerState = hardwareState.pins['POWER_SENSE_4']
    if (!powerState || typeof powerState.state !== 'number') return 0
    return Math.round((powerState.state / 1023) * 100) // Convert from 0-1023 to 0-100%
  }

  const setPowerLevel = (percentage: number) => {
    const analogValue = Math.round((percentage / 100) * 1023) // Convert from 0-100% to 0-1023
    onPinClick('POWER_SENSE_4', analogValue.toString())
  }

  const handlePowerLevelChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const percentage = parseInt(event.target.value)
    setPowerLevel(percentage)
  }

  const getPowerStatus = (): string => {
    const level = getCurrentPowerLevel()
    if (level === 0) return 'OFF'
    if (level <= 25) return 'LOW'
    if (level <= 50) return 'MEDIUM'
    if (level <= 75) return 'HIGH'
    return 'MAXIMUM'
  }

  return (
    <div className="advanced-control-panel">
      <div className="control-panel-header">
        <h3>🎛️ Advanced Control Panel</h3>
        <div className="control-tabs">
          {[
            { id: 'presets', label: '📋 Presets' },
            { id: 'batch', label: '⚡ Batch Ops' },
            { id: 'custom', label: '🛠️ Custom' }
          ].map(tab => (
            <button
              key={tab.id}
              className={`control-tab ${activeTab === tab.id ? 'active' : ''}`}
              onClick={() => setActiveTab(tab.id as any)}
            >
              {tab.label}
            </button>
          ))}
        </div>
      </div>

      <div className="control-content">
        {activeTab === 'presets' && (
          <div className="presets-section">
            {['test', 'calibration', 'diagnostic', 'custom'].map(category => {
              const categoryPresets = getPresetsByCategory(category)
              if (categoryPresets.length === 0) return null

              return (
                <div key={category} className="preset-category">
                  <h4>{category.charAt(0).toUpperCase() + category.slice(1)} Presets</h4>
                  <div className="preset-grid">
                    {categoryPresets.map(preset => (
                      <div
                        key={preset.id}
                        className={`preset-card ${selectedPreset === preset.id ? 'active' : ''}`}
                      >
                        <div className="preset-header">
                          <h5>{preset.name}</h5>
                          {preset.category === 'custom' && (
                            <button
                              className="delete-preset-btn"
                              onClick={() => deletePreset(preset.id)}
                              title="Delete preset"
                            >
                              🗑️
                            </button>
                          )}
                        </div>
                        <p className="preset-description">{preset.description}</p>
                        <div className="preset-states">
                          {Object.entries(preset.pinStates).map(([signal, state]) => (
                            <span key={signal} className={`state-chip ${state.toLowerCase()}`}>
                              {signal}: {state}
                            </span>
                          ))}
                        </div>
                        <button
                          className="apply-preset-btn"
                          onClick={() => applyPreset(preset)}
                          disabled={selectedPreset === preset.id}
                        >
                          {selectedPreset === preset.id ? '✅ Applied' : '▶️ Apply'}
                        </button>
                      </div>
                    ))}
                  </div>
                </div>
              )
            })}
          </div>
        )}

        {activeTab === 'batch' && (
          <div className="batch-section">
            <div className="batch-operations">
              {batchOperations.map(operation => (
                <div key={operation.id} className="batch-card">
                  <div className="batch-header">
                    <h5>{operation.name}</h5>
                    <span className="step-count">{operation.steps.length} steps</span>
                  </div>
                  <p className="batch-description">{operation.description}</p>
                  <div className="batch-steps">
                    {operation.steps.slice(0, 3).map((step, index) => (
                      <span key={index} className="step-preview">
                        {step.action.replace('_', ' ')}
                        {step.signal && ` (${step.signal})`}
                      </span>
                    ))}
                    {operation.steps.length > 3 && <span className="step-more">...</span>}
                  </div>
                  <button
                    className="execute-batch-btn"
                    onClick={() => executeBatchOperation(operation)}
                    disabled={isExecutingBatch}
                  >
                    {isExecutingBatch ? '⏳ Executing...' : '▶️ Execute'}
                  </button>
                </div>
              ))}
            </div>

            {batchProgress && (
              <div className="batch-progress">
                <div className="progress-header">
                  <span>Executing batch operation...</span>
                  <span>{batchProgress.current}/{batchProgress.total}</span>
                </div>
                <div className="progress-bar">
                  <div 
                    className="progress-fill"
                    style={{ width: `${(batchProgress.current / batchProgress.total) * 100}%` }}
                  />
                </div>
              </div>
            )}
          </div>
        )}

        {activeTab === 'custom' && (
          <div className="custom-section">
            <div className="custom-preset-creator">
              <h4>Create Custom Preset</h4>
              <p>Set pins to desired states, then save as a preset for quick access.</p>
              
              <div className="form-group">
                <label>Preset Name</label>
                <input
                  type="text"
                  value={customPresetName}
                  onChange={(e) => setCustomPresetName(e.target.value)}
                  placeholder="Enter preset name..."
                />
              </div>

              <div className="form-group">
                <label>Description (optional)</label>
                <textarea
                  value={customPresetDescription}
                  onChange={(e) => setCustomPresetDescription(e.target.value)}
                  placeholder="Describe what this preset does..."
                  rows={3}
                />
              </div>

              <div className="current-states">
                <h5>Current Controllable Pin States</h5>
                <div className="state-display">
                  {['START_4', 'RESET_4', 'POWER_SENSE_4', 'FREQ_DIV10_4'].map(signal => { // Only controllable pins, UART excluded
                    const pinState = hardwareState.pins[signal]
                    return (
                      <div key={signal} className="current-state">
                        <span className="signal-name">{signal}</span>
                        <span className={`state-value ${String(pinState?.state || '').toLowerCase() || 'unknown'}`}>
                          {pinState?.state || 'UNKNOWN'}
                        </span>
                      </div>
                    )
                  })}
                </div>
              </div>

              <button
                className="create-preset-btn"
                onClick={createCustomPreset}
                disabled={!customPresetName.trim()}
              >
                💾 Save as Preset
              </button>
            </div>
          </div>
        )}

        {/* Power Level Control */}
        <div className="control-section">
          <div className="section-header">
            <h4 className="sonicator-4-header">🔋 Sonicator #4 Power Control</h4>
            <p>Simulate different power levels for Sonicator #4</p>
          </div>
          <div className="power-control">
            <div className="power-slider-container">
              <label htmlFor="power-slider">Power Level:</label>
              <div className="slider-with-display">
                <input
                  id="power-slider"
                  type="range"
                  min="0"
                  max="100"
                  value={getCurrentPowerLevel()}
                  onChange={handlePowerLevelChange}
                  className="power-slider"
                />
                <div className="power-display">
                  <span className="power-value">{getCurrentPowerLevel()}%</span>
                  <span className="power-status">{getPowerStatus()}</span>
                </div>
              </div>
            </div>
            <div className="power-presets">
              <button onClick={() => setPowerLevel(0)} className="preset-btn off">OFF</button>
              <button onClick={() => setPowerLevel(25)} className="preset-btn low">25%</button>
              <button onClick={() => setPowerLevel(50)} className="preset-btn medium">50%</button>
              <button onClick={() => setPowerLevel(75)} className="preset-btn high">75%</button>
              <button onClick={() => setPowerLevel(100)} className="preset-btn max">100%</button>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}

export default AdvancedControlPanel
