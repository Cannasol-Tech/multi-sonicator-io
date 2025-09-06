import { TestScenario, TestExecution } from '../types'

const API_BASE = process.env.NODE_ENV === 'production' ? '/api' : 'http://localhost:3001/api'

export class TestAutomationAPI {
  /**
   * Get all available test scenarios
   */
  static async getAvailableScenarios(): Promise<TestScenario[]> {
    const response = await fetch(`${API_BASE}/test/scenarios`)
    
    if (!response.ok) {
      throw new Error(`Failed to get scenarios: ${response.statusText}`)
    }
    
    const data = await response.json()
    return data.scenarios
  }

  /**
   * Get scenarios filtered by tags
   */
  static async getScenariosByTags(tags: string[]): Promise<TestScenario[]> {
    if (tags.length === 0) {
      return this.getAvailableScenarios()
    }

    const tagString = tags.join(',')
    const response = await fetch(`${API_BASE}/test/scenarios/tags/${tagString}`)
    
    if (!response.ok) {
      throw new Error(`Failed to get scenarios by tags: ${response.statusText}`)
    }
    
    const data = await response.json()
    return data.scenarios
  }

  /**
   * Get all available tags
   */
  static async getAvailableTags(): Promise<string[]> {
    const response = await fetch(`${API_BASE}/test/tags`)
    
    if (!response.ok) {
      throw new Error(`Failed to get tags: ${response.statusText}`)
    }
    
    const data = await response.json()
    return data.tags
  }

  /**
   * Get all available feature files
   */
  static async getAvailableFeatures(): Promise<string[]> {
    const response = await fetch(`${API_BASE}/test/features`)
    
    if (!response.ok) {
      throw new Error(`Failed to get features: ${response.statusText}`)
    }
    
    const data = await response.json()
    return data.features
  }

  /**
   * Execute selected test scenarios
   */
  static async executeScenarios(scenarioNames: string[], executionId: string): Promise<boolean> {
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
      const errorData = await response.json()
      throw new Error(errorData.error || `Failed to execute scenarios: ${response.statusText}`)
    }
    
    const data = await response.json()
    return data.success
  }

  /**
   * Get current test execution status
   */
  static async getExecutionStatus(): Promise<{execution: TestExecution | null, in_progress: boolean}> {
    const response = await fetch(`${API_BASE}/test/execution`)
    
    if (!response.ok) {
      throw new Error(`Failed to get execution status: ${response.statusText}`)
    }
    
    const data = await response.json()
    return {
      execution: data.execution,
      in_progress: data.in_progress
    }
  }

  /**
   * Stop current test execution
   */
  static async stopExecution(): Promise<boolean> {
    const response = await fetch(`${API_BASE}/test/stop`, {
      method: 'POST'
    })
    
    if (!response.ok) {
      throw new Error(`Failed to stop execution: ${response.statusText}`)
    }
    
    const data = await response.json()
    return data.success
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
