import { useState, useEffect } from 'react'
import { useTestAutomation } from '../hooks/useTestAutomation'
import { TestScenario, TestExecution } from '../types'
import TestResultsModal, { TestResultsSection } from './TestResultsModal'
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

  const [showResultsModal, setShowResultsModal] = useState(false)
  const [tagSearchQuery, setTagSearchQuery] = useState('')
  const [selectedTags, setSelectedTags] = useState<string[]>([])
  const [activeSection, setActiveSection] = useState<'scenarios' | 'tags'>('scenarios')

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
                    isActive={true}
                    showPercentage={true}
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

            {/* Scenario List */}
            <div className="scenarios-list">
              {filteredScenarios.map((scenario) => (
                <div
                  key={scenario.name}
                  className={`scenario-card ${selectedScenarios.includes(scenario.name) ? 'selected' : ''}`}
                  onMouseEnter={() => handleScenarioHover(scenario)}
                  onMouseLeave={() => handleScenarioHover(null)}
                  style={{
                    background: selectedScenarios.includes(scenario.name)
                      ? 'var(--bg-tertiary)'
                      : 'var(--bg-secondary)',
                    border: `1px solid ${selectedScenarios.includes(scenario.name)
                      ? 'var(--color-primary)'
                      : 'var(--border-color)'}`,
                    borderRadius: '8px',
                    padding: '12px',
                    marginBottom: '8px',
                    cursor: 'pointer',
                    transition: 'all 0.2s ease'
                  }}
                  onClick={() => toggleScenarioSelection(scenario.name)}
                >
                  <div className="scenario-header">
                    <div className="scenario-title">
                      <input
                        type="checkbox"
                        checked={selectedScenarios.includes(scenario.name)}
                        onChange={() => toggleScenarioSelection(scenario.name)}
                        style={{ marginRight: '8px' }}
                      />
                      <span style={{ color: 'var(--text-primary)', fontWeight: '600' }}>
                        {scenario.name}
                      </span>
                    </div>
                  </div>
                  <div className="scenario-description" style={{
                    color: 'var(--text-secondary)',
                    fontSize: '14px',
                    marginTop: '4px',
                    marginLeft: '24px'
                  }}>
                    {scenario.description}
                  </div>
                  <div className="scenario-meta" style={{
                    color: 'var(--text-tertiary)',
                    fontSize: '12px',
                    marginTop: '8px',
                    marginLeft: '24px'
                  }}>
                    <span>{scenario.feature_file}</span>
                    <span style={{ margin: '0 8px' }}>•</span>
                    <span>{scenario.steps.length} steps</span>
                    {scenario.tags.length > 0 && (
                      <>
                        <span style={{ margin: '0 8px' }}>•</span>
                        <span>{scenario.tags.map(tag => `@${tag}`).join(', ')}</span>
                      </>
                    )}
                  </div>
                </div>
              ))}
            </div>

            {/* Execution Buttons */}
            <div className="execution-buttons">
              <button
                className="btn-execute-selected"
                onClick={handleExecuteTests}
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
    </div>
  )
}
          <div className="section-header">
            <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '16px', fontWeight: '600' }}>
              📊 Previous Test Execution
            </h3>
            <button
              className="btn-view-results"
              onClick={() => setShowResultsModal(true)}
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
              View Details
            </button>
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
            {isExecutionInProgress && (
              <div className="execution-progress">
                <IridescentProgressBar 
                  progress={executionProgress.percentage}
                  isActive={true}
                  showPercentage={true}
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
                      padding: '6px 12px',
                      fontSize: '12px',
                      cursor: 'pointer'
                    }}
                  >
                    ⏹️ Stop Execution
                  </button>
                </div>
              </div>
            )}
          </div>
        </div>
      )}

      {/* Section Navigation */}
      <div className="test-automation-nav">
        <button
          className={`nav-button ${activeSection === 'scenarios' ? 'active' : ''}`}
          onClick={() => setActiveSection('scenarios')}
          style={{
            background: activeSection === 'scenarios' ? 'var(--color-primary)' : 'var(--bg-secondary)',
            color: activeSection === 'scenarios' ? 'white' : 'var(--text-secondary)',
            border: '1px solid var(--border-color)',
            borderRadius: '6px 0 0 6px',
            padding: '8px 16px',
            fontSize: '14px',
            cursor: 'pointer'
          }}
        >
          📋 Test Scenarios ({filteredScenarios.length})
        </button>
        <button
          className={`nav-button ${activeSection === 'tags' ? 'active' : ''}`}
          onClick={() => setActiveSection('tags')}
          style={{
            background: activeSection === 'tags' ? 'var(--color-primary)' : 'var(--bg-secondary)',
            color: activeSection === 'tags' ? 'white' : 'var(--text-secondary)',
            border: '1px solid var(--border-color)',
            borderRadius: '0 6px 6px 0',
            padding: '8px 16px',
            fontSize: '14px',
            cursor: 'pointer'
          }}
        >
          🏷️ Filter by Tags ({selectedTags.length})
        </button>
      </div>

      {/* Content Sections */}
      {activeSection === 'scenarios' && (
        <div className="test-automation-section">
          <div className="section-header">
            <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '16px', fontWeight: '600' }}>
              Test Scenarios
            </h3>
            <div className="scenario-actions">
              <button
                onClick={selectAllScenarios}
                disabled={isExecutionInProgress}
                style={{
                  background: 'var(--bg-tertiary)',
                  border: '1px solid var(--border-color)',
                  borderRadius: '4px',
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
                onClick={clearScenarioSelection}
                disabled={isExecutionInProgress}
                style={{
                  background: 'var(--bg-tertiary)',
                  border: '1px solid var(--border-color)',
                  borderRadius: '4px',
                  padding: '4px 8px',
                  fontSize: '12px',
                  color: 'var(--text-secondary)',
                  cursor: 'pointer'
                }}
              >
                Clear All
              </button>
            </div>
          </div>

          <div className="scenario-list">
            {filteredScenarios.map(scenario => (
              <div 
                key={scenario.name}
                className="scenario-item"
                onMouseEnter={() => handleScenarioHover(scenario)}
                onMouseLeave={() => handleScenarioHover(null)}
                style={{
                  background: selectedScenarios.includes(scenario.name) ? 'var(--bg-tertiary)' : 'var(--bg-secondary)',
                  border: `1px solid ${selectedScenarios.includes(scenario.name) ? 'var(--color-primary)' : 'var(--border-color)'}`,
                  borderRadius: '8px',
                  padding: '12px',
                  marginBottom: '8px',
                  cursor: 'pointer',
                  transition: 'all 0.2s ease'
                }}
                onClick={() => toggleScenarioSelection(scenario.name)}
              >
                <div className="scenario-header">
                  <div className="scenario-checkbox">
                    <input
                      type="checkbox"
                      checked={selectedScenarios.includes(scenario.name)}
                      onChange={() => toggleScenarioSelection(scenario.name)}
                      disabled={isExecutionInProgress}
                      style={{ marginRight: '8px' }}
                    />
                    <div className="scenario-name" style={{ color: 'var(--text-primary)', fontWeight: '600' }}>
                      {scenario.name}
                    </div>
                  </div>
                  <div className="scenario-status" style={{ color: getStatusColor(scenario.status) }}>
                    {getStatusIcon(scenario.status)}
                  </div>
                </div>
                <div className="scenario-description" style={{ color: 'var(--text-secondary)', fontSize: '14px', marginTop: '4px' }}>
                  {scenario.description}
                </div>
                <div className="scenario-meta" style={{ marginTop: '8px', display: 'flex', alignItems: 'center', gap: '12px' }}>
                  <span className="feature-file" style={{ color: 'var(--text-tertiary)', fontSize: '12px' }}>
                    📄 {scenario.feature_file}
                  </span>
                  <div className="scenario-tags">
                    {scenario.tags.map(tag => (
                      <span
                        key={tag}
                        className="tag-badge"
                        style={{
                          background: 'var(--bg-primary)',
                          border: '1px solid var(--border-color)',
                          borderRadius: '4px',
                          padding: '2px 6px',
                          fontSize: '10px',
                          color: 'var(--text-tertiary)',
                          marginRight: '4px'
                        }}
                      >
                        {tag}
                      </span>
                    ))}
                  </div>
                </div>
              </div>
            ))}
          </div>

          {/* Execution Controls */}
          <div className="execution-controls">
            <button
              className="btn-execute-selected"
              onClick={handleExecuteSelected}
              disabled={isExecutionInProgress || selectedScenarios.length === 0}
              style={{
                background: selectedScenarios.length > 0 ? 'var(--color-primary)' : 'var(--bg-tertiary)',
                color: selectedScenarios.length > 0 ? 'white' : 'var(--text-tertiary)',
                border: 'none',
                borderRadius: '8px',
                padding: '12px 16px',
                fontSize: '14px',
                fontWeight: '600',
                cursor: selectedScenarios.length > 0 ? 'pointer' : 'not-allowed',
                marginRight: '8px',
                flex: 1
              }}
            >
              🚀 Execute Selected ({selectedScenarios.length})
            </button>
            <button
              className="btn-execute-all"
              onClick={handleExecuteFullSuite}
              disabled={isExecutionInProgress || filteredScenarios.length === 0}
              style={{
                background: 'var(--color-success)',
                color: 'white',
                border: 'none',
                borderRadius: '8px',
                padding: '12px 16px',
                fontSize: '14px',
                fontWeight: '600',
                cursor: 'pointer',
                flex: 1
              }}
            >
              🎯 Full Suite ({filteredScenarios.length})
            </button>
          </div>
        </div>
      )}

      {activeSection === 'tags' && (
        <div className="test-automation-section">
          <div className="section-header">
            <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '16px', fontWeight: '600' }}>
              Filter by Tags
            </h3>
            <div className="tag-actions">
              <button
                onClick={() => {
                  setSelectedTags([])
                  setFilterTags([])
                }}
                style={{
                  background: 'var(--bg-tertiary)',
                  border: '1px solid var(--border-color)',
                  borderRadius: '4px',
                  padding: '4px 8px',
                  fontSize: '12px',
                  color: 'var(--text-secondary)',
                  cursor: 'pointer'
                }}
              >
                Clear Filters
              </button>
            </div>
          </div>

          {/* Tag Search */}
          <div className="tag-search">
            <input
              type="text"
              placeholder="Search tags..."
              value={tagSearchQuery}
              onChange={(e) => setTagSearchQuery(e.target.value)}
              style={{
                width: '100%',
                padding: '8px 12px',
                border: '1px solid var(--border-color)',
                borderRadius: '6px',
                background: 'var(--bg-primary)',
                color: 'var(--text-primary)',
                fontSize: '14px',
                marginBottom: '16px'
              }}
            />
          </div>

          {/* Selected Tags Summary */}
          {selectedTags.length > 0 && (
            <div className="selected-tags-summary" style={{ marginBottom: '16px' }}>
              <div style={{ color: 'var(--text-secondary)', fontSize: '14px', marginBottom: '8px' }}>
                Active Filters ({selectedTags.length}):
              </div>
              <div className="selected-tags">
                {selectedTags.map(tag => (
                  <span
                    key={tag}
                    className="selected-tag"
                    style={{
                      background: 'var(--color-primary)',
                      color: 'white',
                      borderRadius: '4px',
                      padding: '4px 8px',
                      fontSize: '12px',
                      marginRight: '8px',
                      marginBottom: '4px',
                      display: 'inline-flex',
                      alignItems: 'center',
                      gap: '4px'
                    }}
                  >
                    {tag}
                    <button
                      onClick={() => handleTagToggle(tag)}
                      style={{
                        background: 'none',
                        border: 'none',
                        color: 'white',
                        cursor: 'pointer',
                        padding: '0',
                        fontSize: '12px'
                      }}
                    >
                      ×
                    </button>
                  </span>
                ))}
              </div>
            </div>
          )}

          {/* Available Tags */}
          <div className="available-tags">
            <div style={{ color: 'var(--text-secondary)', fontSize: '14px', marginBottom: '8px' }}>
              Available Tags:
            </div>
            <div className="tags-grid">
              {availableTags
                .filter(tag => tag.toLowerCase().includes(tagSearchQuery.toLowerCase()))
                .map(tag => {
                  const isSelected = selectedTags.includes(tag)
                  const scenarioCount = filteredScenarios.filter(s => s.tags.includes(tag)).length

                  return (
                    <div
                      key={tag}
                      className="tag-item"
                      onClick={() => handleTagToggle(tag)}
                      style={{
                        background: isSelected ? 'var(--color-primary)' : 'var(--bg-secondary)',
                        color: isSelected ? 'white' : 'var(--text-primary)',
                        border: `1px solid ${isSelected ? 'var(--color-primary)' : 'var(--border-color)'}`,
                        borderRadius: '8px',
                        padding: '12px',
                        cursor: 'pointer',
                        transition: 'all 0.2s ease',
                        marginBottom: '8px'
                      }}
                    >
                      <div className="tag-header">
                        <div className="tag-name" style={{ fontWeight: '600', fontSize: '14px' }}>
                          🏷️ {tag}
                        </div>
                        <div className="tag-count" style={{
                          fontSize: '12px',
                          opacity: 0.8,
                          color: isSelected ? 'rgba(255,255,255,0.8)' : 'var(--text-tertiary)'
                        }}>
                          {scenarioCount} scenario{scenarioCount !== 1 ? 's' : ''}
                        </div>
                      </div>
                    </div>
                  )
                })}
            </div>
          </div>

          {/* Filtered Results Summary */}
          {selectedTags.length > 0 && (
            <div className="filter-results" style={{
              background: 'var(--bg-tertiary)',
              border: '1px solid var(--border-color)',
              borderRadius: '8px',
              padding: '12px',
              marginTop: '16px'
            }}>
              <div style={{ color: 'var(--text-primary)', fontWeight: '600', marginBottom: '4px' }}>
                Filter Results:
              </div>
              <div style={{ color: 'var(--text-secondary)', fontSize: '14px' }}>
                {filteredScenarios.length} scenario{filteredScenarios.length !== 1 ? 's' : ''} match{filteredScenarios.length === 1 ? 'es' : ''} the selected tag{selectedTags.length !== 1 ? 's' : ''}
              </div>
            </div>
          )}
        </div>
      )}

      <TestResultsModal
        execution={currentExecution}
        visible={showResultsModal}
        onClose={() => setShowResultsModal(false)}
        isLive={isExecutionInProgress}
      />
    </div>
  )
}
