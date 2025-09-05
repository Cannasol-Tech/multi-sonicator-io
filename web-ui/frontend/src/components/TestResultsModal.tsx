import { TestExecution, TestScenario, TestStep } from '../types'
import { TestAutomationAPI } from '../services/testAutomationApi'

interface TestResultsModalProps {
  execution: TestExecution | null
  visible: boolean
  onClose: () => void
}

export default function TestResultsModal({ execution, visible, onClose }: TestResultsModalProps) {
  if (!visible || !execution) {
    return null
  }

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
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content test-results-modal" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h2>
            {TestAutomationAPI.getStatusIcon(execution.status)} 
            Test Results: {execution.execution_id}
          </h2>
          <div className="modal-actions">
            <button className="btn-export" onClick={handleExportResults}>
              📄 Export Results
            </button>
            <button className="btn-close" onClick={onClose}>×</button>
          </div>
        </div>

        <div className="modal-body">
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
                      : undefined
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
              
              return (
                <div key={index} className={`scenario-result ${scenario.status}`}>
                  <div className="scenario-header">
                    <div className="scenario-title">
                      <span className="scenario-icon">
                        {TestAutomationAPI.getStatusIcon(scenario.status)}
                      </span>
                      <span className="scenario-name">{scenario.name}</span>
                      <span className="scenario-feature">({scenario.feature_file})</span>
                    </div>
                    <div className="scenario-meta">
                      <span className="scenario-duration">
                        {TestAutomationAPI.formatDuration(scenario.duration_ms)}
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

                        {step.error_message && (
                          <div className="step-error">
                            {step.error_message}
                          </div>
                        )}

                        {step.pin_interactions.length > 0 && (
                          <div className="step-pins">
                            <strong>Pin Interactions:</strong> {step.pin_interactions.join(', ')}
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
