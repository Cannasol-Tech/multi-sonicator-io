import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { ScenarioDetailsModal } from '../ScenarioDetailsModal'
import { TestScenario } from '../../types'

const mockScenario: TestScenario = {
  name: 'Test Pin Control',
  description: 'Test basic pin control functionality',
  feature_file: 'pin-control.feature',
  tags: ['basic', 'pins', 'hardware'],
  status: 'passed',
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
    },
    {
      step_type: 'Then',
      description: 'the pin state should be HIGH',
      status: 'passed',
      duration_ms: 200,
      error_message: null,
      pin_interactions: ['D7']
    }
  ]
}

describe('ScenarioDetailsModal Component', () => {
  it('does not render when not visible', () => {
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={false}
        onClose={vi.fn()}
      />
    )
    
    expect(screen.queryByText('📋 Test Scenario Details')).not.toBeInTheDocument()
  })

  it('does not render when scenario is null', () => {
    render(
      <ScenarioDetailsModal
        scenario={null}
        visible={true}
        onClose={vi.fn()}
      />
    )
    
    expect(screen.queryByText('📋 Test Scenario Details')).not.toBeInTheDocument()
  })

  it('renders scenario details correctly when visible', () => {
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={vi.fn()}
      />
    )
    
    // Check header
    expect(screen.getByText('📋 Test Scenario Details')).toBeInTheDocument()
    
    // Check scenario name
    expect(screen.getByText('Test Pin Control')).toBeInTheDocument()
    
    // Check description
    expect(screen.getByText('Description')).toBeInTheDocument()
    expect(screen.getByText('Test basic pin control functionality')).toBeInTheDocument()
    
    // Check feature file
    expect(screen.getByText('Feature File')).toBeInTheDocument()
    expect(screen.getByText('📄 pin-control.feature')).toBeInTheDocument()
  })

  it('renders tags correctly', () => {
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={vi.fn()}
      />
    )
    
    expect(screen.getByText('Tags')).toBeInTheDocument()
    expect(screen.getByText('@basic')).toBeInTheDocument()
    expect(screen.getByText('@pins')).toBeInTheDocument()
    expect(screen.getByText('@hardware')).toBeInTheDocument()
  })

  it('renders test steps correctly', () => {
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={vi.fn()}
      />
    )
    
    expect(screen.getByText('Test Steps (3)')).toBeInTheDocument()
    
    // Check step types
    expect(screen.getByText('Given')).toBeInTheDocument()
    expect(screen.getByText('When')).toBeInTheDocument()
    expect(screen.getByText('Then')).toBeInTheDocument()
    
    // Check step descriptions
    expect(screen.getByText('the Arduino is connected')).toBeInTheDocument()
    expect(screen.getByText('I set pin D7 to HIGH')).toBeInTheDocument()
    expect(screen.getByText('the pin state should be HIGH')).toBeInTheDocument()
    
    // Check pin interactions (multiple instances expected)
    expect(screen.getAllByText('Pin Interactions:')).toHaveLength(3) // One for each step
  })

  it('renders status information correctly', () => {
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={vi.fn()}
      />
    )
    
    expect(screen.getByText('Status Information')).toBeInTheDocument()
    expect(screen.getByText('Current Status:')).toBeInTheDocument()
    expect(screen.getByText('passed')).toBeInTheDocument()
    expect(screen.getByText('Total Steps:')).toBeInTheDocument()
    expect(screen.getByText('3')).toBeInTheDocument()
  })

  it('calls onClose when close button is clicked', () => {
    const mockOnClose = vi.fn()
    
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={mockOnClose}
      />
    )
    
    const closeButton = screen.getByText('×')
    fireEvent.click(closeButton)
    
    expect(mockOnClose).toHaveBeenCalledTimes(1)
  })

  it('calls onClose when Close button is clicked', () => {
    const mockOnClose = vi.fn()
    
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={mockOnClose}
      />
    )
    
    const closeButton = screen.getByText('Close')
    fireEvent.click(closeButton)
    
    expect(mockOnClose).toHaveBeenCalledTimes(1)
  })

  it('calls onClose when overlay is clicked', () => {
    const mockOnClose = vi.fn()
    
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={mockOnClose}
      />
    )
    
    const overlay = document.querySelector('.modal-overlay')
    expect(overlay).toBeInTheDocument()
    
    fireEvent.click(overlay!)
    expect(mockOnClose).toHaveBeenCalledTimes(1)
  })

  it('does not call onClose when modal content is clicked', () => {
    const mockOnClose = vi.fn()
    
    render(
      <ScenarioDetailsModal
        scenario={mockScenario}
        visible={true}
        onClose={mockOnClose}
      />
    )
    
    const modalContent = document.querySelector('.modal-content')
    expect(modalContent).toBeInTheDocument()
    
    fireEvent.click(modalContent!)
    expect(mockOnClose).not.toHaveBeenCalled()
  })

  it('handles scenario without tags', () => {
    const scenarioWithoutTags = { ...mockScenario, tags: [] }
    
    render(
      <ScenarioDetailsModal
        scenario={scenarioWithoutTags}
        visible={true}
        onClose={vi.fn()}
      />
    )
    
    expect(screen.queryByText('Tags')).not.toBeInTheDocument()
  })

  it('handles steps without pin interactions', () => {
    const scenarioWithoutPins = {
      ...mockScenario,
      steps: [
        {
          step_type: 'Given',
          description: 'the system is ready',
          status: 'passed',
          duration_ms: 100,
          error_message: null,
          pin_interactions: []
        }
      ]
    }
    
    render(
      <ScenarioDetailsModal
        scenario={scenarioWithoutPins}
        visible={true}
        onClose={vi.fn()}
      />
    )
    
    expect(screen.getByText('Test Steps (1)')).toBeInTheDocument()
    expect(screen.getByText('the system is ready')).toBeInTheDocument()
    expect(screen.queryByText('Pin Interactions:')).not.toBeInTheDocument()
  })
})
