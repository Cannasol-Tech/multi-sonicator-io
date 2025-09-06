import { render, screen } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import IridescentProgressBar from '../IridescentProgressBar'

describe('IridescentProgressBar', () => {
  it('should render with basic props', () => {
    render(
      <IridescentProgressBar
        progress={50}
        isRunning={false}
        isComplete={false}
      />
    )
    
    expect(screen.getByText('Test Execution Progress')).toBeInTheDocument()
    expect(screen.getByText('50%')).toBeInTheDocument()
  })

  it('should show running status message when isRunning is true', () => {
    render(
      <IridescentProgressBar
        progress={75}
        isRunning={true}
        isComplete={false}
      />
    )
    
    // Check that the new text is displayed instead of the old "Beautiful iridescent progress in action!"
    expect(screen.getByText('🔄 Test execution in progress...')).toBeInTheDocument()
    expect(screen.queryByText('🌈 Beautiful iridescent progress in action!')).not.toBeInTheDocument()
  })

  it('should show completion status message when isComplete is true', () => {
    render(
      <IridescentProgressBar
        progress={100}
        isRunning={false}
        isComplete={true}
      />
    )
    
    expect(screen.getByText('⚡ Test execution complete!')).toBeInTheDocument()
  })

  it('should not show status message when not running and not complete', () => {
    render(
      <IridescentProgressBar
        progress={0}
        isRunning={false}
        isComplete={false}
      />
    )
    
    expect(screen.queryByText('🔄 Test execution in progress...')).not.toBeInTheDocument()
    expect(screen.queryByText('⚡ Test execution complete!')).not.toBeInTheDocument()
  })

  it('should display progress with custom item labels', () => {
    render(
      <IridescentProgressBar
        progress={60}
        isRunning={false}
        isComplete={false}
        totalItems={10}
        currentItem={6}
        itemLabel="tests"
      />
    )
    
    expect(screen.getByText('6 / 10 tests (60%)')).toBeInTheDocument()
  })

  it('should show step progress when enabled', () => {
    render(
      <IridescentProgressBar
        progress={50}
        isRunning={true}
        isComplete={false}
        showStepProgress={true}
        stepProgress={75}
        currentStep={3}
        totalSteps={4}
      />
    )
    
    expect(screen.getByText('Step 3 / 4')).toBeInTheDocument()
  })

  it('should apply custom className', () => {
    const { container } = render(
      <IridescentProgressBar
        progress={50}
        isRunning={false}
        isComplete={false}
        className="custom-progress"
      />
    )
    
    expect(container.querySelector('.custom-progress')).toBeInTheDocument()
  })

  it('should calculate current item from progress when not provided', () => {
    render(
      <IridescentProgressBar
        progress={40}
        isRunning={false}
        isComplete={false}
        totalItems={25}
      />
    )
    
    // 40% of 25 = 10
    expect(screen.getByText('10 / 25 scenarios (40%)')).toBeInTheDocument()
  })
})
