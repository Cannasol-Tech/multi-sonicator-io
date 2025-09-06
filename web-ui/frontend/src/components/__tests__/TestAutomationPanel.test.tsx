import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import React from 'react'
import TestAutomationPanel from '../TestAutomationPanel'

// Mock test automation API
vi.mock('../../services/testAutomationApi', () => ({
  TestAutomationAPI: {
    getScenarios: vi.fn(),
    executeScenarios: vi.fn(),
    stopExecution: vi.fn(),
    getExecutionResults: vi.fn(),
    getStatusIcon: vi.fn(() => '✅'),
    formatDuration: vi.fn((ms) => `${ms}ms`)
  }
}))

// Mock useTestAutomation hook
const mockUseTestAutomation = {
  scenarios: [],
  isLoading: false,
  error: null,
  selectedScenarios: [],
  isExecutionInProgress: false,
  currentExecution: null,
  executionProgress: { completed: 0, total: 0, percentage: 0 },
  currentStepProgress: null,
  loadScenarios: vi.fn(),
  selectScenario: vi.fn(),
  deselectScenario: vi.fn(),
  selectAllScenarios: vi.fn(),
  clearSelectedScenarios: vi.fn(),
  executeSelectedScenarios: vi.fn(),
  stopExecution: vi.fn(),
  getFilteredScenarios: vi.fn(() => []),
  getExecutionProgress: vi.fn(() => ({ completed: 0, total: 0, percentage: 0 })),
  getCurrentStepProgress: vi.fn(() => null),
  clearError: vi.fn(),
  getStatusIcon: vi.fn(() => '✅'),
  formatDuration: vi.fn((ms) => `${ms}ms`)
}

vi.mock('../../hooks/useTestAutomation', () => ({
  useTestAutomation: () => mockUseTestAutomation
}))

// Mock scenarios data
const mockScenarios = [
  {
    id: 'scenario-1',
    name: 'Basic Pin Test',
    description: 'Test basic pin operations',
    feature: 'pin-control.feature',
    tags: ['@smoke', '@pin'],
    steps: [
      { description: 'Given the Arduino is connected', pin_interactions: ['D7', 'D8'] },
      { description: 'When I set pin D8 to HIGH', pin_interactions: ['D8'] },
      { description: 'Then pin D8 should be HIGH', pin_interactions: [] }
    ]
  },
  {
    id: 'scenario-2',
    name: 'PWM Test',
    description: 'Test PWM functionality',
    feature: 'pwm-control.feature',
    tags: ['@pwm'],
    steps: [
      { description: 'Given the Arduino is connected', pin_interactions: [] },
      { description: 'When I set pin D9 to PWM=128', pin_interactions: ['D9'] },
      { description: 'Then pin D9 should be PWM=128', pin_interactions: ['D9'] }
    ]
  }
]

describe('TestAutomationPanel Component', () => {
  const mockProps = {
    connected: true,
    onPinHighlight: vi.fn(),
  }

  beforeEach(() => {
    vi.clearAllMocks()
    // Reset mock hook state
    mockUseTestAutomation.scenarios = mockScenarios
    mockUseTestAutomation.isLoading = false
    mockUseTestAutomation.error = null
    mockUseTestAutomation.selectedScenarios = []
    mockUseTestAutomation.isExecutionInProgress = false
    mockUseTestAutomation.currentExecution = null
    mockUseTestAutomation.getFilteredScenarios.mockReturnValue(mockScenarios)
  })

  describe('Rendering', () => {
    it('renders the test automation panel', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        expect(screen.getByText(/Test Automation/i)).toBeInTheDocument()
      })
    })

    it('loads and displays scenarios', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        expect(screen.getByText('Basic Pin Test')).toBeInTheDocument()
        expect(screen.getByText('PWM Test')).toBeInTheDocument()
      })
    })

    it('shows scenario details', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        expect(screen.getByText('Test basic pin operations')).toBeInTheDocument()
        expect(screen.getByText('pin-control.feature')).toBeInTheDocument()
      })
    })

    it('displays scenario tags', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        expect(screen.getByText('@smoke')).toBeInTheDocument()
        expect(screen.getByText('@pin')).toBeInTheDocument()
        expect(screen.getByText('@pwm')).toBeInTheDocument()
      })
    })
  })

  describe('Scenario Selection', () => {
    it('allows selecting individual scenarios', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        expect(checkbox).toBeChecked()
      })
    })

    it('allows selecting all scenarios', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const selectAllButton = screen.getByText(/Select All/i)
        fireEvent.click(selectAllButton)
        
        const checkboxes = screen.getAllByRole('checkbox')
        checkboxes.forEach(checkbox => {
          expect(checkbox).toBeChecked()
        })
      })
    })

    it('allows clearing all selections', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        // First select all
        const selectAllButton = screen.getByText(/Select All/i)
        fireEvent.click(selectAllButton)
        
        // Then clear all
        const clearButton = screen.getByText(/Clear/i)
        fireEvent.click(clearButton)
        
        const checkboxes = screen.getAllByRole('checkbox')
        checkboxes.forEach(checkbox => {
          expect(checkbox).not.toBeChecked()
        })
      })
    })

    it('shows selected count', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        expect(screen.getByText(/1 selected/i)).toBeInTheDocument()
      })
    })
  })

  describe('Test Execution', () => {
    it('starts test execution when run button is clicked', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
        
        expect(mockTestAutomationAPI.executeScenarios).toHaveBeenCalledWith(['scenario-1'])
      })
    })

    it('disables run button when no scenarios selected', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const runButton = screen.getByText(/Run Selected/i)
        expect(runButton).toBeDisabled()
      })
    })

    it('shows execution progress', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
      })
      
      await waitFor(() => {
        expect(screen.getByText(/Running/i)).toBeInTheDocument()
      })
    })

    it('allows stopping execution', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
      })
      
      await waitFor(() => {
        const stopButton = screen.getByText(/Stop/i)
        fireEvent.click(stopButton)
        
        expect(mockTestAutomationAPI.stopExecution).toHaveBeenCalled()
      })
    })
  })

  describe('Filtering and Search', () => {
    it('filters scenarios by tag', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const smokeTag = screen.getByText('@smoke')
        fireEvent.click(smokeTag)
        
        expect(screen.getByText('Basic Pin Test')).toBeInTheDocument()
        expect(screen.queryByText('PWM Test')).not.toBeInTheDocument()
      })
    })

    it('searches scenarios by name', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const searchInput = screen.getByPlaceholderText(/Search scenarios/i)
        fireEvent.change(searchInput, { target: { value: 'PWM' } })
        
        expect(screen.queryByText('Basic Pin Test')).not.toBeInTheDocument()
        expect(screen.getByText('PWM Test')).toBeInTheDocument()
      })
    })

    it('clears search when search input is cleared', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const searchInput = screen.getByPlaceholderText(/Search scenarios/i)
        fireEvent.change(searchInput, { target: { value: 'PWM' } })
        fireEvent.change(searchInput, { target: { value: '' } })
        
        expect(screen.getByText('Basic Pin Test')).toBeInTheDocument()
        expect(screen.getByText('PWM Test')).toBeInTheDocument()
      })
    })
  })

  describe('Results Display', () => {
    it('shows test results after execution', async () => {
      const mockResults = {
        status: 'completed',
        results: [
          {
            scenarioId: 'scenario-1',
            status: 'passed',
            duration: 1500,
            steps: [
              { description: 'Given the Arduino is connected', status: 'passed', duration: 500 },
              { description: 'When I set pin D8 to HIGH', status: 'passed', duration: 500 },
              { description: 'Then pin D8 should be HIGH', status: 'passed', duration: 500 }
            ]
          }
        ]
      }
      
      mockTestAutomationAPI.getExecutionResults.mockResolvedValue(mockResults)
      
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
      })
      
      await waitFor(() => {
        expect(screen.getByText(/Results/i)).toBeInTheDocument()
        expect(screen.getByText(/passed/i)).toBeInTheDocument()
      })
    })

    it('shows execution statistics', async () => {
      const mockResults = {
        status: 'completed',
        results: [
          { scenarioId: 'scenario-1', status: 'passed', duration: 1500 },
          { scenarioId: 'scenario-2', status: 'failed', duration: 800 }
        ]
      }
      
      mockTestAutomationAPI.getExecutionResults.mockResolvedValue(mockResults)
      
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const selectAllButton = screen.getByText(/Select All/i)
        fireEvent.click(selectAllButton)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
      })
      
      await waitFor(() => {
        expect(screen.getByText(/1 passed/i)).toBeInTheDocument()
        expect(screen.getByText(/1 failed/i)).toBeInTheDocument()
      })
    })

    it('allows exporting results', async () => {
      const mockResults = {
        status: 'completed',
        results: [
          { scenarioId: 'scenario-1', status: 'passed', duration: 1500 }
        ]
      }
      
      mockTestAutomationAPI.getExecutionResults.mockResolvedValue(mockResults)
      
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
      })
      
      await waitFor(() => {
        const exportButton = screen.getByText(/Export/i)
        expect(exportButton).toBeInTheDocument()
      })
    })
  })

  describe('Pin Highlighting Integration', () => {
    it('highlights pins during test execution', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
      })
      
      // Should call onPinHighlight during execution
      await waitFor(() => {
        expect(mockProps.onPinHighlight).toHaveBeenCalled()
      })
    })
  })

  describe('Connection Status Handling', () => {
    it('disables controls when disconnected', () => {
      render(<TestAutomationPanel {...mockProps} connected={false} />)
      
      const runButton = screen.getByText(/Run Selected/i)
      expect(runButton).toBeDisabled()
    })

    it('shows disconnected warning', () => {
      render(<TestAutomationPanel {...mockProps} connected={false} />)
      
      expect(screen.getByText(/Arduino not connected/i)).toBeInTheDocument()
    })
  })

  describe('Error Handling', () => {
    it('handles scenario loading errors', async () => {
      mockTestAutomationAPI.getScenarios.mockRejectedValue(new Error('Failed to load'))
      
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        expect(screen.getByText(/Failed to load scenarios/i)).toBeInTheDocument()
      })
    })

    it('handles execution errors', async () => {
      mockTestAutomationAPI.executeScenarios.mockRejectedValue(new Error('Execution failed'))
      
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        fireEvent.click(checkbox)
        
        const runButton = screen.getByText(/Run Selected/i)
        fireEvent.click(runButton)
      })
      
      await waitFor(() => {
        expect(screen.getByText(/Execution failed/i)).toBeInTheDocument()
      })
    })
  })

  describe('Accessibility', () => {
    it('has proper labels for all controls', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        expect(screen.getByLabelText(/Search scenarios/i)).toBeInTheDocument()
        expect(screen.getByLabelText(/Basic Pin Test/i)).toBeInTheDocument()
      })
    })

    it('supports keyboard navigation', async () => {
      render(<TestAutomationPanel {...mockProps} />)
      
      await waitFor(() => {
        const checkbox = screen.getByLabelText(/Basic Pin Test/i)
        checkbox.focus()
        expect(document.activeElement).toBe(checkbox)
      })
    })
  })

  describe('Button Styling', () => {
    it('renders Select All and Clear All buttons with correct classes', async () => {
      render(<TestAutomationPanel {...mockProps} />)

      await waitFor(() => {
        const selectAllButton = screen.getByText(/Select All/i)
        const clearAllButton = screen.getByText(/Clear All/i)

        expect(selectAllButton).toBeInTheDocument()
        expect(clearAllButton).toBeInTheDocument()

        // Verify they have the correct CSS classes for theme styling
        expect(selectAllButton).toHaveClass('btn-select-all')
        expect(clearAllButton).toHaveClass('btn-clear-all')
      })
    })
  })

  describe('Live Test Results', () => {
    it('shows live progress button when execution is in progress', async () => {
      // Mock execution in progress with proper step progress
      mockUseTestAutomation.isExecutionInProgress = true
      mockUseTestAutomation.currentExecution = {
        execution_id: 'live-test-123',
        status: 'running',
        total_scenarios: 3,
        passed_scenarios: 1,
        failed_scenarios: 0
      }
      mockUseTestAutomation.getCurrentStepProgress.mockReturnValue({
        current: 2,
        total: 5
      })

      render(<TestAutomationPanel {...mockProps} />)

      await waitFor(() => {
        expect(screen.getByText('📊 Live Progress')).toBeInTheDocument()
      })
    })

    it('shows view results button when execution is complete', async () => {
      // Mock completed execution with no step progress
      mockUseTestAutomation.isExecutionInProgress = false
      mockUseTestAutomation.currentExecution = {
        execution_id: 'completed-test-123',
        status: 'passed',
        total_scenarios: 3,
        passed_scenarios: 3,
        failed_scenarios: 0
      }
      mockUseTestAutomation.getCurrentStepProgress.mockReturnValue(null)

      render(<TestAutomationPanel {...mockProps} />)

      await waitFor(() => {
        expect(screen.getByText('📊 View Results')).toBeInTheDocument()
      })
    })

    it('does not show results section when no execution exists', async () => {
      // Mock no execution
      mockUseTestAutomation.isExecutionInProgress = false
      mockUseTestAutomation.currentExecution = null
      mockUseTestAutomation.getCurrentStepProgress.mockReturnValue(null)

      render(<TestAutomationPanel {...mockProps} />)

      // The execution section should not be rendered at all
      expect(screen.queryByText('📊 View Results')).not.toBeInTheDocument()
      expect(screen.queryByText('📊 Live Progress')).not.toBeInTheDocument()
    })
  })
})
