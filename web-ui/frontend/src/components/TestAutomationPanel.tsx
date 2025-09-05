import { useState, useEffect } from 'react'
import { useTestAutomation } from '../hooks/useTestAutomation'
import { TestScenario, TestExecution } from '../types'
import TestResultsModal from './TestResultsModal'

interface TestAutomationPanelProps {
  onPinHighlight?: (pins: string[]) => void
  onTestProgress?: (execution: TestExecution) => void
}

export default function TestAutomationPanel({ onPinHighlight, onTestProgress }: TestAutomationPanelProps) {
  const {
    availableScenarios,
    availableTags,
    currentExecution,
    isExecutionInProgress,
    selectedScenarios,
    filterTags,
    loading,
    error,
    executeScenarios,
    stopExecution,
    toggleScenarioSelection,
    selectAllScenarios,
    clearScenarioSelection,
    setFilterTags,
    getFilteredScenarios,
    getExecutionProgress,
    getCurrentStepProgress,
    formatDuration,
    getStatusColor,
    getStatusIcon,
    clearError
  } = useTestAutomation()

  const [selectedTags, setSelectedTags] = useState<string[]>([])
  const [showAdvanced, setShowAdvanced] = useState(false)
  const [showResultsModal, setShowResultsModal] = useState(false)

  const filteredScenarios = getFilteredScenarios()
  const executionProgress = getExecutionProgress()
  const stepProgress = getCurrentStepProgress()

  // Handle tag filter changes
  const handleTagFilterChange = (tag: string, checked: boolean) => {
    const newTags = checked 
      ? [...selectedTags, tag]
      : selectedTags.filter(t => t !== tag)
    
    setSelectedTags(newTags)
    setFilterTags(newTags)
  }

  // Handle scenario hover for pin highlighting
  const handleScenarioHover = (scenario: TestScenario | null) => {
    if (onPinHighlight && scenario) {
      const pins = new Set<string>()
      scenario.steps.forEach(step => {
        step.pin_interactions.forEach(pin => pins.add(pin))
      })
      onPinHighlight(Array.from(pins))
    } else if (onPinHighlight) {
      onPinHighlight([])
    }
  }

  // Handle test execution start
  const handleExecuteTests = async () => {
    if (selectedScenarios.length === 0) {
      alert('Please select at least one test scenario')
      return
    }

    const success = await executeScenarios(selectedScenarios)
    if (!success) {
      alert('Failed to start test execution')
    }
  }

  // Handle test execution stop
  const handleStopTests = async () => {
    const success = await stopExecution()
    if (!success) {
      alert('Failed to stop test execution')
    }
  }

  // Notify parent of test progress
  useEffect(() => {
    if (onTestProgress && currentExecution) {
      onTestProgress(currentExecution)
    }
  }, [currentExecution, onTestProgress])

  if (loading) {
    return (
      <div className="test-automation-panel loading">
        <div className="loading-spinner">
          <div className="spinner"></div>
          <p>Loading test scenarios...</p>
        </div>
      </div>
    )
  }

  return (
    <div className="test-automation-panel">
      <div className="panel-header">
        <h3>🧪 Test Automation</h3>
        <div className="panel-controls">
          <button 
            className="btn-toggle"
            onClick={() => setShowAdvanced(!showAdvanced)}
          >
            {showAdvanced ? 'Simple' : 'Advanced'}
          </button>
        </div>
      </div>

      {error && (
        <div className="error-banner">
          <span className="error-icon">⚠️</span>
          <span className="error-message">{error}</span>
          <button className="error-close" onClick={clearError}>×</button>
        </div>
      )}

      {/* Test Execution Status */}
      {currentExecution && (
        <div className="execution-status">
          <div className="execution-header">
            <h4>
              {getStatusIcon(currentExecution.status)} 
              Test Execution: {currentExecution.execution_id}
            </h4>
            <div className="execution-controls">
              {!isExecutionInProgress && (
                <button
                  className="btn-view-results"
                  onClick={() => setShowResultsModal(true)}
                  title="View detailed test results"
                >
                  📊 View Results
                </button>
              )}
              {isExecutionInProgress && (
                <button
                  className="btn-stop"
                  onClick={handleStopTests}
                  title="Stop test execution"
                >
                  ⏹️ Stop
                </button>
              )}
            </div>
          </div>

          <div className="execution-progress">
            <div className="progress-bar">
              <div 
                className="progress-fill"
                style={{ 
                  width: `${executionProgress.percentage}%`,
                  backgroundColor: getStatusColor(currentExecution.status)
                }}
              />
            </div>
            <div className="progress-text">
              {executionProgress.current} / {executionProgress.total} scenarios
              ({executionProgress.percentage}%)
            </div>
          </div>

          {isExecutionInProgress && stepProgress.total > 0 && (
            <div className="step-progress">
              <div className="step-progress-bar">
                <div 
                  className="step-progress-fill"
                  style={{ 
                    width: `${(stepProgress.current / stepProgress.total) * 100}%`
                  }}
                />
              </div>
              <div className="step-progress-text">
                Step {stepProgress.current} / {stepProgress.total}
              </div>
            </div>
          )}

          <div className="execution-stats">
            <span className="stat passed">✅ {currentExecution.passed_scenarios}</span>
            <span className="stat failed">❌ {currentExecution.failed_scenarios}</span>
            <span className="stat duration">
              ⏱️ {formatDuration(
                currentExecution.end_time 
                  ? currentExecution.end_time - (currentExecution.start_time || 0)
                  : Date.now() - (currentExecution.start_time || 0)
              )}
            </span>
          </div>
        </div>
      )}

      {/* Tag Filters */}
      {showAdvanced && availableTags.length > 0 && (
        <div className="tag-filters">
          <h4>Filter by Tags:</h4>
          <div className="tag-list">
            {availableTags.map(tag => (
              <label key={tag} className="tag-filter">
                <input
                  type="checkbox"
                  checked={selectedTags.includes(tag)}
                  onChange={(e) => handleTagFilterChange(tag, e.target.checked)}
                />
                <span className="tag-name">@{tag}</span>
              </label>
            ))}
          </div>
        </div>
      )}

      {/* Scenario Selection */}
      <div className="scenario-selection">
        <div className="selection-header">
          <h4>Test Scenarios ({filteredScenarios.length})</h4>
          <div className="selection-controls">
            <button 
              className="btn-select-all"
              onClick={selectAllScenarios}
              disabled={isExecutionInProgress}
            >
              Select All
            </button>
            <button 
              className="btn-clear"
              onClick={clearScenarioSelection}
              disabled={isExecutionInProgress}
            >
              Clear
            </button>
          </div>
        </div>

        <div className="scenario-list">
          {filteredScenarios.map(scenario => (
            <div 
              key={scenario.name}
              className={`scenario-item ${selectedScenarios.includes(scenario.name) ? 'selected' : ''}`}
              onMouseEnter={() => handleScenarioHover(scenario)}
              onMouseLeave={() => handleScenarioHover(null)}
            >
              <label className="scenario-checkbox">
                <input
                  type="checkbox"
                  checked={selectedScenarios.includes(scenario.name)}
                  onChange={() => toggleScenarioSelection(scenario.name)}
                  disabled={isExecutionInProgress}
                />
                <div className="scenario-info">
                  <div className="scenario-name">{scenario.name}</div>
                  <div className="scenario-meta">
                    <span className="feature-file">📄 {scenario.feature_file}</span>
                    {scenario.tags.length > 0 && (
                      <span className="scenario-tags">
                        {scenario.tags.map(tag => (
                          <span key={tag} className="tag">@{tag}</span>
                        ))}
                      </span>
                    )}
                  </div>
                  <div className="scenario-steps">
                    {scenario.steps.length} steps
                    {scenario.steps.some(s => s.pin_interactions.length > 0) && (
                      <span className="pin-interactions">
                        🔌 {Array.from(new Set(
                          scenario.steps.flatMap(s => s.pin_interactions)
                        )).join(', ')}
                      </span>
                    )}
                  </div>
                </div>
              </label>
            </div>
          ))}
        </div>
      </div>

      {/* Execution Controls */}
      <div className="execution-controls">
        <button
          className="btn-execute"
          onClick={handleExecuteTests}
          disabled={isExecutionInProgress || selectedScenarios.length === 0}
        >
          {isExecutionInProgress ? '🔄 Running...' : '▶️ Execute Tests'}
        </button>
        
        <div className="selected-count">
          {selectedScenarios.length} scenario{selectedScenarios.length !== 1 ? 's' : ''} selected
        </div>
      </div>

      <TestResultsModal
        execution={currentExecution}
        visible={showResultsModal}
        onClose={() => setShowResultsModal(false)}
      />
    </div>
  )
}
