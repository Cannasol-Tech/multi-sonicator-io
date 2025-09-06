import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
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
  execution_id: 'test-123',
  status: 'passed',
  start_time: 1000,
  end_time: 5000,
  total_scenarios: 2,
  passed_scenarios: 2,
  failed_scenarios: 0,
  current_scenario_index: 2,
  scenarios: [
    {
      name: 'Test Scenario 1',
      description: 'First test scenario',
      feature_file: 'test.feature',
      tags: ['@smoke', '@regression'],
      status: 'passed',
      duration_ms: 2000,
      steps: [
        {
          step_type: 'Given',
          description: 'the system is initialized',
          status: 'passed',
          duration_ms: 500,
          pin_interactions: ['D7', 'D8']
        },
        {
          step_type: 'When',
          description: 'I trigger the test',
          status: 'passed',
          duration_ms: 1000,
          pin_interactions: ['A2']
        },
        {
          step_type: 'Then',
          description: 'the result should be correct',
          status: 'passed',
          duration_ms: 500,
          pin_interactions: []
        }
      ]
    },
    {
      name: 'Test Scenario 2',
      description: 'Second test scenario',
      feature_file: 'test.feature',
      tags: ['@integration'],
      status: 'passed',
      duration_ms: 2000,
      steps: [
        {
          step_type: 'Given',
          description: 'the hardware is connected',
          status: 'passed',
          duration_ms: 800,
          pin_interactions: ['D9']
        },
        {
          step_type: 'When',
          description: 'I run the integration test',
          status: 'passed',
          duration_ms: 1200,
          pin_interactions: ['A1', 'A3']
        }
      ]
    }
  ]
}

describe('TestResultsModal', () => {
  it('should not render when not visible', () => {
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={false}
        onClose={() => {}}
      />
    )
    
    expect(screen.queryByText('Test Results: test-123')).not.toBeInTheDocument()
  })

  it('should not render when execution is null', () => {
    render(
      <TestResultsModal
        execution={null}
        visible={true}
        onClose={() => {}}
      />
    )
    
    expect(screen.queryByText(/Test Results:/)).not.toBeInTheDocument()
  })

  it('should render modal with execution data when visible', () => {
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={() => {}}
      />
    )
    
    // Check header
    expect(screen.getByText(/Test Results:/)).toBeInTheDocument()
    expect(screen.getByText('test-123')).toBeInTheDocument()
    
    // Check summary stats
    expect(screen.getByText('2')).toBeInTheDocument() // passed scenarios
    expect(screen.getByText('0')).toBeInTheDocument() // failed scenarios
    expect(screen.getByText('Passed')).toBeInTheDocument()
    expect(screen.getByText('Failed')).toBeInTheDocument()
    expect(screen.getByText('Total')).toBeInTheDocument()
    expect(screen.getByText('Duration')).toBeInTheDocument()
    
    // Check scenarios
    expect(screen.getByText('Test Scenario 1')).toBeInTheDocument()
    expect(screen.getByText('Test Scenario 2')).toBeInTheDocument()
    
    // Check tags
    expect(screen.getByText('@smoke')).toBeInTheDocument()
    expect(screen.getByText('@regression')).toBeInTheDocument()
    expect(screen.getByText('@integration')).toBeInTheDocument()
  })

  it('should call onClose when close button is clicked', () => {
    const onCloseMock = vi.fn()
    
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={onCloseMock}
      />
    )
    
    const closeButton = screen.getByText('×')
    fireEvent.click(closeButton)
    
    expect(onCloseMock).toHaveBeenCalledTimes(1)
  })

  it('should call onClose when overlay is clicked', () => {
    const onCloseMock = vi.fn()
    
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={onCloseMock}
      />
    )
    
    const overlay = document.querySelector('.test-results-modal-overlay')
    expect(overlay).toBeInTheDocument()
    
    fireEvent.click(overlay!)
    expect(onCloseMock).toHaveBeenCalledTimes(1)
  })

  it('should not call onClose when modal content is clicked', () => {
    const onCloseMock = vi.fn()
    
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={onCloseMock}
      />
    )
    
    const modalContent = document.querySelector('.test-results-modal-content')
    expect(modalContent).toBeInTheDocument()
    
    fireEvent.click(modalContent!)
    expect(onCloseMock).not.toHaveBeenCalled()
  })

  it('should render step details correctly', () => {
    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={() => {}}
      />
    )
    
    // Check step types and descriptions
    expect(screen.getAllByText('Given')).toHaveLength(2)
    expect(screen.getAllByText('When')).toHaveLength(2)
    expect(screen.getByText('Then')).toBeInTheDocument()
    
    expect(screen.getByText('the system is initialized')).toBeInTheDocument()
    expect(screen.getByText('I trigger the test')).toBeInTheDocument()
    expect(screen.getByText('the result should be correct')).toBeInTheDocument()
    
    // Check pin interactions (now displayed with "Pins:" label)
    expect(screen.getByText('Pins:')).toBeInTheDocument()
    expect(screen.getByText('D7, D8')).toBeInTheDocument()
    expect(screen.getByText('A2')).toBeInTheDocument()
  })

  it('should handle export functionality', () => {
    // Mock URL.createObjectURL and related functions
    const mockCreateObjectURL = vi.fn(() => 'mock-url')
    const mockRevokeObjectURL = vi.fn()
    const mockClick = vi.fn()

    Object.defineProperty(URL, 'createObjectURL', {
      value: mockCreateObjectURL,
      writable: true
    })
    Object.defineProperty(URL, 'revokeObjectURL', {
      value: mockRevokeObjectURL,
      writable: true
    })

    // Mock document.createElement
    const mockAnchor = {
      href: '',
      download: '',
      click: mockClick
    }

    const originalCreateElement = document.createElement
    const originalAppendChild = document.body.appendChild
    const originalRemoveChild = document.body.removeChild

    document.createElement = vi.fn((tagName) => {
      if (tagName === 'a') {
        return mockAnchor as any
      }
      return originalCreateElement.call(document, tagName)
    })

    document.body.appendChild = vi.fn()
    document.body.removeChild = vi.fn()

    render(
      <TestResultsModal
        execution={mockExecution}
        visible={true}
        onClose={() => {}}
      />
    )

    const exportButton = screen.getByText('📄 Export Results')
    fireEvent.click(exportButton)

    expect(mockCreateObjectURL).toHaveBeenCalled()
    expect(mockClick).toHaveBeenCalled()
    expect(document.body.appendChild).toHaveBeenCalled()
    expect(document.body.removeChild).toHaveBeenCalled()
    expect(mockRevokeObjectURL).toHaveBeenCalled()

    // Restore original functions
    document.createElement = originalCreateElement
    document.body.appendChild = originalAppendChild
    document.body.removeChild = originalRemoveChild
  })
})
