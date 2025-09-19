// Test Automation API Service
// Provides API integration and mock fallbacks for test automation features

import { TestExecution, TestScenario, TestStep } from '../types'

interface ExecutionStatusPayload {
  execution: TestExecution | null
  in_progress: boolean
  timestamp?: number
}

const cloneTestStep = (step: TestStep): TestStep => ({
  ...step,
  pin_interactions: [...(step.pin_interactions || [])]
})

const cloneTestScenario = (scenario: TestScenario): TestScenario => ({
  ...scenario,
  steps: (scenario.steps || []).map(cloneTestStep)
})

const cloneTestExecution = (execution: TestExecution): TestExecution => ({
  ...execution,
  scenarios: execution.scenarios.map(cloneTestScenario),
  results: execution.results ? [...execution.results] : execution.results
})

const filterScenariosByTagsHelper = (scenarios: TestScenario[], tags: string[]): TestScenario[] => {
  if (!tags.length) {
    return scenarios
  }

  const normalized = tags.map(tag => tag.toLowerCase())

  return scenarios.filter(scenario => {
    const scenarioTags = (scenario.tags || []).map(tag => tag.toLowerCase())
    return normalized.every(tag => scenarioTags.includes(tag))
  })
}

const filterScenariosByFeatureHelper = (scenarios: TestScenario[], featureFiles: string[]): TestScenario[] => {
  if (!featureFiles.length) {
    return scenarios
  }

  const normalized = new Set(featureFiles.map(feature => feature.toLowerCase()))

  return scenarios.filter(scenario => {
    if (scenario.feature_file && normalized.has(scenario.feature_file.toLowerCase())) {
      return true
    }
    if (scenario.feature_name && normalized.has(scenario.feature_name.toLowerCase())) {
      return true
    }
    return false
  })
}

const getScenarioPinsHelper = (scenario: TestScenario): string[] => {
  const pins = new Set<string>()
  ;(scenario.steps || []).forEach(step => {
    (step.pin_interactions || []).forEach(pin => pins.add(pin))
  })
  return Array.from(pins)
}

const getScenariosByPinsHelper = (scenarios: TestScenario[], pins: string[]): TestScenario[] => {
  if (!pins.length) {
    return []
  }

  const normalized = new Set(pins.map(pin => pin.toUpperCase()))

  return scenarios.filter(scenario => {
    const scenarioPins = getScenarioPinsHelper(scenario).map(pin => pin.toUpperCase())
    return scenarioPins.some(pin => normalized.has(pin))
  })
}

/**
 * Determines whether the web UI should use mock data for test automation endpoints.
 */
export const shouldUseMockData = (): boolean => {
  if (typeof window === 'undefined') {
    return true
  }

  const viteEnv = (import.meta as Record<string, any>).env || {}
  if (viteEnv.VITE_USE_TEST_AUTOMATION_MOCKS === 'true') {
    return true
  }

  if (typeof process !== 'undefined' && process.env?.VITE_USE_TEST_AUTOMATION_MOCKS === 'true') {
    return true
  }

  const globalFlags = globalThis as Record<string, any>
  if (globalFlags.WEB_UI_SANDBOX_USE_MOCKS === true) {
    return true
  }

  return false
}

class MockExecutionManager {
  private static currentExecution: TestExecution | null = null
  private static history: TestExecution[] = []

  private static readonly scenarios: TestScenario[] = [
    {
      id: 'scenario-1',
      name: 'Basic Sonicator Test',
      description: 'Test basic sonicator functionality',
      feature_file: 'sonicator.feature',
      feature_name: 'Sonicator Control',
      tags: ['basic', 'sonicator'],
      steps: [
        {
          step_type: 'Given',
          description: 'Sonicator is powered on',
          status: 'pending',
          pin_interactions: ['POWER_SENSE_4']
        },
        {
          step_type: 'When',
          description: 'Start command is sent',
          status: 'pending',
          pin_interactions: ['START_4']
        },
        {
          step_type: 'Then',
          description: 'Frequency lock is detected',
          status: 'pending',
          pin_interactions: ['FREQ_LOCK_4']
        }
      ],
      status: 'pending'
    },
    {
      id: 'scenario-2',
      name: 'Amplitude Control Test',
      description: 'Test amplitude control functionality',
      feature_file: 'amplitude.feature',
      feature_name: 'Amplitude Control',
      tags: ['amplitude', 'control'],
      steps: [
        {
          step_type: 'Given',
          description: 'Amplitude control is available',
          status: 'pending',
          pin_interactions: ['AMPLITUDE_ALL']
        },
        {
          step_type: 'When',
          description: 'Amplitude is set to 50%',
          status: 'pending',
          pin_interactions: ['AMPLITUDE_ALL']
        },
        {
          step_type: 'Then',
          description: 'Amplitude setting is confirmed',
          status: 'pending',
          pin_interactions: ['AMPLITUDE_ALL']
        }
      ],
      status: 'pending'
    }
  ]

  static getAvailableScenarios(): TestScenario[] {
    return this.scenarios.map(cloneTestScenario)
  }

  static getAvailableTags(): string[] {
    const tags = new Set<string>()
    this.scenarios.forEach(scenario => {
      scenario.tags.forEach(tag => tags.add(tag))
    })
    return Array.from(tags)
  }

  static getAvailableFeatures(): string[] {
    const features = new Set<string>()
    this.scenarios.forEach(scenario => {
      if (scenario.feature_file) {
        features.add(scenario.feature_file)
      }
      if (scenario.feature_name) {
        features.add(scenario.feature_name)
      }
    })
    return Array.from(features)
  }

  static executeScenarios(scenarioNames: string[], executionId: string): boolean {
    if (!scenarioNames.length) {
      return false
    }

    if (this.currentExecution && this.currentExecution.status === 'running') {
      return false
    }

    const selected = this.scenarios.filter(scenario => {
      const identifier = scenario.id || scenario.name
      return scenarioNames.includes(identifier) || scenarioNames.includes(scenario.name)
    })

    if (!selected.length) {
      return false
    }

    const now = Date.now()
    const execution: TestExecution = {
      execution_id: executionId || `exec_${now}`,
      scenarios: selected.map((scenario, index) => ({
        ...cloneTestScenario(scenario),
        status: index === 0 ? 'running' : 'pending'
      })),
      status: 'running',
      start_time: now,
      total_scenarios: selected.length,
      passed_scenarios: 0,
      failed_scenarios: 0,
      current_scenario_index: 0,
      current_step_index: 0
    }

    this.currentExecution = execution
    return true
  }

  static stopExecution(): boolean {
    if (!this.currentExecution) {
      return false
    }

    const wasRunning = this.currentExecution.status === 'running'

    if (wasRunning) {
      this.currentExecution.status = 'error'
    }

    this.currentExecution.end_time = Date.now()
    this.history.unshift(cloneTestExecution(this.currentExecution))

    return wasRunning
  }

  static getExecutionStatus(): ExecutionStatusPayload {
    return {
      execution: this.currentExecution ? cloneTestExecution(this.currentExecution) : null,
      in_progress: this.currentExecution?.status === 'running'
    }
  }

  static getCurrentExecution(): TestExecution | null {
    return this.currentExecution ? cloneTestExecution(this.currentExecution) : null
  }

  static getExecutionHistory(): TestExecution[] {
    return this.history.map(cloneTestExecution)
  }

  static isExecutionInProgress(): boolean {
    return this.currentExecution?.status === 'running'
  }
}

/**
 * Client for the Test Automation backend API with robust mock fallbacks.
 */
export class TestAutomationAPI {
  private static lastKnownExecution: TestExecution | null = null
  private static lastExecutionInProgress = false

  private static readonly SCENARIOS_ENDPOINT = '/api/test/scenarios'
  private static readonly SCENARIOS_BY_TAG_ENDPOINT = '/api/test/scenarios/tags'
  private static readonly EXECUTE_ENDPOINT = '/api/test/execute'
  private static readonly STOP_ENDPOINT = '/api/test/stop'
  private static readonly STATUS_ENDPOINT = '/api/test/execution'
  private static readonly TAGS_ENDPOINT = '/api/test/tags'
  private static readonly FEATURES_ENDPOINT = '/api/test/features'
  private static readonly HISTORY_ENDPOINT = '/api/test/history'

  private static async getJson<T>(endpoint: string): Promise<T | null> {
    try {
      const response = await fetch(endpoint)
      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`)
      }
      return await response.json() as T
    } catch (error) {
      console.warn(`[TestAutomationAPI] GET ${endpoint} failed`, error)
      return null
    }
  }

  private static updateLastKnownStatus(status: ExecutionStatusPayload): ExecutionStatusPayload {
    this.lastKnownExecution = status.execution ? cloneTestExecution(status.execution) : null
    this.lastExecutionInProgress = status.in_progress
    return status
  }

  private static getFallbackStatus(): ExecutionStatusPayload {
    const status = MockExecutionManager.getExecutionStatus()
    return this.updateLastKnownStatus(status)
  }

  /**
   * Fetches available test scenarios.
   */
  static async getAvailableScenarios(): Promise<TestScenario[]> {
    if (!shouldUseMockData()) {
      const response = await this.getJson<{ scenarios: TestScenario[] }>(this.SCENARIOS_ENDPOINT)
      if (response?.scenarios) {
        return response.scenarios
      }
    }

    return MockExecutionManager.getAvailableScenarios()
  }

  /**
   * Fetches available test tags.
   */
  static async getAvailableTags(): Promise<string[]> {
    if (!shouldUseMockData()) {
      const response = await this.getJson<{ tags: string[] }>(this.TAGS_ENDPOINT)
      if (response?.tags) {
        return response.tags
      }
    }

    return MockExecutionManager.getAvailableTags()
  }

  /**
   * Fetches available feature files.
   */
  static async getAvailableFeatures(): Promise<string[]> {
    if (!shouldUseMockData()) {
      const response = await this.getJson<{ features: string[] }>(this.FEATURES_ENDPOINT)
      if (response?.features) {
        return response.features
      }
    }

    return MockExecutionManager.getAvailableFeatures()
  }

  /**
   * Executes the provided scenarios using the backend or mock fallback.
   */
  static async executeScenarios(scenarioNames: string[], executionId: string): Promise<boolean> {
    if (!scenarioNames.length) {
      return false
    }

    if (!shouldUseMockData()) {
      try {
        const response = await fetch(this.EXECUTE_ENDPOINT, {
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
          throw new Error(`HTTP ${response.status}: ${response.statusText}`)
        }

        const payload = await response.json() as { success?: boolean }
        if (payload?.success) {
          this.lastExecutionInProgress = true
          return true
        }
      } catch (error) {
        console.warn('[TestAutomationAPI] executeScenarios falling back to mock', error)
      }
    }

    MockExecutionManager.executeScenarios(scenarioNames, executionId)
    const status = MockExecutionManager.getExecutionStatus()
    this.updateLastKnownStatus(status)
    return false
  }

  /**
   * Stops the current execution if one is running.
   */
  static async stopExecution(): Promise<boolean> {
    if (!shouldUseMockData()) {
      try {
        const response = await fetch(this.STOP_ENDPOINT, {
          method: 'POST'
        })

        if (!response.ok) {
          throw new Error(`HTTP ${response.status}: ${response.statusText}`)
        }

        const payload = await response.json() as { success?: boolean }
        if (typeof payload?.success === 'boolean') {
          this.lastExecutionInProgress = payload.success ? false : this.lastExecutionInProgress
          return payload.success
        }
      } catch (error) {
        console.warn('[TestAutomationAPI] stopExecution falling back to mock', error)
      }
    }

    MockExecutionManager.stopExecution()
    const status = MockExecutionManager.getExecutionStatus()
    this.updateLastKnownStatus(status)
    return false
  }

  /**
   * Retrieves scenarios filtered by tags with backend fallback.
   */
  static async getScenariosByTags(tags: string[]): Promise<TestScenario[]> {
    if (!tags.length) {
      return this.getAvailableScenarios()
    }

    if (!shouldUseMockData()) {
      const rawTags = tags.join(',')
      const response = await this.getJson<{ scenarios: TestScenario[] }>(`${this.SCENARIOS_BY_TAG_ENDPOINT}/${rawTags}`)
      if (response?.scenarios) {
        return response.scenarios
      }
    }

    const scenarios = MockExecutionManager.getAvailableScenarios()
    return filterScenariosByTagsHelper(scenarios, tags)
  }

  /**
   * Retrieves the current execution status.
   */
  static async getExecutionStatus(): Promise<ExecutionStatusPayload> {
    if (!shouldUseMockData()) {
      const response = await this.getJson<ExecutionStatusPayload>(this.STATUS_ENDPOINT)
      if (response) {
        return this.updateLastKnownStatus(response)
      }
    }

    if (this.lastKnownExecution || this.lastExecutionInProgress) {
      return {
        execution: this.lastKnownExecution ? cloneTestExecution(this.lastKnownExecution) : null,
        in_progress: this.lastExecutionInProgress
      }
    }

    return this.getFallbackStatus()
  }

  /**
   * Retrieves execution history entries.
   */
  static async getExecutionHistory(): Promise<TestExecution[]> {
    if (!shouldUseMockData()) {
      const response = await this.getJson<{ history: TestExecution[] }>(this.HISTORY_ENDPOINT)
      if (response?.history) {
        return response.history
      }
    }

    return MockExecutionManager.getExecutionHistory()
  }

  /**
   * Generates a unique execution identifier.
   */
  static generateExecutionId(): string {
    const timestamp = Date.now()
    const random = Math.random().toString(36).slice(2, 8)
    return `exec_${timestamp}_${random}`
  }

  /**
   * Formats a duration in milliseconds into a human readable string.
   */
  static formatDuration(durationMs: number): string {
    if (!durationMs || durationMs <= 0) {
      return '0s'
    }

    if (durationMs < 1000) {
      return `${durationMs}ms`
    }

    const totalSeconds = Math.floor(durationMs / 1000)
    const hours = Math.floor(totalSeconds / 3600)
    const minutes = Math.floor((totalSeconds % 3600) / 60)
    const seconds = totalSeconds % 60

    const parts: string[] = []
    if (hours > 0) {
      parts.push(`${hours}h`)
    }
    if (minutes > 0) {
      parts.push(`${minutes}m`)
    }
    if (seconds > 0) {
      parts.push(`${seconds}s`)
    }

    return parts.length ? parts.join(' ') : '0s'
  }

  /**
   * Resolves a display color for the provided execution status.
   */
  static getStatusColor(status: string): string {
    switch (status) {
      case 'passed':
        return '#22c55e'
      case 'failed':
        return '#ef4444'
      case 'running':
        return '#3b82f6'
      default:
        return '#6b7280'
    }
  }

  /**
   * Resolves an icon for the provided execution status.
   */
  static getStatusIcon(status: string): string {
    switch (status) {
      case 'passed':
        return '✅'
      case 'failed':
        return '❌'
      case 'running':
        return '🔄'
      default:
        return '❓'
    }
  }

  /**
   * Filters scenarios by the supplied feature files.
   */
  static filterScenariosByFeature(scenarios: TestScenario[], featureFiles: string[]): TestScenario[] {
    return filterScenariosByFeatureHelper(scenarios, featureFiles)
  }

  /**
   * Filters scenarios by the supplied tags.
   */
  static filterScenariosByTags(scenarios: TestScenario[], tags: string[]): TestScenario[] {
    return filterScenariosByTagsHelper(scenarios, tags)
  }

  /**
   * Retrieves scenarios that interact with the provided pins.
   */
  static getScenariosByPins(scenarios: TestScenario[], pins: string[]): TestScenario[] {
    return getScenariosByPinsHelper(scenarios, pins)
  }

  /**
   * Retrieves all pins touched by a scenario.
   */
  static getScenarioPins(scenario: TestScenario): string[] {
    return getScenarioPinsHelper(scenario)
  }

  /**
   * Calculates execution progress as a percentage of completed scenarios.
   */
  static getExecutionProgress(execution: TestExecution): number {
    const total = execution.total_scenarios || execution.scenarios.length
    if (total === 0) {
      return 0
    }
    const completed = Math.max(0, execution.passed_scenarios + execution.failed_scenarios)
    return Math.min(100, Math.round((completed / total) * 100))
  }

  /**
   * Calculates the current step progress for the active scenario.
   */
  static getCurrentStepProgress(execution: TestExecution): { current: number; total: number; percentage: number } {
    const currentScenario = execution.scenarios[execution.current_scenario_index]
    if (!currentScenario || !currentScenario.steps?.length) {
      return { current: 0, total: 0, percentage: 0 }
    }

    const totalSteps = currentScenario.steps.length
    const currentIndex = Math.min(totalSteps - 1, Math.max(0, execution.current_step_index ?? 0))
    const currentStepNumber = currentScenario.status === 'passed' ? totalSteps : currentIndex + 1
    const percentage = Math.min(100, Math.round((currentStepNumber / totalSteps) * 100))

    return {
      current: currentStepNumber,
      total: totalSteps,
      percentage
    }
  }
}
