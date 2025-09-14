import { render, screen } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { TestResultsSection } from '../TestResultsModal'
import { TestExecution } from '../../types'

// Mock the TestAutomationAPI
vi.mock('../../services/testAutomationApi', () => ({
  TestAutomationAPI: {
    getStatusIcon: (status: string) => {
      switch (status) {
        case 'passed': return '✅'
        case 'failed': return '❌'
        case 'running': return '🔄'
        default: return '⚪'
      }
    },
    formatDuration: (ms?: number) => {
      if (!ms) return 'N/A'
      return `${ms}ms`
    }
  }
}))

const mockExecution: TestExecution = {
  execution_id: 'test-exec-123',
  status: 'passed',
  start_time: Date.now() - 5000,
  end_time: Date.now(),
  total_scenarios: 2,
  passed_scenarios: 2,
  failed_scenarios: 0,
  scenarios: [
    {
      name: 'Basic Pin Test',
      description: 'Test basic pin operations',
      feature_file: 'pin-control.feature',
      tags: ['basic', 'pins'],
      status: 'passed',
      duration_ms: 2000,
      error_message: null,
      steps: [
        {
          step_type: 'Given',
          description: 'the Arduino is connected',
          status: 'passed',
          duration_ms: 500,
          error_message: null,
          pin_interactions: ['D7', 'D8']
        },
        {
          step_type: 'When',
          description: 'I set pin D7 to HIGH',
          status: 'passed',
          duration_ms: 300,
          error_message: null,
          pin_interactions: ['D7']
        }
      ]
    },
    {
      name: 'PWM Test',
      description: 'Test PWM functionality',
      feature_file: 'pwm.feature',
      tags: ['pwm', 'advanced'],
      status: 'passed',
      duration_ms: 3000,
      error_message: null,
      steps: [
        {
          step_type: 'Given',
          description: 'PWM is configured',
          status: 'passed',
          duration_ms: 800,
          error_message: null,
          pin_interactions: ['D9']
        }
      ]
    }
  ]
}

describe('TestResultsSection Component', () => {
  it('renders empty state when no execution provided', () => {
    render(<TestResultsSection execution={null} />)
    
    expect(screen.getByText('📊 Test Results')).toBeInTheDocument()
    expect(screen.getByText('No test results available')).toBeInTheDocument()
    expect(screen.getByText('Run a test scenario to see results here')).toBeInTheDocument()
  })

  it('renders test execution results correctly', () => {
    render(<TestResultsSection execution={mockExecution} />)
    
    // Check header
    expect(screen.getByText(/Test Results: test-exec-123/)).toBeInTheDocument()
    
    // Check summary stats (multiple "2" values expected)
    expect(screen.getAllByText('2')).toHaveLength(2) // Passed scenarios and Total scenarios
    expect(screen.getByText('0')).toBeInTheDocument() // Failed scenarios
    expect(screen.getByText('Passed')).toBeInTheDocument()
    expect(screen.getByText('Failed')).toBeInTheDocument()
    expect(screen.getByText('Total')).toBeInTheDocument()
    expect(screen.getByText('Duration')).toBeInTheDocument()
  })

  it('renders scenario results', () => {
    render(<TestResultsSection execution={mockExecution} />)
    
    // Check scenario names
    expect(screen.getByText('Basic Pin Test')).toBeInTheDocument()
    expect(screen.getByText('PWM Test')).toBeInTheDocument()
    
    // Check feature files
    expect(screen.getByText('(pin-control.feature)')).toBeInTheDocument()
    expect(screen.getByText('(pwm.feature)')).toBeInTheDocument()
    
    // Check tags
    expect(screen.getByText('@basic')).toBeInTheDocument()
    expect(screen.getByText('@pins')).toBeInTheDocument()
    expect(screen.getByText('@pwm')).toBeInTheDocument()
    expect(screen.getByText('@advanced')).toBeInTheDocument()
  })

  it('renders step details', () => {
    render(<TestResultsSection execution={mockExecution} />)
    
    // Check step descriptions
    expect(screen.getByText('the Arduino is connected')).toBeInTheDocument()
    expect(screen.getByText('I set pin D7 to HIGH')).toBeInTheDocument()
    expect(screen.getByText('PWM is configured')).toBeInTheDocument()
    
    // Check step types (multiple instances expected)
    expect(screen.getAllByText('Given')).toHaveLength(2) // Two "Given" steps in the test data
    expect(screen.getByText('When')).toBeInTheDocument()
    
    // Check pin interactions
    expect(screen.getByText('D7, D8')).toBeInTheDocument()
    expect(screen.getByText('D7')).toBeInTheDocument()
    expect(screen.getByText('D9')).toBeInTheDocument()
  })

  it('shows live indicator when execution is live', () => {
    const liveExecution = { ...mockExecution, status: 'running' }
    render(<TestResultsSection execution={liveExecution} isLive={true} />)
    
    expect(screen.getByText('LIVE')).toBeInTheDocument()
    expect(screen.getByText(/Live Test Progress/)).toBeInTheDocument()
  })

  it('shows export button when execution is complete', () => {
    render(<TestResultsSection execution={mockExecution} />)
    
    const exportButton = screen.getByText('📄 Export Results')
    expect(exportButton).toBeInTheDocument()
    expect(exportButton.tagName).toBe('BUTTON')
  })

  it('hides export button during live execution', () => {
    const liveExecution = { ...mockExecution, status: 'running' }
    render(<TestResultsSection execution={liveExecution} isLive={true} />)
    
    expect(screen.queryByText('📄 Export Results')).not.toBeInTheDocument()
  })

  it('applies custom className', () => {
    const { container } = render(
      <TestResultsSection execution={mockExecution} className="custom-class" />
    )
    
    expect(container.firstChild).toHaveClass('test-results-section')
    expect(container.firstChild).toHaveClass('custom-class')
  })

  it('shows progress bar during live execution', () => {
    const liveExecution = { 
      ...mockExecution, 
      status: 'running',
      current_scenario_index: 0
    }
    render(<TestResultsSection execution={liveExecution} isLive={true} />)
    
    expect(screen.getByText('Execution Progress')).toBeInTheDocument()
    expect(screen.getByText('Scenario 1 of 2')).toBeInTheDocument()
  })
})
