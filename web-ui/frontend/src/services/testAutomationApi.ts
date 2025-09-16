import { TestScenario, TestExecution } from '../types'

const API_BASE = '/api'

// Mock data for development when backend is not available
const MOCK_SCENARIOS: TestScenario[] = [
  {
    id: 'scenario-1',
    name: 'Basic Power Control',
    description: 'Test basic power on/off functionality',
    feature_file: 'power-control.feature',
    tags: ['@smoke', '@power'],
    status: 'pending',
    steps: [
      {
        step_type: 'Given',
        description: 'the system is powered off',
        status: 'pending',
        pin_interactions: ['POWER_SENSE_4']
      },
      {
        step_type: 'When',
        description: 'power is turned on',
        status: 'pending',
        pin_interactions: ['START_4']
      },
      {
        step_type: 'Then',
        description: 'power sensor detects power',
        status: 'pending',
        pin_interactions: ['POWER_SENSE_4']
      }
    ]
  },
  {
    id: 'scenario-2',
    name: 'Power Sensing Test',
    description: 'Test power level sensing functionality using POWER_SENSE_4 pin',
    feature_file: 'power-sensing.feature',
    tags: ['@power', '@analog', '@hil'],
    status: 'pending',
    steps: [
      {
        step_type: 'Given',
        description: 'the Arduino Test Harness is connected',
        status: 'pending',
        pin_interactions: ['UART_TXD']
      },
      {
        step_type: 'When',
        description: 'power sensing is enabled',
        status: 'pending',
        pin_interactions: ['POWER_SENSE_4']
      },
      {
        step_type: 'Then',
        description: 'power levels are accurately measured',
        status: 'pending',
        pin_interactions: ['POWER_SENSE_4']
      }
    ]
  },
  {
    id: 'scenario-3',
    name: 'Frequency Lock Control',
    description: 'Test frequency lock functionality using FREQ_LOCK_4 pin',
    feature_file: 'frequency-lock.feature',
    tags: ['@frequency', '@digital', '@hil'],
    status: 'pending',
    steps: [
      {
        step_type: 'Given',
        description: 'the system is in frequency unlocked state',
        status: 'pending',
        pin_interactions: ['FREQ_LOCK_4']
      },
      {
        step_type: 'When',
        description: 'frequency lock is activated',
        status: 'pending',
        pin_interactions: ['FREQ_LOCK_4']
      },
      {
        step_type: 'Then',
        description: 'frequency lock status is confirmed',
        status: 'pending',
        pin_interactions: ['FREQ_LOCK_4']
      }
    ]
  },
  {
    id: 'scenario-4',
    name: 'Overload Protection',
    description: 'Test overload protection using OVERLOAD_4 pin',
    feature_file: 'overload-protection.feature',
    tags: ['@safety', '@analog', '@hil'],
    status: 'pending',
    steps: [
      {
        step_type: 'Given',
        description: 'normal operating conditions',
        status: 'pending',
        pin_interactions: ['OVERLOAD_4']
      },
      {
        step_type: 'When',
        description: 'overload condition is detected',
        status: 'pending',
        pin_interactions: ['OVERLOAD_4']
      },
      {
        step_type: 'Then',
        description: 'system enters safe state',
        status: 'pending',
        pin_interactions: ['RESET_4']
      }
    ]
  },
  {
    id: 'scenario-5',
    name: 'PWM Amplitude Control',
    description: 'Test PWM amplitude control using AMPLITUDE_ALL pin',
    feature_file: 'amplitude-control.feature',
    tags: ['@pwm', '@analog', '@hil'],
    status: 'pending',
    steps: [
      {
        step_type: 'Given',
        description: 'PWM output is configured',
        status: 'pending',
        pin_interactions: ['AMPLITUDE_ALL']
      },
      {
        step_type: 'When',
        description: 'amplitude is set to 50%',
        status: 'pending',
        pin_interactions: ['AMPLITUDE_ALL']
      },
      {
        step_type: 'Then',
        description: 'PWM duty cycle reflects 50% amplitude',
        status: 'pending',
        pin_interactions: ['AMPLITUDE_ALL']
      }
    ]
  },
  {
    id: 'scenario-6',
    name: 'Frequency Division Monitoring',
    description: 'Test frequency division monitoring using FREQ_DIV10_4 pin',
    feature_file: 'frequency-division.feature',
    tags: ['@frequency', '@digital', '@hil'],
    status: 'pending',
    steps: [
      {
        step_type: 'Given',
        description: 'operating frequency is 20kHz',
        status: 'pending',
        pin_interactions: ['FREQ_DIV10_4']
      },
      {
        step_type: 'When',
        description: 'frequency division is enabled',
        status: 'pending',
        pin_interactions: ['FREQ_DIV10_4']
      },
      {
        step_type: 'Then',
        description: 'output frequency is 2kHz',
        status: 'pending',
        pin_interactions: ['FREQ_DIV10_4']
      }
    ]
  }
]

// Check if we should use mock data (when backend is not available)
const shouldUseMockData = () => {
  // Force mock data if explicitly requested
  if (import.meta.env.VITE_USE_MOCK_DATA === 'true') {
    return true
  }

  // In development, we'll try the backend first and fall back to mock data if it fails
  // This allows the Stop Execution button to work when backend is available
  return false
}

const MOCK_TAGS = ['@smoke', '@pin-control', '@hil', '@power', '@analog', '@frequency', '@digital', '@safety', '@overload', '@output', '@integration', '@system']

export class TestAutomationAPI {
  static async getAvailableScenarios(): Promise<TestScenario[]> {
    // Use mock data in development or when backend is not available
    if (shouldUseMockData()) {
      // Simulate network delay
      await new Promise(resolve => setTimeout(resolve, 500))
      return MOCK_SCENARIOS
    }

    try {
      const response = await fetch(`${API_BASE}/test/scenarios`)

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock data')
        return MOCK_SCENARIOS
      }

      const data = await response.json()
      return data.scenarios
    } catch (error) {
      console.warn('Backend not available, falling back to mock data:', error)
      return MOCK_SCENARIOS
    }
  }

  /**
   * Get scenarios filtered by tags
   */
  static async getScenariosByTags(tags: string[]): Promise<TestScenario[]> {
    if (tags.length === 0) {
      return this.getAvailableScenarios()
    }

    // Use mock data in development or when backend is not available
    if (shouldUseMockData()) {
      await new Promise(resolve => setTimeout(resolve, 300))
      return MOCK_SCENARIOS.filter(scenario =>
        scenario.tags.some(tag => tags.includes(tag))
      )
    }

    try {
      const tagString = tags.join(',')
      const response = await fetch(`${API_BASE}/test/scenarios/tags/${tagString}`)

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock data')
        return MOCK_SCENARIOS.filter(scenario =>
          scenario.tags.some(tag => tags.includes(tag))
        )
      }

      const data = await response.json()
      return data.scenarios
    } catch (error) {
      console.warn('Backend not available, falling back to mock data:', error)
      return MOCK_SCENARIOS.filter(scenario =>
        scenario.tags.some(tag => tags.includes(tag))
      )
    }
  }

  /**
   * Get all available tags
   */
  static async getAvailableTags(): Promise<string[]> {
    // Use mock data in development or when backend is not available
    if (shouldUseMockData()) {
      await new Promise(resolve => setTimeout(resolve, 200))
      return MOCK_TAGS
    }

    try {
      const response = await fetch(`${API_BASE}/test/tags`)

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock data')
        return MOCK_TAGS
      }

      const data = await response.json()
      return data.tags
    } catch (error) {
      console.warn('Backend not available, falling back to mock data:', error)
      return MOCK_TAGS
    }
  }

  /**
   * Get all available feature files
   */
  static async getAvailableFeatures(): Promise<string[]> {
    // Use mock data in development or when backend is not available
    if (shouldUseMockData()) {
      await new Promise(resolve => setTimeout(resolve, 200))
      return ['pin-control.feature', 'power-sensing.feature', 'frequency-control.feature', 'overload-protection.feature', 'system-integration.feature']
    }

    try {
      const response = await fetch(`${API_BASE}/test/features`)

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock data')
        return ['pin-control.feature', 'power-sensing.feature', 'frequency-control.feature', 'overload-protection.feature', 'system-integration.feature']
      }

      const data = await response.json()
      return data.features
    } catch (error) {
      console.warn('Backend not available, falling back to mock data:', error)
      return ['pin-control.feature', 'power-sensing.feature', 'frequency-control.feature', 'overload-protection.feature', 'system-integration.feature']
    }
  }

  /**
   * Execute selected test scenarios
   */
  static async executeScenarios(scenarioNames: string[], executionId: string): Promise<boolean> {
    // Use mock execution in development or when backend is not available
    if (shouldUseMockData()) {
      const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
      return await mockManager.startExecution(scenarioNames, executionId)
    }

    try {
      const response = await fetch(`${API_BASE}/test/execute`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          scenarios: scenarioNames,
          execution_id: executionId
        })
      })

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock execution')
        const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
        return await mockManager.startExecution(scenarioNames, executionId)
      }

      const data = await response.json()
      return data.success
    } catch (error) {
      console.warn('Backend not available, falling back to mock execution:', error)
      const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
      return await mockManager.startExecution(scenarioNames, executionId)
    }
  }

  /**
   * Get current test execution status
   */
  static async getExecutionStatus(): Promise<{execution: TestExecution | null, in_progress: boolean}> {
    // Use mock execution in development or when backend is not available
    if (shouldUseMockData()) {
      const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
      return mockManager.getExecutionStatus()
    }

    try {
      const response = await fetch(`${API_BASE}/test/execution`)

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock execution status')
        const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
        return mockManager.getExecutionStatus()
      }

      const data = await response.json()
      return {
        execution: data.execution,
        in_progress: data.in_progress
      }
    } catch (error) {
      console.warn('Backend not available, falling back to mock execution status:', error)
      const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
      return mockManager.getExecutionStatus()
    }
  }

  /**
   * Stop current test execution
   */
  static async stopExecution(): Promise<boolean> {
    // Use mock execution in development or when backend is not available
    if (shouldUseMockData()) {
      console.log('Using mock execution for stop')
      const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
      return mockManager.stopExecution()
    }

    console.log('Attempting to stop execution via backend API')
    try {
      const response = await fetch(`${API_BASE}/test/stop`, {
        method: 'POST'
      })

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock execution stop')
        const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
        return mockManager.stopExecution()
      }

      const data = await response.json()
      console.log('Stop execution response:', data)
      return data.success
    } catch (error) {
      console.warn('Backend not available, falling back to mock execution stop:', error)
      const mockManager = TestAutomationAPI.MockExecutionManager.getInstance()
      return mockManager.stopExecution()
    }
  }

  /**
   * Generate a unique execution ID
   */
  static generateExecutionId(): string {
    return `exec_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`
  }

  /**
   * Filter scenarios by feature file
   */
  static filterScenariosByFeature(scenarios: TestScenario[], featureFiles: string[]): TestScenario[] {
    if (featureFiles.length === 0) {
      return scenarios
    }
    
    return scenarios.filter(scenario => featureFiles.includes(scenario.feature_file))
  }

  /**
   * Filter scenarios by tags
   */
  static filterScenariosByTags(scenarios: TestScenario[], tags: string[]): TestScenario[] {
    if (tags.length === 0) {
      return scenarios
    }
    
    return scenarios.filter(scenario => 
      tags.some(tag => scenario.tags.includes(tag))
    )
  }

  /**
   * Get scenarios that interact with specific pins
   */
  static getScenariosByPins(scenarios: TestScenario[], pins: string[]): TestScenario[] {
    if (pins.length === 0) {
      return scenarios
    }
    
    return scenarios.filter(scenario => 
      scenario.steps.some(step => 
        step.pin_interactions.some(pin => pins.includes(pin))
      )
    )
  }

  /**
   * Get all pins involved in a scenario
   */
  static getScenarioPins(scenario: TestScenario): string[] {
    const pins = new Set<string>()
    
    scenario.steps.forEach(step => {
      step.pin_interactions.forEach(pin => pins.add(pin))
    })
    
    return Array.from(pins)
  }

  /**
   * Get test execution progress percentage
   */
  static getExecutionProgress(execution: TestExecution): number {
    if (execution.total_scenarios === 0) {
      return 0
    }
    
    const completedScenarios = execution.passed_scenarios + execution.failed_scenarios
    return Math.round((completedScenarios / execution.total_scenarios) * 100)
  }

  /**
   * Get current step progress for active scenario
   */
  static getCurrentStepProgress(execution: TestExecution): {current: number, total: number} {
    if (execution.current_scenario_index >= execution.scenarios.length) {
      return {current: 0, total: 0}
    }
    
    const currentScenario = execution.scenarios[execution.current_scenario_index]
    const completedSteps = currentScenario.steps.filter(step => 
      step.status === 'passed' || step.status === 'failed' || step.status === 'error'
    ).length
    
    return {
      current: completedSteps,
      total: currentScenario.steps.length
    }
  }

  /**
   * Format execution duration
   */
  static formatDuration(durationMs?: number): string {
    if (!durationMs || durationMs < 0) {
      return '0s'
    }

    // Safety check: if duration seems to be in seconds instead of milliseconds
    // (i.e., unreasonably large), assume it's a timestamp issue and handle gracefully
    if (durationMs > 1000000000) { // More than ~11 days in milliseconds is suspicious
      console.warn('Duration appears to be in wrong units, attempting to correct:', durationMs)
      return 'Invalid duration'
    }

    const totalSeconds = Math.floor(durationMs / 1000)
    const hours = Math.floor(totalSeconds / 3600)
    const minutes = Math.floor((totalSeconds % 3600) / 60)
    const seconds = totalSeconds % 60

    if (hours > 0) {
      return `${hours}h ${minutes}m ${seconds}s`
    } else if (minutes > 0) {
      return `${minutes}m ${seconds}s`
    } else if (totalSeconds > 0) {
      return `${seconds}s`
    } else {
      // For very short durations, show milliseconds
      return `${Math.round(durationMs)}ms`
    }
  }

  /**
   * Get status color for UI display
   */
  static getStatusColor(status: string): string {
    switch (status) {
      case 'passed': return '#22c55e'
      case 'failed': return '#ef4444'
      case 'error': return '#f97316'
      case 'running': return '#3b82f6'
      case 'pending': return '#6b7280'
      case 'skipped': return '#a3a3a3'
      default: return '#6b7280'
    }
  }

  /**
   * Get status icon for UI display
   */
  static getStatusIcon(status: string): string {
    switch (status) {
      case 'passed': return '✅'
      case 'failed': return '❌'
      case 'error': return '⚠️'
      case 'running': return '🔄'
      case 'pending': return '⏳'
      case 'skipped': return '⏭️'
      default: return '❓'
    }
  }
}
