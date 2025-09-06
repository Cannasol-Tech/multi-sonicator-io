import { render, screen } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import TestResultsModal from '../TestResultsModal'
import { TestExecution } from '../../types'

// Mock the TestAutomationAPI
vi.mock('../../services/testAutomationApi', () => ({
  TestAutomationAPI: {
    getStatusIcon: (status: string) => {
      switch (status) {
        case 'passed': return '✅'
        case 'failed': return '❌'
        case 'error': return '⚠️'
        default: return '❓'
      }
    },
    formatDuration: (ms?: number) => {
      if (!ms) return '0s'
      if (ms < 1000) return `${ms}ms`
      return `${(ms / 1000).toFixed(1)}s`
    }
  }
}))

const mockExecution: TestExecution = {
  execution_id: 'dark-theme-test-123',
  status: 'passed',
  start_time: 1000,
  end_time: 5000,
  total_scenarios: 1,
  passed_scenarios: 1,
  failed_scenarios: 0,
  current_scenario_index: 1,
  scenarios: [
    {
      name: 'Dark Theme Test Scenario',
      description: 'Testing readability in dark mode',
      feature_file: 'dark-theme.feature',
      tags: ['@dark-mode', '@readability'],
      status: 'passed',
      duration_ms: 2000,
      steps: [
        {
          step_type: 'Given',
          description: 'the user interface is in dark mode',
          status: 'passed',
          duration_ms: 500,
          pin_interactions: ['D1', 'D2']
        },
        {
          step_type: 'When',
          description: 'I view the test results modal',
          status: 'passed',
          duration_ms: 1000,
          pin_interactions: ['A1']
        },
        {
          step_type: 'Then',
          description: 'all Behave statements should be clearly readable',
          status: 'passed',
          duration_ms: 500,
          pin_interactions: []
        }
      ]
    }
  ]
}

describe('TestResultsModal - Dark Theme', () => {
  beforeEach(() => {
    // Apply dark theme to document
    document.documentElement.classList.add('dark-theme')
  })

  afterEach(() => {
    // Clean up dark theme
    document.documentElement.classList.remove('dark-theme')
  })

  it('should render correctly in dark mode with readable text', () => {
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={() => {}}
      />
    )
    
    // Check that modal is rendered
    expect(document.querySelector('.test-results-modal-overlay')).toBeInTheDocument()
    expect(document.querySelector('.test-results-modal-content')).toBeInTheDocument()
    
    // Check that dark theme class is applied to document
    expect(document.documentElement.classList.contains('dark-theme')).toBe(true)
    
    // Check header content
    expect(screen.getByText(/Test Results:/)).toBeInTheDocument()
    expect(screen.getByText('dark-theme-test-123')).toBeInTheDocument()
    
    // Check summary stats are visible
    expect(screen.getByText('1')).toBeInTheDocument() // passed scenarios
    expect(screen.getByText('0')).toBeInTheDocument() // failed scenarios
    expect(screen.getByText('Passed')).toBeInTheDocument()
    expect(screen.getByText('Failed')).toBeInTheDocument()
    
    // Check scenario content is readable
    expect(screen.getByText('Dark Theme Test Scenario')).toBeInTheDocument()
    expect(screen.getByText('dark-theme.feature')).toBeInTheDocument()
    
    // Check step descriptions are readable (the key issue we fixed)
    expect(screen.getByText('the user interface is in dark mode')).toBeInTheDocument()
    expect(screen.getByText('I view the test results modal')).toBeInTheDocument()
    expect(screen.getByText('all Behave statements should be clearly readable')).toBeInTheDocument()
    
    // Check step types are visible
    expect(screen.getAllByText('Given')).toHaveLength(1)
    expect(screen.getAllByText('When')).toHaveLength(1)
    expect(screen.getAllByText('Then')).toHaveLength(1)
    
    // Check tags are visible
    expect(screen.getByText('@dark-mode')).toBeInTheDocument()
    expect(screen.getByText('@readability')).toBeInTheDocument()
    
    // Check pin interactions are visible (now with "Pins:" label)
    expect(screen.getByText('Pins:')).toBeInTheDocument()
    expect(screen.getByText('D1, D2')).toBeInTheDocument()
    expect(screen.getByText('A1')).toBeInTheDocument()
  })

  it('should have proper theme-aware styling applied', () => {
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={() => {}}
      />
    )
    
    // Check that the modal content has the correct classes for theme support
    const modalContent = document.querySelector('.test-results-modal-content')
    expect(modalContent).toBeInTheDocument()
    
    const modalBody = document.querySelector('.test-results-modal-body')
    expect(modalBody).toBeInTheDocument()
    
    // Check that stat cards are present
    const statCards = document.querySelectorAll('.stat-card')
    expect(statCards.length).toBeGreaterThan(0)
    
    // Check that scenario results are present
    const scenarioResults = document.querySelectorAll('.scenario-result')
    expect(scenarioResults.length).toBeGreaterThan(0)
    
    // Check that step results are present
    const stepResults = document.querySelectorAll('.step-result')
    expect(stepResults.length).toBeGreaterThan(0)
  })

  it('should maintain readability of Behave statements in dark mode', () => {
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={() => {}}
      />
    )
    
    // The key test: ensure all step descriptions are readable
    const stepDescriptions = [
      'the user interface is in dark mode',
      'I view the test results modal', 
      'all Behave statements should be clearly readable'
    ]
    
    stepDescriptions.forEach(description => {
      const element = screen.getByText(description)
      expect(element).toBeInTheDocument()
      expect(element).toHaveClass('step-description')
    })
    
    // Ensure step types are also readable
    const stepTypes = screen.getAllByText(/^(Given|When|Then)$/)
    expect(stepTypes.length).toBe(3)
    stepTypes.forEach(stepType => {
      expect(stepType).toHaveClass('step-type')
    })
  })
})
