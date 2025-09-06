import { useState, useEffect } from 'react'

interface IridescentProgressBarProps {
  progress: number // 0-100
  isRunning: boolean
  isComplete: boolean
  totalItems?: number
  currentItem?: number
  itemLabel?: string
  showStepProgress?: boolean
  stepProgress?: number
  currentStep?: number
  totalSteps?: number
  className?: string
}

export default function IridescentProgressBar({
  progress,
  isRunning,
  isComplete,
  totalItems = 25,
  currentItem,
  itemLabel = 'scenarios',
  showStepProgress = false,
  stepProgress = 0,
  currentStep = 1,
  totalSteps = 10,
  className = ''
}: IridescentProgressBarProps) {
  
  const getProgressBarClass = () => {
    if (isComplete) return 'progress-fill metallic-blue'
    if (isRunning) return 'progress-fill iridescent'
    return 'progress-fill'
  }

  const getStepProgressBarClass = () => {
    if (isComplete) return 'step-progress-fill metallic-blue'
    if (isRunning) return 'step-progress-fill iridescent'
    return 'step-progress-fill'
  }

  const calculatedCurrentItem = currentItem ?? Math.round((progress / 100) * totalItems)

  return (
    <div className={`iridescent-progress-container ${className}`}>
      {/* Main Progress Section */}
      <div className="progress-section">
        <div className="progress-label">
          <span style={{ fontSize: '14px', fontWeight: '500', color: 'var(--text-primary)' }}>
            Test Execution Progress
          </span>
          <span style={{ fontSize: '12px', color: 'var(--text-tertiary)' }}>
            {Math.round(progress)}%
          </span>
        </div>
        
        <div className="progress-bar" style={{
          width: '100%',
          height: '20px',
          background: 'var(--bg-tertiary)',
          borderRadius: '10px',
          overflow: 'hidden',
          marginBottom: '8px'
        }}>
          <div
            className={getProgressBarClass()}
            style={{
              width: `${progress}%`,
              height: '100%',
              background: (!isRunning && !isComplete) ? 'var(--border-color)' : undefined,
              transition: 'width 0.3s ease',
              borderRadius: '10px'
            }}
          />
        </div>
        
        <div style={{ 
          fontSize: '12px', 
          color: 'var(--text-tertiary)', 
          textAlign: 'center',
          marginBottom: showStepProgress ? '15px' : '0'
        }}>
          {calculatedCurrentItem} / {totalItems} {itemLabel} ({Math.round(progress)}%)
        </div>
      </div>

      {/* Step Progress Section */}
      {showStepProgress && isRunning && (
        <div className="step-progress-section">
          <div style={{ 
            fontSize: '11px', 
            color: 'var(--text-tertiary)', 
            marginBottom: '4px' 
          }}>
            Step {currentStep} / {totalSteps}
          </div>
          <div style={{
            width: '100%',
            height: '8px',
            background: 'var(--bg-tertiary)',
            borderRadius: '4px',
            overflow: 'hidden'
          }}>
            <div
              className={getStepProgressBarClass()}
              style={{
                width: `${stepProgress}%`,
                height: '100%',
                background: (!isRunning && !isComplete) ? 'var(--border-color)' : undefined,
                transition: 'width 0.3s ease',
                borderRadius: '4px'
              }}
            />
          </div>
        </div>
      )}

      {/* Status Message */}
      {isRunning && (
        <div style={{
          fontSize: '12px',
          color: 'var(--text-secondary)',
          textAlign: 'center',
          marginTop: '8px',
          fontStyle: 'italic'
        }}>
          🔄 Test execution in progress...
        </div>
      )}

      {isComplete && (
        <div style={{
          fontSize: '12px',
          color: 'var(--text-secondary)',
          textAlign: 'center',
          marginTop: '8px',
          fontWeight: '600'
        }}>
          ⚡ Test execution complete!
        </div>
      )}
    </div>
  )
}
