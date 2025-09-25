import { TestExecution, TestScenario } from '../types'
import { TestAutomationAPI } from '../services/testAutomationApi'

interface TestResultsModalProps {
  execution: TestExecution | null
  visible: boolean
  onClose: () => void
  isLive?: boolean
}

interface TestResultsSectionProps {
  execution: TestExecution | null
  isLive?: boolean
  className?: string
}

// Embedded Test Results Section Component
export function TestResultsSection({ execution, isLive = false, className = '' }: TestResultsSectionProps) {
  if (!execution) {
    return (
      <div className={`test-results-section ${className}`}>
        <div className="test-results-header">
          <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '18px', fontWeight: '600' }}>
            📊 Test Results
          </h3>
        </div>
        <div className="test-results-empty">
          <div className="empty-state">
            <span className="empty-icon">🧪</span>
            <p>No test results available</p>
            <p className="empty-subtitle">Run a test scenario to see results here</p>
          </div>
        </div>
      </div>
    )
  }

  const isExecutionInProgress = execution.status === 'running'
  const isLiveMode = isLive || isExecutionInProgress

  const handleExportResults = () => {
    const results = {
      execution_id: execution.execution_id,
      status: execution.status,
      start_time: execution.start_time,
      end_time: execution.end_time,
      duration_ms: execution.end_time && execution.start_time
        ? execution.end_time - execution.start_time
        : undefined,
      total_scenarios: execution.total_scenarios,
      passed_scenarios: execution.passed_scenarios,
      failed_scenarios: execution.failed_scenarios,
      scenarios: execution.scenarios.map(scenario => ({
        name: scenario.name,
        status: scenario.status,
        duration_ms: scenario.duration_ms,
        error_message: scenario.error_message,
        steps: scenario.steps.map(step => ({
          description: step.description,
          status: step.status,
          duration_ms: step.duration_ms,
          error_message: step.error_message,
          pin_interactions: step.pin_interactions
        }))
      }))
    }

    const blob = new Blob([JSON.stringify(results, null, 2)], { type: 'application/json' })
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a')
    a.href = url
    a.download = `test-results-${execution.execution_id}.json`
    document.body.appendChild(a)
    a.click()
    document.body.removeChild(a)
    URL.revokeObjectURL(url)
  }

  const getScenarioSummary = (scenario: TestScenario) => {
    const passedSteps = scenario.steps.filter(s => s.status === 'passed').length
    const failedSteps = scenario.steps.filter(s => s.status === 'failed').length
    const totalSteps = scenario.steps.length

    return { passedSteps, failedSteps, totalSteps }
  }

  return (
    <div className={`test-results-section ${className}`}>
      <div className="test-results-header">
        <h3 style={{ color: 'var(--text-primary)', margin: 0, fontSize: '18px', fontWeight: '600' }}>
          {TestAutomationAPI.getStatusIcon(execution.status)}
          📊 {isLiveMode ? 'Live Test Progress' : 'Test Results'}: {execution.execution_id}
          {isLiveMode && (
            <span className="live-indicator">
              <span className="live-dot"></span>
              LIVE
            </span>
          )}
        </h3>
        <div className="test-results-actions">
          {!isExecutionInProgress && (
            <button className="test-results-btn-export" onClick={handleExportResults}>
              📄 Export Results
            </button>
          )}
        </div>
      </div>

      <div className="test-results-body">
        {/* Live Progress Indicator */}
        {isLiveMode && (
          <div className="live-progress-section">
            <div className="progress-header">
              <h4>Execution Progress</h4>
              <div className="progress-stats">
                {execution.current_scenario_index !== undefined && (
                  <span className="current-scenario">
                    Scenario {execution.current_scenario_index + 1} of {execution.total_scenarios}
                  </span>
                )}
              </div>
            </div>
            <div className="progress-bar-container">
              <div
                className="progress-bar live"
                style={{
                  width: `${((execution.passed_scenarios + execution.failed_scenarios) / execution.total_scenarios) * 100}%`
                }}
              ></div>
            </div>
          </div>
        )}

        {/* Execution Summary */}
        <div className="execution-summary">
          <div className="summary-stats">
            <div className="stat-card passed">
              <div className="stat-number">{execution.passed_scenarios}</div>
              <div className="stat-label">Passed</div>
            </div>
            <div className="stat-card failed">
              <div className="stat-number">{execution.failed_scenarios}</div>
              <div className="stat-label">Failed</div>
            </div>
            <div className="stat-card total">
              <div className="stat-number">{execution.total_scenarios}</div>
              <div className="stat-label">Total</div>
            </div>
            <div className="stat-card duration">
              <div className="stat-number">
                {TestAutomationAPI.formatDuration(
                  execution.end_time && execution.start_time
                    ? execution.end_time - execution.start_time
                    : 0
                )}
              </div>
              <div className="stat-label">Duration</div>
            </div>
          </div>
        </div>

        {/* Scenario Results */}
        <div className="scenario-results">
          <h4>Scenario Results</h4>
          {execution.scenarios.map((scenario, index) => {
            const summary = getScenarioSummary(scenario)
            const isCurrentScenario = isLiveMode && execution.current_scenario_index === index
            const isPendingScenario = isLiveMode && execution.current_scenario_index !== undefined && index > execution.current_scenario_index

            return (
              <div key={index} className={`scenario-result ${scenario.status} ${isCurrentScenario ? 'current' : ''} ${isPendingScenario ? 'pending' : ''}`}>
                <div className="scenario-header">
                  <div className="scenario-title">
                    <span className="scenario-icon">
                      {isCurrentScenario && scenario.status === 'running' ? '🔄' : TestAutomationAPI.getStatusIcon(scenario.status)}
                    </span>
                    <span className="scenario-name">{scenario.name}</span>
                    <span className="scenario-feature">({scenario.feature_file})</span>
                    {isCurrentScenario && (
                      <span className="current-indicator">RUNNING</span>
                    )}
                    {isPendingScenario && (
                      <span className="pending-indicator">PENDING</span>
                    )}
                  </div>
                  <div className="scenario-meta">
                    <span className="scenario-duration">
                      {scenario.duration_ms ? TestAutomationAPI.formatDuration(scenario.duration_ms) : (isCurrentScenario ? 'In Progress...' : '')}
                    </span>
                    <span className="scenario-step-count">
                      {summary.passedSteps}/{summary.totalSteps} steps
                    </span>
                  </div>
                </div>

                {scenario.error_message && (
                  <div className="scenario-error">
                    <strong>Error:</strong> {scenario.error_message}
                  </div>
                )}

                {scenario.tags.length > 0 && (
                  <div className="scenario-tags">
                    {scenario.tags.map(tag => (
                      <span key={tag} className="tag">@{tag}</span>
                    ))}
                  </div>
                )}

                {/* Step Details */}
                <div className="step-results">
                  {scenario.steps.map((step, stepIndex) => (
                    <div key={stepIndex} className={`step-result ${step.status}`}>
                      <div className="step-header">
                        <div className="step-main-content">
                          <span className="step-icon">
                            {TestAutomationAPI.getStatusIcon(step.status)}
                          </span>
                          <span className="step-type">{step.step_type}</span>
                          <span className="step-description">{step.description}</span>
                          {step.duration_ms && (
                            <span className="step-duration">
                              {TestAutomationAPI.formatDuration(step.duration_ms)}
                            </span>
                          )}
                        </div>

                        {step.pin_interactions.length > 0 && (
                          <div className="step-pins">
                            <span className="step-pins-label">Pins:</span>
                            <span className="step-pins-list">{step.pin_interactions.join(', ')}</span>
                          </div>
                        )}
                      </div>

                      {step.error_message && (
                        <div className="step-error">
                          {step.error_message}
                        </div>
                      )}
                    </div>
                  ))}
                </div>
              </div>
            )
          })}
        </div>
      </div>
    </div>
  )
}

// Original Modal Component (kept for backward compatibility)
export default function TestResultsModal({ execution, visible, onClose, isLive = false }: TestResultsModalProps) {
  if (!visible || !execution) {
    return null
  }

  const isExecutionInProgress = execution.status === 'running'
  const isLiveMode = isLive || isExecutionInProgress

  const handleExportResults = () => {
    const results = {
      execution_id: execution.execution_id,
      status: execution.status,
      start_time: execution.start_time,
      end_time: execution.end_time,
      duration_ms: execution.end_time && execution.start_time 
        ? execution.end_time - execution.start_time 
        : undefined,
      total_scenarios: execution.total_scenarios,
      passed_scenarios: execution.passed_scenarios,
      failed_scenarios: execution.failed_scenarios,
      scenarios: execution.scenarios.map(scenario => ({
        name: scenario.name,
        status: scenario.status,
        duration_ms: scenario.duration_ms,
        error_message: scenario.error_message,
        steps: scenario.steps.map(step => ({
          description: step.description,
          status: step.status,
          duration_ms: step.duration_ms,
          error_message: step.error_message,
          pin_interactions: step.pin_interactions
        }))
      }))
    }

    const blob = new Blob([JSON.stringify(results, null, 2)], { type: 'application/json' })
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a')
    a.href = url
    a.download = `test-results-${execution.execution_id}.json`
    document.body.appendChild(a)
    a.click()
    document.body.removeChild(a)
    URL.revokeObjectURL(url)
  }

  const getScenarioSummary = (scenario: TestScenario) => {
    const passedSteps = scenario.steps.filter(s => s.status === 'passed').length
    const failedSteps = scenario.steps.filter(s => s.status === 'failed').length
    const totalSteps = scenario.steps.length

    return { passedSteps, failedSteps, totalSteps }
  }

  return (
    <div className="test-results-modal-overlay" onClick={onClose}>
      <div className="test-results-modal-content" onClick={(e) => e.stopPropagation()}>
        <div className="test-results-modal-header">
          <h2>
            {TestAutomationAPI.getStatusIcon(execution.status)}
            {isLiveMode ? 'Live Test Progress' : 'Test Results'}: {execution.execution_id}
            {isLiveMode && (
              <span className="live-indicator">
                <span className="live-dot"></span>
                LIVE
              </span>
            )}
          </h2>
          <div className="test-results-modal-actions">
            {!isExecutionInProgress && (
              <button className="test-results-btn-export" onClick={handleExportResults}>
                📄 Export Results
              </button>
            )}
            <button className="test-results-modal-close-button" onClick={onClose}>×</button>
          </div>
        </div>

        <div className="test-results-modal-body">
          {/* Live Progress Indicator */}
          {isLiveMode && (
            <div className="live-progress-section">
              <div className="progress-header">
                <h3>Execution Progress</h3>
                <div className="progress-stats">
                  {execution.current_scenario_index !== undefined && (
                    <span className="current-scenario">
                      Scenario {execution.current_scenario_index + 1} of {execution.total_scenarios}
                    </span>
                  )}
                </div>
              </div>
              <div className="progress-bar-container">
                <div
                  className="progress-bar live"
                  style={{
                    width: `${((execution.passed_scenarios + execution.failed_scenarios) / execution.total_scenarios) * 100}%`
                  }}
                ></div>
              </div>
            </div>
          )}

          {/* Execution Summary */}
          <div className="execution-summary">
            <div className="summary-stats">
              <div className="stat-card passed">
                <div className="stat-number">{execution.passed_scenarios}</div>
                <div className="stat-label">Passed</div>
              </div>
              <div className="stat-card failed">
                <div className="stat-number">{execution.failed_scenarios}</div>
                <div className="stat-label">Failed</div>
              </div>
              <div className="stat-card total">
                <div className="stat-number">{execution.total_scenarios}</div>
                <div className="stat-label">Total</div>
              </div>
              <div className="stat-card duration">
                <div className="stat-number">
                  {TestAutomationAPI.formatDuration(
                    execution.end_time && execution.start_time
                      ? execution.end_time - execution.start_time // Both should now be in milliseconds
                      : 0
                  )}
                </div>
                <div className="stat-label">Duration</div>
              </div>
            </div>
          </div>

          {/* Scenario Results */}
          <div className="scenario-results">
            <h3>Scenario Results</h3>
            {execution.scenarios.map((scenario, index) => {
              const summary = getScenarioSummary(scenario)
              const isCurrentScenario = isLiveMode && execution.current_scenario_index === index
              const isPendingScenario = isLiveMode && execution.current_scenario_index !== undefined && index > execution.current_scenario_index

              return (
                <div key={index} className={`scenario-result ${scenario.status} ${isCurrentScenario ? 'current' : ''} ${isPendingScenario ? 'pending' : ''}`}>
                  <div className="scenario-header">
                    <div className="scenario-title">
                      <span className="scenario-icon">
                        {isCurrentScenario && scenario.status === 'running' ? '🔄' : TestAutomationAPI.getStatusIcon(scenario.status)}
                      </span>
                      <span className="scenario-name">{scenario.name}</span>
                      <span className="scenario-feature">({scenario.feature_file})</span>
                      {isCurrentScenario && (
                        <span className="current-indicator">RUNNING</span>
                      )}
                      {isPendingScenario && (
                        <span className="pending-indicator">PENDING</span>
                      )}
                    </div>
                    <div className="scenario-meta">
                      <span className="scenario-duration">
                        {scenario.duration_ms ? TestAutomationAPI.formatDuration(scenario.duration_ms) : (isCurrentScenario ? 'In Progress...' : '')}
                      </span>
                      <span className="scenario-step-count">
                        {summary.passedSteps}/{summary.totalSteps} steps
                      </span>
                    </div>
                  </div>

                  {scenario.error_message && (
                    <div className="scenario-error">
                      <strong>Error:</strong> {scenario.error_message}
                    </div>
                  )}

                  {scenario.tags.length > 0 && (
                    <div className="scenario-tags">
                      {scenario.tags.map(tag => (
                        <span key={tag} className="tag">@{tag}</span>
                      ))}
                    </div>
                  )}

                  {/* Step Details */}
                  <div className="step-results">
                    {scenario.steps.map((step, stepIndex) => (
                      <div key={stepIndex} className={`step-result ${step.status}`}>
                        <div className="step-header">
                          <div className="step-main-content">
                            <span className="step-icon">
                              {TestAutomationAPI.getStatusIcon(step.status)}
                            </span>
                            <span className="step-type">{step.step_type}</span>
                            <span className="step-description">{step.description}</span>
                            {step.duration_ms && (
                              <span className="step-duration">
                                {TestAutomationAPI.formatDuration(step.duration_ms)}
                              </span>
                            )}
                          </div>

                          {step.pin_interactions.length > 0 && (
                            <div className="step-pins">
                              <span className="step-pins-label">Pins:</span>
                              <span className="step-pins-list">{step.pin_interactions.join(', ')}</span>
                            </div>
                          )}
                        </div>

                        {step.error_message && (
                          <div className="step-error">
                            {step.error_message}
                          </div>
                        )}
                      </div>
                    ))}
                  </div>
                </div>
              )
            })}
          </div>
        </div>
      </div>
    </div>
  )
}
