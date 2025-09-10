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

  describe('Progress Bar Styling', () => {
    it('applies correct CSS classes for different states', () => {
      const { container, rerender } = render(
        <IridescentProgressBar
          progress={50}
          isRunning={false}
          isComplete={false}
        />
      )
      
      let progressFill = container.querySelector('.progress-fill')
      expect(progressFill).toHaveClass('progress-fill')
      expect(progressFill).not.toHaveClass('iridescent')
      expect(progressFill).not.toHaveClass('metallic-blue')
      
      // Test running state
      rerender(
        <IridescentProgressBar
          progress={50}
          isRunning={true}
          isComplete={false}
        />
      )
      progressFill = container.querySelector('.progress-fill')
      expect(progressFill).toHaveClass('progress-fill', 'iridescent')
      
      // Test complete state
      rerender(
        <IridescentProgressBar
          progress={100}
          isRunning={false}
          isComplete={true}
        />
      )
      progressFill = container.querySelector('.progress-fill')
      expect(progressFill).toHaveClass('progress-fill', 'metallic-blue')
    })

    it('sets correct width based on progress', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={73}
          isRunning={false}
          isComplete={false}
        />
      )
      
      const progressFill = container.querySelector('.progress-fill') as HTMLElement
      expect(progressFill.style.width).toBe('73%')
    })

    it('applies transition for smooth animation', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={50}
          isRunning={false}
          isComplete={false}
        />
      )
      
      const progressFill = container.querySelector('.progress-fill') as HTMLElement
      expect(progressFill.style.transition).toBe('width 0.3s ease')
    })
  })

  describe('Step Progress Feature', () => {
    it('does not show step progress when not running', () => {
      render(
        <IridescentProgressBar
          progress={50}
          isRunning={false}
          isComplete={false}
          showStepProgress={true}
          currentStep={2}
          totalSteps={5}
        />
      )
      
      expect(screen.queryByText('Step 2 / 5')).not.toBeInTheDocument()
    })

    it('applies correct step progress width', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={50}
          isRunning={true}
          isComplete={false}
          showStepProgress={true}
          stepProgress={65}
        />
      )
      
      const stepProgressFill = container.querySelector('.step-progress-fill') as HTMLElement
      expect(stepProgressFill.style.width).toBe('65%')
    })

    it('applies correct step progress styling classes', () => {
      const { container, rerender } = render(
        <IridescentProgressBar
          progress={50}
          isRunning={true}
          isComplete={false}
          showStepProgress={true}
          stepProgress={50}
        />
      )
      
      let stepProgressFill = container.querySelector('.step-progress-fill')
      expect(stepProgressFill).toHaveClass('step-progress-fill', 'iridescent')
      
      // Test complete state
      rerender(
        <IridescentProgressBar
          progress={100}
          isRunning={true}
          isComplete={true}
          showStepProgress={true}
          stepProgress={100}
        />
      )
      stepProgressFill = container.querySelector('.step-progress-fill')
      expect(stepProgressFill).toHaveClass('step-progress-fill', 'metallic-blue')
    })
  })

  describe('Edge Cases and Error Handling', () => {
    it('handles zero progress correctly', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={0}
          isRunning={false}
          isComplete={false}
        />
      )
      
      expect(screen.getByText('0%')).toBeInTheDocument()
      expect(screen.getByText('0 / 25 scenarios (0%)')).toBeInTheDocument()
      
      const progressFill = container.querySelector('.progress-fill') as HTMLElement
      expect(progressFill.style.width).toBe('0%')
    })

    it('handles 100% progress correctly', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={100}
          isRunning={false}
          isComplete={false}
        />
      )
      
      expect(screen.getByText('100%')).toBeInTheDocument()
      const progressFill = container.querySelector('.progress-fill') as HTMLElement
      expect(progressFill.style.width).toBe('100%')
    })

    it('handles progress values over 100', () => {
      render(
        <IridescentProgressBar
          progress={120}
          isRunning={false}
          isComplete={false}
        />
      )
      
      expect(screen.getByText('120%')).toBeInTheDocument()
    })

    it('handles zero total items', () => {
      render(
        <IridescentProgressBar
          progress={50}
          isRunning={false}
          isComplete={false}
          totalItems={0}
        />
      )
      
      expect(screen.getByText('0 / 0 scenarios (50%)')).toBeInTheDocument()
    })

    it('uses default step values when not provided', () => {
      render(
        <IridescentProgressBar
          progress={50}
          isRunning={true}
          isComplete={false}
          showStepProgress={true}
        />
      )
      
      expect(screen.getByText('Step 1 / 10')).toBeInTheDocument()
    })

    it('rounds calculated current item correctly', () => {
      render(
        <IridescentProgressBar
          progress={33}
          isRunning={false}
          isComplete={false}
          totalItems={10}
        />
      )
      
      // 33% of 10 = 3.3, should round to 3
      expect(screen.getByText('3 / 10 scenarios (33%)')).toBeInTheDocument()
    })
  })

  describe('Accessibility', () => {
    it('provides semantic structure with clear hierarchy', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={65}
          isRunning={true}
          isComplete={false}
        />
      )
      
      const progressContainer = container.querySelector('.iridescent-progress-container')
      const progressSection = container.querySelector('.progress-section')
      expect(progressContainer).toContainElement(progressSection)
    })

    it('uses appropriate font sizes for text hierarchy', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={50}
          isRunning={false}
          isComplete={false}
        />
      )
      
      // Check main label uses larger font
      const mainLabel = container.querySelector('[style*="fontSize: 14px"]')
      expect(mainLabel).toBeInTheDocument()
      expect(mainLabel).toHaveTextContent('Test Execution Progress')
      
      // Check secondary text uses smaller font
      const secondaryText = container.querySelector('[style*="fontSize: 12px"]')
      expect(secondaryText).toBeInTheDocument()
    })

    it('maintains readable text content for screen readers', () => {
      render(
        <IridescentProgressBar
          progress={75}
          isRunning={true}
          isComplete={false}
          totalItems={20}
          currentItem={15}
          itemLabel="tests"
        />
      )
      
      expect(screen.getByText('Test Execution Progress')).toBeInTheDocument()
      expect(screen.getByText('75%')).toBeInTheDocument()
      expect(screen.getByText('15 / 20 tests (75%)')).toBeInTheDocument()
      expect(screen.getByText('🔄 Test execution in progress...')).toBeInTheDocument()
    })
  })

  describe('CSS Custom Properties', () => {
    it('uses CSS variables for consistent theming', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={50}
          isRunning={false}
          isComplete={false}
        />
      )
      
      // Check that CSS custom properties are used in inline styles
      const elementsWithCSSVars = container.querySelectorAll('[style*="var("]')
      expect(elementsWithCSSVars.length).toBeGreaterThan(0)
    })
  })

  describe('Visual Regression', () => {
    it('matches snapshot for default idle state', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={42}
          isRunning={false}
          isComplete={false}
        />
      )
      expect(container.firstChild).toMatchSnapshot()
    })

    it('matches snapshot for running state with step progress', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={67}
          isRunning={true}
          isComplete={false}
          showStepProgress={true}
          stepProgress={23}
          currentStep={4}
          totalSteps={7}
          totalItems={15}
          itemLabel="scenarios"
        />
      )
      expect(container.firstChild).toMatchSnapshot()
    })

    it('matches snapshot for completed state', () => {
      const { container } = render(
        <IridescentProgressBar
          progress={100}
          isRunning={false}
          isComplete={true}
          totalItems={10}
          currentItem={10}
          itemLabel="tests"
        />
      )
      expect(container.firstChild).toMatchSnapshot()
    })
  })
})
