import { useState, useEffect } from 'react'
import { useTestAutomation } from '../hooks/useTestAutomation'
import { TestScenario, TestExecution } from '../types'
import TestResultsModal, { TestResultsSection } from './TestResultsModal'
import { ScenarioDetailsModal } from './ScenarioDetailsModal'
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
    getStatusColor,
    getStatusIcon,
    clearError
  } = useTestAutomation()

  const [showResultsModal, setShowResultsModal] = useState(false)
  const [selectedTags, setSelectedTags] = useState<string[]>([])
  const [showDetailsModal, setShowDetailsModal] = useState(false)
  const [selectedScenarioForDetails, setSelectedScenarioForDetails] = useState<TestScenario | null>(null)

  // Get filtered scenarios and execution progress
  const filteredScenarios = getFilteredScenarios()
  const executionProgress = getExecutionProgress()

  // Handle scenario hover for pin highlighting
  const handleScenarioHover = (scenario: TestScenario | null) => {
    if (scenario && onPinHighlight) {
      const pins = scenario.steps.flatMap(step => step.pin_interactions)
      const uniquePins = [...new Set(pins)]
      onPinHighlight(uniquePins)
    } else if (onPinHighlight) {
      onPinHighlight([])
    }
  }

  // Handle tag selection
  const handleTagToggle = (tag: string) => {
    const newSelectedTags = selectedTags.includes(tag)
      ? selectedTags.filter(t => t !== tag)
      : [...selectedTags, tag]
    
    setSelectedTags(newSelectedTags)
    setFilterTags(newSelectedTags)
  }

  // Handle execution
  const handleExecuteSelected = async () => {
    if (selectedScenarios.length === 0) {
      alert('Please select at least one scenario to execute')
      return
    }
    await executeScenarios(selectedScenarios)
    setShowResultsModal(true)
  }

  const handleExecuteFullSuite = async () => {
    const allScenarioNames = filteredScenarios.map(s => s.name)
    await executeScenarios(allScenarioNames)
    setShowResultsModal(true)
  }

  // Handle showing scenario details
  const handleShowScenarioDetails = (scenario: TestScenario) => {
    setSelectedScenarioForDetails(scenario)
    setShowDetailsModal(true)
  }

  // Handle closing scenario details
  const handleCloseScenarioDetails = () => {
    setShowDetailsModal(false)
    setSelectedScenarioForDetails(null)
  }



  // Update test progress callback
  useEffect(() => {
    if (currentExecution && onTestProgress) {
      onTestProgress(currentExecution)
    }
  }, [currentExecution, onTestProgress])

  // Show loading state
  if (loading) {
    return (
      <div className="test-automation-panel">
        <div className="test-automation-loading">
          <div className="loading-spinner">
            <div className="spinner"></div>
            <p style={{ color: 'var(--text-secondary)' }}>Loading test scenarios...</p>
          </div>
        </div>
      </div>
    )
  }

  return (
    <div className="test-automation-panel">
      {/* Header */}
      <div className="test-automation-header">
        <h2 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '20px', fontWeight: '600' }}>
          🧪 Test Automation
        </h2>
        {error && (
          <div className="test-automation-error">
            <span className="error-icon">⚠️</span>
            <span className="error-message" style={{ color: 'var(--color-error)' }}>{error}</span>
            <button className="error-close" onClick={clearError} style={{ color: 'var(--text-tertiary)' }}>×</button>
          </div>
        )}
      </div>

      {/* Two-column layout: Test Controls and Test Results */}
      <div className="test-automation-content">
        {/* Left Column: Test Controls */}
        <div className="test-controls-section">
          {/* Current Execution Progress (if running) */}
          {isExecutionInProgress && currentExecution && (
            <div className="test-automation-section">
              <div className="section-header">
                <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '16px', fontWeight: '600' }}>
                  🔄 Test Execution in Progress
                </h3>
              </div>
              <div className="execution-summary">
                <div className="execution-info">
                  <div className="execution-id" style={{ color: 'var(--text-primary)', fontWeight: '600' }}>
                    {currentExecution.execution_id}
                  </div>
                  <div className="execution-status" style={{ color: getStatusColor(currentExecution.status) }}>
                    {getStatusIcon(currentExecution.status)} {currentExecution.status.toUpperCase()}
                  </div>
                </div>
                <div className="execution-stats">
                  <div className="stat-card">
                    <div className="stat-value" style={{ color: 'var(--color-success)' }}>
                      {currentExecution.passed_scenarios}
                    </div>
                    <div className="stat-label" style={{ color: 'var(--text-tertiary)' }}>Passed</div>
                  </div>
                  <div className="stat-card">
                    <div className="stat-value" style={{ color: 'var(--color-error)' }}>
                      {currentExecution.failed_scenarios}
                    </div>
                    <div className="stat-label" style={{ color: 'var(--text-tertiary)' }}>Failed</div>
                  </div>
                  <div className="stat-card">
                    <div className="stat-value" style={{ color: 'var(--text-primary)' }}>
                      {currentExecution.total_scenarios}
                    </div>
                    <div className="stat-label" style={{ color: 'var(--text-tertiary)' }}>Total</div>
                  </div>
                </div>
                <div className="execution-progress">
                  <IridescentProgressBar
                    progress={executionProgress.percentage}
                    isRunning={true}
                    isComplete={false}
                  />
                  <div className="progress-actions">
                    <button
                      className="btn-stop"
                      onClick={stopExecution}
                      style={{
                        background: 'var(--color-error)',
                        color: 'white',
                        border: 'none',
                        borderRadius: '6px',
                        padding: '8px 16px',
                        fontSize: '14px',
                        fontWeight: '500',
                        cursor: 'pointer',
                        marginTop: '12px'
                      }}
                    >
                      🛑 Stop Execution
                    </button>
                  </div>
                </div>
              </div>
            </div>
          )}

          {/* Tag Filtering Section */}
          <div className="test-automation-section">
            <div className="section-header">
              <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '16px', fontWeight: '600' }}>
                🏷️ Filter by Tags
              </h3>
              <button
                className="btn-clear-tags"
                onClick={() => {
                  setSelectedTags([])
                  setFilterTags([])
                }}
                disabled={selectedTags.length === 0}
                style={{
                  background: selectedTags.length > 0 ? 'var(--bg-tertiary)' : 'transparent',
                  border: '1px solid var(--border-color)',
                  borderRadius: '6px',
                  padding: '4px 8px',
                  fontSize: '12px',
                  color: selectedTags.length > 0 ? 'var(--text-secondary)' : 'var(--text-tertiary)',
                  cursor: selectedTags.length > 0 ? 'pointer' : 'not-allowed'
                }}
              >
                Clear Tags
              </button>
            </div>
            
            {/* Tag Filter Buttons */}
            <div className="tag-filter-container" style={{
              display: 'flex',
              flexWrap: 'wrap',
              gap: '6px',
              marginBottom: '12px'
            }}>
              {availableTags.map((tag) => (
                <button
                  key={tag}
                  className={`tag-filter-btn ${selectedTags.includes(tag) ? 'selected' : ''}`}
                  onClick={() => handleTagToggle(tag)}
                  style={{
                    background: selectedTags.includes(tag)
                      ? 'var(--color-primary)'
                      : 'var(--bg-tertiary)',
                    color: selectedTags.includes(tag)
                      ? 'white'
                      : 'var(--text-secondary)',
                    border: `1px solid ${selectedTags.includes(tag)
                      ? 'var(--color-primary)'
                      : 'var(--border-color)'}`,
                    borderRadius: '20px',
                    padding: '4px 12px',
                    fontSize: '11px',
                    fontWeight: '500',
                    cursor: 'pointer',
                    transition: 'all 0.2s ease',
                    whiteSpace: 'nowrap'
                  }}
                  onMouseEnter={(e) => {
                    if (!selectedTags.includes(tag)) {
                      e.currentTarget.style.background = 'var(--bg-quaternary)'
                      e.currentTarget.style.borderColor = 'var(--color-primary)'
                    }
                  }}
                  onMouseLeave={(e) => {
                    if (!selectedTags.includes(tag)) {
                      e.currentTarget.style.background = 'var(--bg-tertiary)'
                      e.currentTarget.style.borderColor = 'var(--border-color)'
                    }
                  }}
                >
                  @{tag}
                </button>
              ))}
            </div>
            
            {/* Active Filter Summary */}
            {selectedTags.length > 0 && (
              <div className="active-filters-summary" style={{
                padding: '8px 12px',
                background: 'var(--bg-tertiary)',
                borderRadius: '6px',
                border: '1px solid var(--border-color)',
                marginBottom: '12px'
              }}>
                <div style={{
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'space-between'
                }}>
                  <span style={{
                    color: 'var(--text-secondary)',
                    fontSize: '12px'
                  }}>
                    📊 Filtered: {filteredScenarios.length} scenarios
                  </span>
                  <span style={{
                    color: 'var(--color-primary)',
                    fontSize: '12px',
                    fontWeight: '500'
                  }}>
                    Tags: {selectedTags.join(', ')}
                  </span>
                </div>
              </div>
            )}
          </div>

          {/* Test Scenario Controls */}
          <div className="test-automation-section">
            <div className="section-header">
              <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '16px', fontWeight: '600' }}>
                🎯 Test Scenarios
              </h3>
              <div className="section-actions">
                <button
                  className="btn-select-all"
                  onClick={selectAllScenarios}
                  style={{
                    background: 'var(--bg-tertiary)',
                    border: '1px solid var(--border-color)',
                    borderRadius: '6px',
                    padding: '4px 8px',
                    fontSize: '12px',
                    color: 'var(--text-secondary)',
                    cursor: 'pointer',
                    marginRight: '8px'
                  }}
                >
                  Select All
                </button>
                <button
                  className="btn-clear-selection"
                  onClick={clearScenarioSelection}
                  style={{
                    background: 'var(--bg-tertiary)',
                    border: '1px solid var(--border-color)',
                    borderRadius: '6px',
                    padding: '4px 8px',
                    fontSize: '12px',
                    color: 'var(--text-secondary)',
                    cursor: 'pointer'
                  }}
                >
                  Clear
                </button>
              </div>
            </div>

            {/* Compact Scenario List */}
            <div className="scenarios-list-compact">
              {filteredScenarios.map((scenario) => (
                <div
                  key={scenario.name}
                  className={`scenario-card-compact ${selectedScenarios.includes(scenario.name) ? 'selected' : ''}`}
                  onMouseEnter={() => handleScenarioHover(scenario)}
                  onMouseLeave={() => handleScenarioHover(null)}
                  style={{
                    background: selectedScenarios.includes(scenario.name)
                      ? 'var(--bg-tertiary)'
                      : 'var(--bg-secondary)',
                    border: `1px solid ${selectedScenarios.includes(scenario.name)
                      ? 'var(--color-primary)'
                      : 'var(--border-color)'}`,
                    borderRadius: '6px',
                    padding: '8px 12px',
                    marginBottom: '4px',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between',
                    transition: 'all 0.2s ease'
                  }}
                >
                  <div className="scenario-main-content" style={{
                    display: 'flex',
                    alignItems: 'center',
                    flex: 1,
                    cursor: 'pointer'
                  }}
                  onClick={() => toggleScenarioSelection(scenario.name)}
                  >
                    <input
                      type="checkbox"
                      checked={selectedScenarios.includes(scenario.name)}
                      onChange={() => toggleScenarioSelection(scenario.name)}
                      onClick={(e) => e.stopPropagation()}
                      style={{
                        marginRight: '12px',
                        cursor: 'pointer'
                      }}
                    />
                    <div style={{ flex: 1 }}>
                      <div style={{
                        color: 'var(--text-primary)',
                        fontWeight: '500',
                        fontSize: '14px',
                        marginBottom: '2px'
                      }}>
                        {scenario.name}
                      </div>
                      {/* Scenario Tags */}
                      {scenario.tags && scenario.tags.length > 0 && (
                        <div className="scenario-tags" style={{
                          display: 'flex',
                          flexWrap: 'wrap',
                          gap: '4px',
                          marginTop: '4px'
                        }}>
                          {scenario.tags.slice(0, 3).map((tag) => (
                            <span
                              key={tag}
                              className="scenario-tag"
                              style={{
                                background: selectedTags.includes(tag)
                                  ? 'var(--color-primary)'
                                  : 'var(--bg-quaternary)',
                                color: selectedTags.includes(tag)
                                  ? 'white'
                                  : 'var(--text-tertiary)',
                                fontSize: '9px',
                                fontWeight: '500',
                                padding: '2px 6px',
                                borderRadius: '10px',
                                border: selectedTags.includes(tag)
                                  ? '1px solid var(--color-primary)'
                                  : '1px solid var(--border-color)'
                              }}
                            >
                              @{tag}
                            </span>
                          ))}
                          {scenario.tags.length > 3 && (
                            <span
                              style={{
                                color: 'var(--text-tertiary)',
                                fontSize: '9px',
                                fontWeight: '500',
                                padding: '2px 6px'
                              }}
                            >
                              +{scenario.tags.length - 3}
                            </span>
                          )}
                        </div>
                      )}
                    </div>
                  </div>

                  <button
                    className="scenario-details-btn"
                    onClick={(e) => {
                      e.stopPropagation()
                      handleShowScenarioDetails(scenario)
                    }}
                    style={{
                      background: 'var(--color-primary)',
                      color: 'white',
                      border: 'none',
                      borderRadius: '4px',
                      padding: '4px 8px',
                      fontSize: '11px',
                      fontWeight: '500',
                      cursor: 'pointer',
                      transition: 'all 0.2s ease',
                      marginLeft: '8px'
                    }}
                    onMouseEnter={(e) => {
                      e.currentTarget.style.background = 'var(--color-primary-hover)'
                      e.currentTarget.style.transform = 'translateY(-1px)'
                    }}
                    onMouseLeave={(e) => {
                      e.currentTarget.style.background = 'var(--color-primary)'
                      e.currentTarget.style.transform = 'translateY(0)'
                    }}
                  >
                    📋 Details
                  </button>
                </div>
              ))}
            </div>

            {/* Execution Buttons */}
            <div className="execution-buttons">
              <button
                className="btn-execute-selected"
                onClick={handleExecuteSelected}
                disabled={isExecutionInProgress || selectedScenarios.length === 0}
                style={{
                  background: selectedScenarios.length > 0 && !isExecutionInProgress
                    ? 'var(--color-primary)'
                    : 'var(--bg-tertiary)',
                  color: selectedScenarios.length > 0 && !isExecutionInProgress
                    ? 'white'
                    : 'var(--text-tertiary)',
                  border: 'none',
                  borderRadius: '8px',
                  padding: '12px 24px',
                  fontSize: '14px',
                  fontWeight: '600',
                  cursor: selectedScenarios.length > 0 && !isExecutionInProgress ? 'pointer' : 'not-allowed',
                  marginRight: '12px',
                  transition: 'all 0.2s ease'
                }}
              >
                {isExecutionInProgress ? '🔄 Running...' : `▶️ Execute Selected (${selectedScenarios.length})`}
              </button>
              <button
                className="btn-execute-all"
                onClick={handleExecuteFullSuite}
                disabled={isExecutionInProgress || filteredScenarios.length === 0}
                style={{
                  background: !isExecutionInProgress && filteredScenarios.length > 0
                    ? 'var(--color-secondary)'
                    : 'var(--bg-tertiary)',
                  color: !isExecutionInProgress && filteredScenarios.length > 0
                    ? 'white'
                    : 'var(--text-tertiary)',
                  border: 'none',
                  borderRadius: '8px',
                  padding: '12px 24px',
                  fontSize: '14px',
                  fontWeight: '600',
                  cursor: !isExecutionInProgress && filteredScenarios.length > 0 ? 'pointer' : 'not-allowed',
                  transition: 'all 0.2s ease'
                }}
              >
                {isExecutionInProgress ? '🔄 Running...' : `🚀 Execute All (${filteredScenarios.length})`}
              </button>
            </div>
          </div>
        </div>

        {/* Right Column: Test Results (Always Visible) */}
        <div className="test-results-column">
          <TestResultsSection
            execution={currentExecution}
            isLive={isExecutionInProgress}
            className="embedded-results"
          />
        </div>
      </div>

      {/* Keep modal for backward compatibility (can be removed later) */}
      <TestResultsModal
        execution={currentExecution}
        visible={showResultsModal}
        onClose={() => setShowResultsModal(false)}
        isLive={isExecutionInProgress}
      />

      {/* Scenario Details Modal */}
      <ScenarioDetailsModal
        scenario={selectedScenarioForDetails}
        visible={showDetailsModal}
        onClose={handleCloseScenarioDetails}
      />
    </div>
  )
}
