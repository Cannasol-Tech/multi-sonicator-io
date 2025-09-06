import { useState, useEffect } from 'react'
import { useTestAutomation } from '../hooks/useTestAutomation'
import { TestScenario, TestExecution } from '../types'
import TestResultsModal from './TestResultsModal'
import IridescentProgressBar from './IridescentProgressBar'

interface TestAutomationPanelProps {
  onPinHighlight?: (pins: string[]) => void
  onTestProgress?: (execution: TestExecution) => void
}

export default function TestAutomationPanel({ onPinHighlight, onTestProgress }: TestAutomationPanelProps) {
  const {
    availableTags,
    currentExecution,
    isExecutionInProgress,
    selectedScenarios,
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

  // Handle individual test execution
  const handleExecuteIndividualTest = async (scenarioName: string) => {
    const success = await executeScenarios([scenarioName])
    if (!success) {
      alert(`Failed to start execution for ${scenarioName}`)
    }
  }

  // Handle full acceptance test suite execution
  const handleExecuteFullSuite = async () => {
    const allScenarios = getFilteredScenarios().map(scenario => scenario.name)
    if (allScenarios.length === 0) {
      alert('No test scenarios available')
      return
    }

    const success = await executeScenarios(allScenarios)
    if (!success) {
      alert('Failed to start full acceptance test suite execution')
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

      {/* Enhanced Test Execution Status */}
      {currentExecution && (
        <div className="execution-status-enhanced">
          <div className="execution-header">
            <div className="execution-title-section">
              <h3 className="execution-title">
                {getStatusIcon(currentExecution.status)}
                Test Execution
              </h3>
              <div className="execution-id">ID: {currentExecution.execution_id}</div>
              <div className="execution-status-badge">
                <span className={`status-indicator ${currentExecution.status}`}>
                  {currentExecution.status.toUpperCase()}
                </span>
              </div>
            </div>
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

          <IridescentProgressBar
            progress={executionProgress.percentage}
            isRunning={isExecutionInProgress}
            isComplete={currentExecution.status === 'passed'}
            totalItems={executionProgress.total}
            currentItem={executionProgress.current}
            itemLabel="scenarios"
            showStepProgress={isExecutionInProgress && stepProgress.total > 0}
            stepProgress={(stepProgress.current / stepProgress.total) * 100}
            currentStep={stepProgress.current}
            totalSteps={stepProgress.total}
            className="test-execution-progress"
          />

          <div className="execution-stats-enhanced">
            <div className="stats-grid">
              <div className="stat-card passed">
                <div className="stat-icon">✅</div>
                <div className="stat-content">
                  <div className="stat-value">{currentExecution.passed_scenarios}</div>
                  <div className="stat-label">Passed</div>
                </div>
              </div>
              <div className="stat-card failed">
                <div className="stat-icon">❌</div>
                <div className="stat-content">
                  <div className="stat-value">{currentExecution.failed_scenarios}</div>
                  <div className="stat-label">Failed</div>
                </div>
              </div>
              <div className="stat-card total">
                <div className="stat-icon">📊</div>
                <div className="stat-content">
                  <div className="stat-value">{currentExecution.total_scenarios}</div>
                  <div className="stat-label">Total</div>
                </div>
              </div>
              <div className="stat-card duration">
                <div className="stat-icon">⏱️</div>
                <div className="stat-content">
                  <div className="stat-value">
                    {formatDuration(
                      currentExecution.end_time && currentExecution.start_time
                        ? currentExecution.end_time - currentExecution.start_time
                        : currentExecution.start_time
                          ? Date.now() - currentExecution.start_time
                          : 0
                    )}
                  </div>
                  <div className="stat-label">Duration</div>
                </div>
              </div>
            </div>
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

      {/* Enhanced Scenario Selection */}
      <div className="scenario-selection-controls">
        <div className="selection-header">
          <h4>📋 Test Scenarios ({filteredScenarios.length})</h4>
          <div className="selection-buttons">
            <button
              className="btn-select-all"
              onClick={selectAllScenarios}
              disabled={isExecutionInProgress}
              title="Select all visible scenarios"
            >
              ✅ Select All
            </button>
            <button
              className="btn-clear-all"
              onClick={clearScenarioSelection}
              disabled={isExecutionInProgress}
              title="Clear all selections"
            >
              🗑️ Clear All
            </button>
          </div>
        </div>
      </div>

      <div className="scenario-selection">{/* Container for scenario list */}

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
              <button
                className="btn-execute-individual"
                onClick={(e) => {
                  e.stopPropagation()
                  handleExecuteIndividualTest(scenario.name)
                }}
                disabled={isExecutionInProgress}
                title={`Execute ${scenario.name} individually`}
              >
                ▶️ Run
              </button>
            </div>
          ))}
        </div>
      </div>

      {/* Enhanced Execution Controls */}
      <div className="execution-controls-enhanced">
        <div className="controls-header">
          <h4>🚀 Test Execution</h4>
          <div className="selected-count-badge">
            <span className="count-number">{selectedScenarios.length}</span>
            <span className="count-label">scenario{selectedScenarios.length !== 1 ? 's' : ''} selected</span>
          </div>
        </div>

        <div className="execution-buttons-grid">
          <button
            className={`btn-execute-primary ${isExecutionInProgress ? 'running' : ''}`}
            onClick={handleExecuteTests}
            disabled={isExecutionInProgress || selectedScenarios.length === 0}
            title={selectedScenarios.length === 0 ? 'Select scenarios to execute' : 'Execute selected scenarios'}
          >
            <div className="btn-content">
              <span className="btn-icon">{isExecutionInProgress ? '🔄' : '▶️'}</span>
              <span className="btn-text">
                {isExecutionInProgress ? 'Running Tests...' : 'Execute Selected'}
              </span>
              {selectedScenarios.length > 0 && !isExecutionInProgress && (
                <span className="btn-count">({selectedScenarios.length})</span>
              )}
            </div>
          </button>

          <button
            className={`btn-execute-suite ${isExecutionInProgress ? 'running' : ''}`}
            onClick={handleExecuteFullSuite}
            disabled={isExecutionInProgress}
            title="Execute entire acceptance test suite"
          >
            <div className="btn-content">
              <span className="btn-icon">{isExecutionInProgress ? '🔄' : '🚀'}</span>
              <span className="btn-text">
                {isExecutionInProgress ? 'Running Suite...' : 'Full Test Suite'}
              </span>
              <span className="btn-count">({getFilteredScenarios().length})</span>
            </div>
          </button>
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
