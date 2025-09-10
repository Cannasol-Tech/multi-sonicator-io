import { TestExecution } from '../types'
import { TestAutomationAPI } from '../services/testAutomationApi'

interface TestExecutionIndicatorProps {
  execution: TestExecution | null
  visible: boolean
}

export default function TestExecutionIndicator({ execution, visible }: TestExecutionIndicatorProps) {
  if (!visible || !execution) {
    return null
  }

  const progress = TestAutomationAPI.getExecutionProgress(execution)
  const stepProgress = TestAutomationAPI.getCurrentStepProgress(execution)
  const currentScenario = execution.current_scenario_index < execution.scenarios.length 
    ? execution.scenarios[execution.current_scenario_index] 
    : null

  return (
    <div className={`test-execution-indicator ${execution.status}`}>
      <div className="indicator-header">
        <h4 style={{ margin: 0, fontSize: '14px', display: 'flex', alignItems: 'center', gap: '8px' }}>
          {TestAutomationAPI.getStatusIcon(execution.status)}
          Test Execution
        </h4>
        <div style={{ fontSize: '12px', color: '#6c757d' }}>
          {execution.execution_id.split('_')[1]}
        </div>
      </div>

      <div className="indicator-progress" style={{ margin: '8px 0' }}>
        <div style={{
          width: '100%',
          height: '6px',
          background: 'var(--bg-tertiary)',
          borderRadius: '3px',
          overflow: 'hidden'
        }}>
          <div
            className={
              execution.status === 'running' ? 'progress-fill iridescent' :
              execution.status === 'passed' ? 'progress-fill metallic-blue' :
              'progress-fill'
            }
            style={{
              width: `${progress}%`,
              height: '100%',
              background: (execution.status === 'running' || execution.status === 'passed') ? undefined : TestAutomationAPI.getStatusColor(execution.status),
              transition: 'width 0.3s ease'
            }}
          />
        </div>
        <div style={{ 
          fontSize: '11px', 
          color: '#6c757d', 
          marginTop: '4px',
          textAlign: 'center'
        }}>
          {execution.passed_scenarios + execution.failed_scenarios} / {execution.total_scenarios} scenarios
        </div>
      </div>

      {currentScenario && execution.status === 'running' && (
        <div className="current-scenario" style={{ 
          background: '#f8f9fa', 
          padding: '8px', 
          borderRadius: '4px',
          marginBottom: '8px'
        }}>
          <div style={{ fontSize: '12px', fontWeight: '500', marginBottom: '4px' }}>
            Current: {currentScenario.name}
          </div>
          
          {stepProgress.total > 0 && (
            <div style={{ fontSize: '11px', color: 'var(--text-secondary)' }}>
              Step {stepProgress.current} / {stepProgress.total}
              <div style={{
                width: '100%',
                height: '3px',
                background: 'var(--bg-tertiary)',
                borderRadius: '2px',
                marginTop: '2px',
                overflow: 'hidden'
              }}>
                <div
                  className={
                    execution.status === 'running' ? 'step-progress-fill iridescent' :
                    execution.status === 'passed' ? 'step-progress-fill metallic-blue' :
                    'step-progress-fill'
                  }
                  style={{
                    width: `${(stepProgress.current / stepProgress.total) * 100}%`,
                    height: '100%',
                    background: (execution.status === 'running' || execution.status === 'passed') ? undefined : 'var(--color-info)',
                    transition: 'width 0.3s ease'
                  }}
                />
              </div>
            </div>
          )}
        </div>
      )}

      <div className="indicator-stats" style={{ 
        display: 'flex', 
        justifyContent: 'space-between',
        fontSize: '11px'
      }}>
        <span style={{ color: '#28a745' }}>
          ✅ {execution.passed_scenarios}
        </span>
        <span style={{ color: '#dc3545' }}>
          ❌ {execution.failed_scenarios}
        </span>
        <span style={{ color: '#6c757d' }}>
          ⏱️ {TestAutomationAPI.formatDuration(
            execution.end_time 
              ? execution.end_time - (execution.start_time || 0)
              : Date.now() - (execution.start_time || 0)
          )}
        </span>
      </div>

      {execution.status !== 'running' && (
        <div style={{
          marginTop: '8px',
          padding: '6px',
          background: execution.status === 'passed' ? 'rgba(16, 185, 129, 0.15)' : 'rgba(220, 38, 38, 0.15)',
          color: execution.status === 'passed' ? 'var(--color-success)' : 'var(--color-error)',
          borderRadius: '4px',
          fontSize: '12px',
          textAlign: 'center'
        }}>
          {execution.status === 'passed' ? 'All tests passed!' : 
           execution.status === 'failed' ? 'Some tests failed' :
           execution.status === 'error' ? 'Execution error' :
           'Test completed'}
        </div>
      )}
    </div>
  )
}
