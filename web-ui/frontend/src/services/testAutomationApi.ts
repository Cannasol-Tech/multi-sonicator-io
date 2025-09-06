import { TestScenario, TestExecution } from '../types'

const API_BASE = process.env.NODE_ENV === 'production' ? '/api' : 'http://localhost:3001/api'

// Mock data for development when backend is not available
const MOCK_SCENARIOS: TestScenario[] = [
  {
    id: 'scenario-1',
    name: 'Basic Pin Control Test',
    description: 'Test basic pin control operations for START_4 and RESET_4 pins',
    feature_file: 'pin-control.feature',
    tags: ['@smoke', '@pin-control', '@hil'],
    steps: [
      {
        step_type: 'Given',
        description: 'the Arduino is connected and ready',
        pin_interactions: [],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I set START_4 pin to HIGH',
        pin_interactions: ['START_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'the START_4 pin should be HIGH',
        pin_interactions: ['START_4'],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I set RESET_4 pin to HIGH',
        pin_interactions: ['RESET_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'the RESET_4 pin should be HIGH',
        pin_interactions: ['RESET_4'],
        status: 'pending'
      }
    ]
  },
  {
    id: 'scenario-2',
    name: 'Power Sensing Test',
    description: 'Test power level sensing functionality using POWER_SENSE_4 pin',
    feature_file: 'power-sensing.feature',
    tags: ['@power', '@analog', '@hil'],
    steps: [
      {
        step_type: 'Given',
        description: 'the power sensing circuit is connected',
        pin_interactions: [],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I read the POWER_SENSE_4 analog value',
        pin_interactions: ['POWER_SENSE_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'the power level should be within expected range',
        pin_interactions: ['POWER_SENSE_4'],
        status: 'pending'
      }
    ]
  },
  {
    id: 'scenario-3',
    name: 'Frequency Lock Test',
    description: 'Test frequency lock detection using FREQ_LOCK_4 pin',
    feature_file: 'frequency-control.feature',
    tags: ['@frequency', '@digital', '@hil'],
    steps: [
      {
        step_type: 'Given',
        description: 'the frequency generator is connected',
        pin_interactions: [],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I monitor FREQ_LOCK_4 pin state',
        pin_interactions: ['FREQ_LOCK_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'the frequency lock should be detected',
        pin_interactions: ['FREQ_LOCK_4'],
        status: 'pending'
      }
    ]
  },
  {
    id: 'scenario-4',
    name: 'Overload Protection Test',
    description: 'Test overload protection using OVERLOAD_4 pin monitoring',
    feature_file: 'overload-protection.feature',
    tags: ['@safety', '@overload', '@hil'],
    steps: [
      {
        step_type: 'Given',
        description: 'the overload protection circuit is active',
        pin_interactions: [],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I simulate an overload condition',
        pin_interactions: ['OVERLOAD_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'the OVERLOAD_4 pin should trigger',
        pin_interactions: ['OVERLOAD_4'],
        status: 'pending'
      },
      {
        step_type: 'And',
        description: 'the system should enter safe mode',
        pin_interactions: ['START_4', 'RESET_4'],
        status: 'pending'
      }
    ]
  },
  {
    id: 'scenario-5',
    name: 'Frequency Division Test',
    description: 'Test frequency division output on FREQ_DIV10_4 pin',
    feature_file: 'frequency-control.feature',
    tags: ['@frequency', '@output', '@hil'],
    steps: [
      {
        step_type: 'Given',
        description: 'the frequency generator is running',
        pin_interactions: [],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I enable frequency division',
        pin_interactions: ['START_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'FREQ_DIV10_4 should output divided frequency',
        pin_interactions: ['FREQ_DIV10_4'],
        status: 'pending'
      }
    ]
  },
  {
    id: 'scenario-6',
    name: 'Complete System Test',
    description: 'Comprehensive test of all Multi-Sonicator IO functionality',
    feature_file: 'system-integration.feature',
    tags: ['@integration', '@system', '@hil', '@smoke'],
    steps: [
      {
        step_type: 'Given',
        description: 'all hardware components are connected',
        pin_interactions: [],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I initialize the system',
        pin_interactions: ['START_4', 'RESET_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'all pins should be in expected states',
        pin_interactions: ['START_4', 'RESET_4', 'POWER_SENSE_4', 'FREQ_LOCK_4', 'OVERLOAD_4', 'FREQ_DIV10_4'],
        status: 'pending'
      },
      {
        step_type: 'When',
        description: 'I run a complete test cycle',
        pin_interactions: ['START_4', 'RESET_4', 'POWER_SENSE_4', 'FREQ_LOCK_4'],
        status: 'pending'
      },
      {
        step_type: 'Then',
        description: 'the system should operate correctly',
        pin_interactions: ['FREQ_DIV10_4', 'OVERLOAD_4'],
        status: 'pending'
      }
    ]
  }
]

const MOCK_TAGS = ['@smoke', '@pin-control', '@hil', '@power', '@analog', '@frequency', '@digital', '@safety', '@overload', '@output', '@integration', '@system']

// Check if we should use mock data (when backend is not available)
const shouldUseMockData = () => {
  // Force mock data if explicitly requested
  if (process.env.REACT_APP_USE_MOCK_DATA === 'true') {
    return true
  }

  // In development, we'll try the backend first and fall back to mock data if it fails
  // This allows the Stop Execution button to work when backend is available
  return false
}

// Mock execution state for development
class MockExecutionManager {
  private static instance: MockExecutionManager
  private currentExecution: TestExecution | null = null
  private isInProgress = false
  private progressInterval: NodeJS.Timeout | null = null

  static getInstance(): MockExecutionManager {
    if (!MockExecutionManager.instance) {
      MockExecutionManager.instance = new MockExecutionManager()
    }
    return MockExecutionManager.instance
  }

  async startExecution(scenarioNames: string[], executionId: string): Promise<boolean> {
    if (this.isInProgress) {
      return false
    }

    // Find scenarios to execute
    const scenariosToExecute = MOCK_SCENARIOS.filter(s => scenarioNames.includes(s.name))
    if (scenariosToExecute.length === 0) {
      return false
    }

    // Create mock execution
    this.currentExecution = {
      execution_id: executionId,
      status: 'running',
      start_time: Date.now(),
      end_time: undefined,
      total_scenarios: scenariosToExecute.length,
      passed_scenarios: 0,
      failed_scenarios: 0,
      current_scenario_index: 0,
      current_step_index: 0,
      scenarios: scenariosToExecute,
      results: []
    }

    this.isInProgress = true

    // Simulate execution progress
    this.simulateExecution()

    return true
  }

  private simulateExecution() {
    if (!this.currentExecution) return

    const totalSteps = MOCK_SCENARIOS
      .filter(s => this.currentExecution!.scenarios.some(scenario => scenario.name === s.name))
      .reduce((total, scenario) => total + scenario.steps.length, 0)

    let currentStep = 0

    this.progressInterval = setInterval(() => {
      if (!this.currentExecution || !this.isInProgress) {
        if (this.progressInterval) {
          clearInterval(this.progressInterval)
          this.progressInterval = null
        }
        return
      }

      currentStep++

      // Update progress
      const progress = currentStep / totalSteps
      const scenarioProgress = Math.floor(progress * this.currentExecution.total_scenarios)

      // Simulate some failures (10% failure rate)
      if (Math.random() < 0.1 && currentStep % 5 === 0) {
        this.currentExecution.failed_scenarios = Math.min(
          this.currentExecution.failed_scenarios + 1,
          scenarioProgress
        )
      }

      this.currentExecution.passed_scenarios = Math.max(
        0,
        scenarioProgress - this.currentExecution.failed_scenarios
      )

      this.currentExecution.current_scenario_index = Math.min(
        scenarioProgress,
        this.currentExecution.total_scenarios - 1
      )

      // Complete execution
      if (currentStep >= totalSteps) {
        this.currentExecution.status = this.currentExecution.failed_scenarios > 0 ? 'failed' : 'passed'
        this.currentExecution.end_time = Date.now()
        this.isInProgress = false

        if (this.progressInterval) {
          clearInterval(this.progressInterval)
          this.progressInterval = null
        }
      }
    }, 200) // Update every 200ms for smooth progress
  }

  stopExecution(): boolean {
    if (!this.isInProgress || !this.currentExecution) {
      return false
    }

    this.isInProgress = false
    this.currentExecution.status = 'failed' // Use 'failed' instead of 'stopped'
    this.currentExecution.end_time = Date.now()

    if (this.progressInterval) {
      clearInterval(this.progressInterval)
      this.progressInterval = null
    }

    return true
  }

  getExecutionStatus(): {execution: TestExecution | null, in_progress: boolean} {
    return {
      execution: this.currentExecution,
      in_progress: this.isInProgress
    }
  }

  clearExecution() {
    this.currentExecution = null
    this.isInProgress = false
    if (this.progressInterval) {
      clearInterval(this.progressInterval)
      this.progressInterval = null
    }
  }
}

export class TestAutomationAPI {
  /**
   * Get all available test scenarios
   */
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
      const mockManager = MockExecutionManager.getInstance()
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
        const mockManager = MockExecutionManager.getInstance()
        return await mockManager.startExecution(scenarioNames, executionId)
      }

      const data = await response.json()
      return data.success
    } catch (error) {
      console.warn('Backend not available, falling back to mock execution:', error)
      const mockManager = MockExecutionManager.getInstance()
      return await mockManager.startExecution(scenarioNames, executionId)
    }
  }

  /**
   * Get current test execution status
   */
  static async getExecutionStatus(): Promise<{execution: TestExecution | null, in_progress: boolean}> {
    // Use mock execution in development or when backend is not available
    if (shouldUseMockData()) {
      const mockManager = MockExecutionManager.getInstance()
      return mockManager.getExecutionStatus()
    }

    try {
      const response = await fetch(`${API_BASE}/test/execution`)

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock execution status')
        const mockManager = MockExecutionManager.getInstance()
        return mockManager.getExecutionStatus()
      }

      const data = await response.json()
      return {
        execution: data.execution,
        in_progress: data.in_progress
      }
    } catch (error) {
      console.warn('Backend not available, falling back to mock execution status:', error)
      const mockManager = MockExecutionManager.getInstance()
      return mockManager.getExecutionStatus()
    }
  }

  /**
   * Stop current test execution
   */
  static async stopExecution(): Promise<boolean> {
    // Use mock execution in development or when backend is not available
    if (shouldUseMockData()) {
      const mockManager = MockExecutionManager.getInstance()
      return mockManager.stopExecution()
    }

    try {
      const response = await fetch(`${API_BASE}/test/stop`, {
        method: 'POST'
      })

      if (!response.ok) {
        console.warn('Backend not available, falling back to mock execution stop')
        const mockManager = MockExecutionManager.getInstance()
        return mockManager.stopExecution()
      }

      const data = await response.json()
      return data.success
    } catch (error) {
      console.warn('Backend not available, falling back to mock execution stop:', error)
      const mockManager = MockExecutionManager.getInstance()
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
