import { useState, useEffect } from 'react'

interface ProgressBarDemoProps {
  className?: string
}

export default function ProgressBarDemo({ className = '' }: ProgressBarDemoProps) {
  const [progress, setProgress] = useState(0)
  const [isRunning, setIsRunning] = useState(false)

  useEffect(() => {
    let interval: NodeJS.Timeout
    
    if (isRunning) {
      interval = setInterval(() => {
        setProgress(prev => {
          if (prev >= 100) {
            setIsRunning(false)
            return 100
          }
          return prev + 2
        })
      }, 100)
    }

    return () => {
      if (interval) clearInterval(interval)
    }
  }, [isRunning])

  const handleStart = () => {
    setProgress(0)
    setIsRunning(true)
  }

  const handleReset = () => {
    setProgress(0)
    setIsRunning(false)
  }

  return (
    <div className={`progress-demo ${className}`} style={{ 
      padding: '20px', 
      background: 'white', 
      borderRadius: '8px', 
      boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
      margin: '20px'
    }}>
      <h3 style={{ marginBottom: '20px', color: '#374151' }}>
        🌈 Iridescent Progress Bar Demo
      </h3>
      
      <div style={{ marginBottom: '15px' }}>
        <div style={{ 
          display: 'flex', 
          justifyContent: 'space-between', 
          alignItems: 'center',
          marginBottom: '8px'
        }}>
          <span style={{ fontSize: '14px', fontWeight: '500', color: '#374151' }}>
            Test Progress
          </span>
          <span style={{ fontSize: '12px', color: '#6b7280' }}>
            {progress.toFixed(0)}%
          </span>
        </div>
        
        {/* Main Progress Bar */}
        <div style={{
          width: '100%',
          height: '20px',
          background: '#e9ecef',
          borderRadius: '10px',
          overflow: 'hidden',
          marginBottom: '8px'
        }}>
          <div
            className={
              isRunning ? 'progress-fill iridescent' :
              progress === 100 ? 'progress-fill metallic-blue' :
              'progress-fill'
            }
            style={{
              width: `${progress}%`,
              height: '100%',
              background: (isRunning || progress === 100) ? undefined : '#e5e7eb',
              transition: 'width 0.3s ease',
              borderRadius: '10px'
            }}
          />
        </div>
        
        <div style={{ 
          fontSize: '12px', 
          color: '#6c757d', 
          textAlign: 'center',
          marginBottom: '15px'
        }}>
          {Math.round((progress / 100) * 25)} / 25 scenarios ({progress.toFixed(0)}%)
        </div>
      </div>

      {/* Step Progress Bar */}
      {isRunning && (
        <div style={{ marginBottom: '15px' }}>
          <div style={{ fontSize: '11px', color: '#6c757d', marginBottom: '4px' }}>
            Step {Math.min(Math.floor(progress / 10) + 1, 10)} / 10
          </div>
          <div style={{
            width: '100%',
            height: '8px',
            background: '#e9ecef',
            borderRadius: '4px',
            overflow: 'hidden'
          }}>
            <div
              className={
                isRunning ? 'step-progress-fill iridescent' :
                progress === 100 ? 'step-progress-fill metallic-blue' :
                'step-progress-fill'
              }
              style={{
                width: `${((progress % 10) / 10) * 100}%`,
                height: '100%',
                background: (isRunning || progress === 100) ? undefined : '#17a2b8',
                transition: 'width 0.3s ease',
                borderRadius: '4px'
              }}
            />
          </div>
        </div>
      )}

      {/* Controls */}
      <div style={{ display: 'flex', gap: '10px', justifyContent: 'center' }}>
        <button
          onClick={handleStart}
          disabled={isRunning}
          style={{
            padding: '8px 16px',
            background: isRunning ? '#94a3b8' : '#3b82f6',
            color: 'white',
            border: 'none',
            borderRadius: '6px',
            cursor: isRunning ? 'not-allowed' : 'pointer',
            fontSize: '14px',
            fontWeight: '500'
          }}
        >
          {isRunning ? '🔄 Running...' : '▶️ Start Test'}
        </button>
        
        <button
          onClick={handleReset}
          style={{
            padding: '8px 16px',
            background: '#6b7280',
            color: 'white',
            border: 'none',
            borderRadius: '6px',
            cursor: 'pointer',
            fontSize: '14px',
            fontWeight: '500'
          }}
        >
          🔄 Reset
        </button>
      </div>

      {/* Status */}
      <div style={{ 
        marginTop: '15px', 
        padding: '8px', 
        background: isRunning ? '#dbeafe' : progress === 100 ? '#d4edda' : '#f8f9fa',
        color: isRunning ? '#1e40af' : progress === 100 ? '#155724' : '#6c757d',
        borderRadius: '4px',
        fontSize: '12px',
        textAlign: 'center'
      }}>
        {isRunning ? '🔄 Test execution in progress...' :
         progress === 100 ? '⚡ Test execution complete!' :
         '⏸️ Ready to demonstrate the new progress design'}
      </div>
    </div>
  )
}
