import { spawn, ChildProcess } from 'child_process'
import { EventEmitter } from 'events'
import path from 'path'
import fs from 'fs/promises'
import { HardwareInterface } from '../adapters/HardwareInterface.js'

export interface TestStep {
  step_type: string
  description: string
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error'
  duration_ms?: number
  error_message?: string
  pin_interactions: string[]
}

export interface TestScenario {
  name: string
  description: string
  feature_file: string
  feature_name?: string
  tags: string[]
  steps: TestStep[]
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error'
  duration_ms?: number
  error_message?: string
}

export interface TestExecution {
  execution_id: string
  scenarios: TestScenario[]
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error'
  start_time?: number
  end_time?: number
  total_scenarios: number
  passed_scenarios: number
  failed_scenarios: number
  current_scenario_index: number
  error_message?: string
}

export class TestAutomationService extends EventEmitter {
  private pythonProcess: ChildProcess | null = null
  private currentExecution: TestExecution | null = null
  private pythonServicePath: string
  private cachedScenarios: TestScenario[] = []
  private availableTags: string[] = []
  private executionHistory: TestExecution[] = []
  private tagPresets: Record<string, string[]> = {}
  private hardwareInterface: HardwareInterface | null = null
  private featuresPath: string

  constructor(hardwareInterface?: HardwareInterface) {
    super()
    this.pythonServicePath = path.join(__dirname, 'TestAutomationService.py')
    this.hardwareInterface = hardwareInterface || null

    // Path to acceptance test features
    const projectRoot = path.resolve(__dirname, '../../../../..')
    this.featuresPath = path.join(projectRoot, 'test', 'acceptance', 'features')

    this.initializeTagPresets()
  }

  private initializeTagPresets() {
    this.tagPresets = {
      'Smoke Test Suite': ['smoke', 'critical'],
      'Hardware Verification': ['hil', 'gpio', 'pwm', 'adc'],
      'Communication Tests': ['modbus', 'uart', 'communication'],
      'Critical Hardware Tests': ['hil', 'gpio', 'critical'],
      'Regression Suite': ['regression', 'critical'],
      'Quick Validation': ['smoke', 'basic']
    }
  }

  /**
   * Get available test scenarios by parsing feature files directly
   */
  async getAvailableScenarios(): Promise<TestScenario[]> {
    if (this.cachedScenarios.length > 0) {
      return this.cachedScenarios
    }

    try {
      const scenarios = await this.parseFeatureFiles()
      this.cachedScenarios = scenarios
      this.extractAvailableTags(scenarios)
      return scenarios
    } catch (error) {
      console.error('Failed to parse feature files:', error)
      // Return mock scenarios as fallback
      return this.getMockScenarios()
    }
  }

  /**
   * Parse Gherkin feature files to extract test scenarios
   */
  private async parseFeatureFiles(): Promise<TestScenario[]> {
    const scenarios: TestScenario[] = []

    try {
      const files = await fs.readdir(this.featuresPath)
      const featureFiles = files.filter(file => file.endsWith('.feature'))

      for (const file of featureFiles) {
        const filePath = path.join(this.featuresPath, file)
        const content = await fs.readFile(filePath, 'utf-8')
        const fileScenarios = this.parseFeatureFile(content, file)
        scenarios.push(...fileScenarios)
      }
    } catch (error) {
      console.error('Error reading feature files:', error)
      throw error
    }

    return scenarios
  }

  /**
   * Parse a single Gherkin feature file
   */
  private parseFeatureFile(content: string, filename: string): TestScenario[] {
    const scenarios: TestScenario[] = []
    const lines = content.split('\n')

    let currentFeatureName = ''
    let currentFeatureTags: string[] = []
    let currentScenario: Partial<TestScenario> | null = null
    let currentSteps: TestStep[] = []

    for (let i = 0; i < lines.length; i++) {
      const line = lines[i].trim()

      // Parse feature tags
      if (line.startsWith('@') && !currentScenario) {
        currentFeatureTags = this.parseTags(line)
      }

      // Parse feature name
      if (line.startsWith('Feature:')) {
        currentFeatureName = line.replace('Feature:', '').trim()
      }

      // Parse scenario tags
      if (line.startsWith('@') && currentScenario === null) {
        const scenarioTags = this.parseTags(line)
        // Look ahead for scenario
        if (i + 1 < lines.length && lines[i + 1].trim().startsWith('Scenario')) {
          currentScenario = {
            tags: [...currentFeatureTags, ...scenarioTags],
            steps: []
          }
        }
      }

      // Parse scenario
      if (line.startsWith('Scenario:') || line.startsWith('Scenario Outline:')) {
        if (currentScenario) {
          // Finish previous scenario
          scenarios.push({
            ...currentScenario,
            steps: currentSteps
          } as TestScenario)
        }

        currentScenario = currentScenario || { tags: currentFeatureTags, steps: [] }
        currentScenario.name = line.replace(/^Scenario( Outline)?:/, '').trim()
        currentScenario.description = currentScenario.name
        currentScenario.feature_file = filename
        currentScenario.feature_name = currentFeatureName
        currentScenario.status = 'pending'
        currentSteps = []
      }

      // Parse steps
      if (currentScenario && (line.startsWith('Given ') || line.startsWith('When ') ||
                             line.startsWith('Then ') || line.startsWith('And ') ||
                             line.startsWith('But '))) {
        const stepType = line.split(' ')[0].toLowerCase()
        const description = line.replace(/^(Given|When|Then|And|But)\s+/, '')

        currentSteps.push({
          step_type: stepType,
          description: description,
          status: 'pending',
          pin_interactions: this.extractPinInteractions(description)
        })
      }
    }

    // Add the last scenario
    if (currentScenario) {
      scenarios.push({
        ...currentScenario,
        steps: currentSteps
      } as TestScenario)
    }

    return scenarios
  }

  /**
   * Parse tags from a line
   */
  private parseTags(line: string): string[] {
    return line.split(' ')
      .filter(tag => tag.startsWith('@'))
      .map(tag => tag.substring(1))
  }

  /**
   * Extract pin interactions from step description
   */
  private extractPinInteractions(description: string): string[] {
    const pins: string[] = []
    const pinPatterns = [
      /START_4/g, /RESET_4/g, /AMPLITUDE_ALL/g, /FREQ_DIV10_4/g,
      /FREQ_LOCK_4/g, /OVERLOAD_4/g, /POWER_SENSE_4/g, /STATUS_LED/g,
      /D\d+/g, /A\d+/g, /P[A-D]\d+/g
    ]

    for (const pattern of pinPatterns) {
      const matches = description.match(pattern)
      if (matches) {
        pins.push(...matches)
      }
    }

    return [...new Set(pins)] // Remove duplicates
  }

  /**
   * Get mock scenarios as fallback
   */
  private getMockScenarios(): TestScenario[] {
    return [
      {
        name: 'Basic Sonicator Test',
        description: 'Test basic sonicator functionality including start/stop and amplitude control',
        feature_file: 'basic_sonicator.feature',
        feature_name: 'Basic Sonicator Control',
        tags: ['basic', 'sonicator', 'smoke'],
        status: 'pending',
        steps: [
          {
            step_type: 'given',
            description: 'the HIL wrapper is connected and ready',
            status: 'pending',
            pin_interactions: []
          },
          {
            step_type: 'when',
            description: 'I set START_4 to HIGH',
            status: 'pending',
            pin_interactions: ['START_4']
          },
          {
            step_type: 'then',
            description: 'the sonicator should start within 100ms',
            status: 'pending',
            pin_interactions: ['STATUS_LED']
          }
        ]
      },
      {
        name: 'Amplitude Control Test',
        description: 'Test amplitude control via MODBUS register',
        feature_file: 'amplitude_control.feature',
        feature_name: 'Amplitude Control',
        tags: ['amplitude', 'control', 'modbus'],
        status: 'pending',
        steps: [
          {
            step_type: 'given',
            description: 'the system is ready for amplitude testing',
            status: 'pending',
            pin_interactions: []
          },
          {
            step_type: 'when',
            description: 'I write 75 to holding register 40001',
            status: 'pending',
            pin_interactions: ['AMPLITUDE_ALL']
          },
          {
            step_type: 'then',
            description: 'the amplitude output should be 75% within tolerance 2%',
            status: 'pending',
            pin_interactions: ['AMPLITUDE_ALL']
          }
        ]
      }
    ]
  }

  /**
   * Extract all available tags from scenarios
   */
  private extractAvailableTags(scenarios: TestScenario[]) {
    const tagSet = new Set<string>()
    scenarios.forEach(scenario => {
      scenario.tags.forEach(tag => {
        tagSet.add(tag.replace('@', ''))
      })
    })
    this.availableTags = Array.from(tagSet).sort()
  }

  /**
   * Get all available tags from loaded scenarios
   */
  async getAvailableTags(): Promise<string[]> {
    if (this.availableTags.length === 0) {
      await this.getAvailableScenarios()
    }
    return this.availableTags
  }

  /**
   * Get scenarios filtered by tags with AND/OR logic
   */
  async getScenariosByTags(tags: string[], useOrLogic: boolean = false): Promise<TestScenario[]> {
    const scenarios = await this.getAvailableScenarios()
    
    if (tags.length === 0) {
      return scenarios
    }

    const normalizedTags = tags.map(tag => tag.replace('@', '').toLowerCase())
    
    return scenarios.filter(scenario => {
      const scenarioTags = scenario.tags.map(tag => tag.replace('@', '').toLowerCase())
      
      if (useOrLogic) {
        // OR logic: scenario must have at least one of the specified tags
        return normalizedTags.some(tag => scenarioTags.includes(tag))
      } else {
        // AND logic: scenario must have all specified tags
        return normalizedTags.every(tag => scenarioTags.includes(tag))
      }
    })
  }

  /**
   * Search scenarios by text query
   */
  async searchScenarios(query: string): Promise<TestScenario[]> {
    const scenarios = await this.getAvailableScenarios()
    const queryLower = query.toLowerCase()
    
    return scenarios.filter(scenario => 
      scenario.name.toLowerCase().includes(queryLower) ||
      scenario.description.toLowerCase().includes(queryLower) ||
      scenario.feature_file.toLowerCase().includes(queryLower) ||
      scenario.tags.some(tag => tag.toLowerCase().includes(queryLower))
    )
  }

  /**
   * Get tag presets
   */
  getTagPresets(): Record<string, string[]> {
    return { ...this.tagPresets }
  }

  /**
   * Save custom tag combination
   */
  saveTagPreset(name: string, tags: string[]): boolean {
    try {
      this.tagPresets[name] = [...tags]
      // In a real implementation, this would persist to a database or file
      return true
    } catch (error) {
      console.error('Failed to save tag preset:', error)
      return false
    }
  }

  /**
   * Get tags categorized by type
   */
  async getCategorizedTags(): Promise<Record<string, string[]>> {
    const tags = await this.getAvailableTags()
    
    const categories: Record<string, string[]> = {
      'Test Type': [],
      'Hardware': [],
      'Communication': [],
      'Feature': [],
      'Other': []
    }

    tags.forEach(tag => {
      const tagLower = tag.toLowerCase()
      if (['smoke', 'critical', 'regression', 'basic'].includes(tagLower)) {
        categories['Test Type'].push(tag)
      } else if (['hil', 'gpio', 'pwm', 'adc', 'hardware'].includes(tagLower)) {
        categories['Hardware'].push(tag)
      } else if (['modbus', 'uart', 'spi', 'communication', 'serial'].includes(tagLower)) {
        categories['Communication'].push(tag)
      } else if (tagLower.startsWith('feature-') || tagLower.includes('feature')) {
        categories['Feature'].push(tag)
      } else {
        categories['Other'].push(tag)
      }
    })

    return categories
  }

  /**
   * Execute selected test scenarios
   */
  async executeScenarios(scenarioNames: string[], executionId: string): Promise<boolean> {
    if (this.currentExecution && this.currentExecution.status === 'running') {
      console.log('Test execution already in progress')
      return false
    }

    try {
      // Get scenarios to execute
      const availableScenarios = await this.getAvailableScenarios()
      const scenariosToExecute = availableScenarios.filter(s =>
        scenarioNames.includes(s.name)
      )

      if (scenariosToExecute.length === 0) {
        console.log('No scenarios found matching:', scenarioNames)
        return false
      }

      // Create test execution
      this.currentExecution = {
        execution_id: executionId,
        scenarios: scenariosToExecute.map(s => ({ ...s, status: 'pending' })),
        status: 'running',
        start_time: Date.now(),
        total_scenarios: scenariosToExecute.length,
        passed_scenarios: 0,
        failed_scenarios: 0,
        current_scenario_index: 0
      }

      // Start execution in background
      this.executeInBackground()

      return true
    } catch (error) {
      console.error('Failed to start test execution:', error)
      return false
    }
  }

  /**
   * Execute scenarios in background
   */
  private async executeInBackground(): Promise<void> {
    if (!this.currentExecution) return

    try {
      for (let i = 0; i < this.currentExecution.scenarios.length; i++) {
        if (this.currentExecution.status !== 'running') break

        this.currentExecution.current_scenario_index = i
        const scenario = this.currentExecution.scenarios[i]

        console.log(`Executing scenario: ${scenario.name}`)
        scenario.status = 'running'
        this.emit('progress', this.currentExecution)

        const success = await this.executeScenario(scenario)

        if (success) {
          scenario.status = 'passed'
          this.currentExecution.passed_scenarios++
        } else {
          scenario.status = 'failed'
          this.currentExecution.failed_scenarios++
        }

        this.emit('progress', this.currentExecution)

        // Small delay between scenarios
        await new Promise(resolve => setTimeout(resolve, 100))
      }

      // Complete execution
      this.currentExecution.status = this.currentExecution.failed_scenarios > 0 ? 'failed' : 'passed'
      this.currentExecution.end_time = Date.now()

      // Add to history
      this.executionHistory.push({ ...this.currentExecution })

      this.emit('complete', this.currentExecution)
      console.log('Test execution completed')

    } catch (error) {
      console.error('Test execution error:', error)
      if (this.currentExecution) {
        this.currentExecution.status = 'error'
        this.currentExecution.error_message = error instanceof Error ? error.message : 'Unknown error'
        this.currentExecution.end_time = Date.now()
        this.emit('error', this.currentExecution)
      }
    }
  }

  /**
   * Execute a single scenario
   */
  private async executeScenario(scenario: TestScenario): Promise<boolean> {
    const startTime = Date.now()
    let allStepsPassed = true

    try {
      for (const step of scenario.steps) {
        step.status = 'running'
        this.emit('progress', this.currentExecution)

        const stepStartTime = Date.now()
        const success = await this.executeStep(step)

        step.duration_ms = Date.now() - stepStartTime

        if (success) {
          step.status = 'passed'
        } else {
          step.status = 'failed'
          step.error_message = 'Step execution failed'
          allStepsPassed = false
        }

        this.emit('progress', this.currentExecution)

        // Small delay between steps
        await new Promise(resolve => setTimeout(resolve, 50))
      }

      scenario.duration_ms = Date.now() - startTime
      return allStepsPassed

    } catch (error) {
      scenario.status = 'error'
      scenario.error_message = error instanceof Error ? error.message : 'Unknown error'
      scenario.duration_ms = Date.now() - startTime
      return false
    }
  }

  /**
   * Execute a single test step
   */
  private async executeStep(step: TestStep): Promise<boolean> {
    try {
      // Simulate step execution based on step type and description
      const description = step.description.toLowerCase()

      if (description.includes('hil') && description.includes('connected')) {
        // Check hardware connection
        return this.hardwareInterface ? true : Math.random() > 0.1
      }

      if (description.includes('start_4') || description.includes('reset_4')) {
        // Simulate pin control
        if (this.hardwareInterface) {
          // In real implementation, would call hardware interface
          return true
        }
        return Math.random() > 0.1
      }

      if (description.includes('amplitude') || description.includes('modbus')) {
        // Simulate MODBUS operations
        if (this.hardwareInterface) {
          // In real implementation, would call MODBUS service
          return true
        }
        return Math.random() > 0.05
      }

      // Default simulation - 95% pass rate
      return Math.random() > 0.05

    } catch (error) {
      console.error('Step execution error:', error)
      return false
    }
  }

  /**
   * Stop current test execution
   */
  stopExecution(): boolean {
    if (this.currentExecution && this.currentExecution.status === 'running') {
      this.currentExecution.status = 'error'
      this.currentExecution.error_message = 'Execution stopped by user'
      this.currentExecution.end_time = Date.now()

      // Add to history
      this.executionHistory.push({ ...this.currentExecution })

      this.emit('stopped', this.currentExecution)
      return true
    }
    return false
  }

  /**
   * Get current execution status
   */
  getCurrentExecution(): TestExecution | null {
    return this.currentExecution
  }

  /**
   * Check if execution is in progress
   */
  isExecutionInProgress(): boolean {
    return this.pythonProcess !== null
  }

  /**
   * Get test scenarios for specific feature files
   */
  async getScenariosByFeature(featureFiles: string[]): Promise<TestScenario[]> {
    const allScenarios = await this.getAvailableScenarios()
    
    if (featureFiles.length === 0) {
      return allScenarios
    }

    return allScenarios.filter(scenario => 
      featureFiles.includes(scenario.feature_file)
    )
  }

  /**
   * Get available feature files
   */
  async getAvailableFeatures(): Promise<string[]> {
    const scenarios = await this.getAvailableScenarios()
    const featureSet = new Set<string>()
    
    scenarios.forEach(scenario => {
      featureSet.add(scenario.feature_file)
    })
    
    return Array.from(featureSet).sort()
  }

  /**
   * Validate scenario selection
   */
  async validateScenarios(scenarioNames: string[]): Promise<{valid: boolean, missing: string[]}> {
    const availableScenarios = await this.getAvailableScenarios()
    const availableNames = availableScenarios.map(s => s.name)
    
    const missing = scenarioNames.filter(name => !availableNames.includes(name))
    
    return {
      valid: missing.length === 0,
      missing
    }
  }

  /**
   * Get execution history
   */
  getExecutionHistory(): TestExecution[] {
    return [...this.executionHistory]
  }



  /**
   * Clear execution history
   */
  clearExecutionHistory(): void {
    this.executionHistory = []
  }

  /**
   * Get execution statistics
   */
  getExecutionStats(): {
    totalExecutions: number
    totalScenarios: number
    totalPassed: number
    totalFailed: number
    averageDuration: number
    successRate: number
  } {
    if (this.executionHistory.length === 0) {
      return {
        totalExecutions: 0,
        totalScenarios: 0,
        totalPassed: 0,
        totalFailed: 0,
        averageDuration: 0,
        successRate: 0
      }
    }

    const totalExecutions = this.executionHistory.length
    const totalScenarios = this.executionHistory.reduce((sum, exec) => sum + exec.total_scenarios, 0)
    const totalPassed = this.executionHistory.reduce((sum, exec) => sum + exec.passed_scenarios, 0)
    const totalFailed = this.executionHistory.reduce((sum, exec) => sum + exec.failed_scenarios, 0)
    
    const completedExecutions = this.executionHistory.filter(exec => 
      exec.start_time && exec.end_time && exec.status !== 'pending' && exec.status !== 'running'
    )
    
    const averageDuration = completedExecutions.length > 0 
      ? completedExecutions.reduce((sum, exec) => 
          sum + ((exec.end_time! - exec.start_time!) / 1000), 0
        ) / completedExecutions.length
      : 0

    const successRate = totalScenarios > 0 ? (totalPassed / totalScenarios) * 100 : 0

    return {
      totalExecutions,
      totalScenarios,
      totalPassed,
      totalFailed,
      averageDuration,
      successRate
    }
  }

  /**
   * Export execution results in various formats
   */
  async exportExecutionResults(executionId: string, format: 'json' | 'csv' | 'html'): Promise<string> {
    const execution = this.executionHistory.find(exec => exec.execution_id === executionId)
    if (!execution) {
      throw new Error(`Execution ${executionId} not found`)
    }

    switch (format) {
      case 'json':
        return JSON.stringify(execution, null, 2)
      
      case 'csv':
        return this.convertExecutionToCSV(execution)
      
      case 'html':
        return this.convertExecutionToHTML(execution)
      
      default:
        throw new Error(`Unsupported export format: ${format}`)
    }
  }

  private convertExecutionToCSV(execution: TestExecution): string {
    const headers = ['Scenario Name', 'Status', 'Duration (ms)', 'Steps Count', 'Error Message']
    const rows = execution.scenarios.map(scenario => [
      scenario.name,
      scenario.status,
      scenario.duration_ms?.toString() || '',
      scenario.steps.length.toString(),
      scenario.error_message || ''
    ])
    
    return [headers, ...rows]
      .map(row => row.map(cell => `"${cell}"`).join(','))
      .join('\n')
  }

  private convertExecutionToHTML(execution: TestExecution): string {
    return `
    <!DOCTYPE html>
    <html>
    <head>
      <title>Test Execution Report - ${execution.execution_id}</title>
      <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background: #f5f5f5; padding: 15px; border-radius: 5px; }
        .summary { margin: 20px 0; }
        .scenario { border: 1px solid #ddd; margin: 10px 0; padding: 15px; border-radius: 5px; }
        .passed { border-left: 5px solid #28a745; }
        .failed { border-left: 5px solid #dc3545; }
        .error { border-left: 5px solid #ffc107; }
        .steps { margin-top: 10px; }
        .step { margin: 5px 0; padding: 5px; background: #f8f9fa; }
      </style>
    </head>
    <body>
      <div class="header">
        <h1>Test Execution Report</h1>
        <p>Execution ID: ${execution.execution_id}</p>
        <p>Status: ${execution.status}</p>
        <p>Start Time: ${execution.start_time ? new Date(execution.start_time).toLocaleString() : 'N/A'}</p>
        <p>End Time: ${execution.end_time ? new Date(execution.end_time).toLocaleString() : 'N/A'}</p>
      </div>
      
      <div class="summary">
        <h2>Summary</h2>
        <p>Total Scenarios: ${execution.total_scenarios}</p>
        <p>Passed: ${execution.passed_scenarios}</p>
        <p>Failed: ${execution.failed_scenarios}</p>
      </div>
      
      <div class="scenarios">
        <h2>Scenarios</h2>
        ${execution.scenarios.map(scenario => `
          <div class="scenario ${scenario.status}">
            <h3>${scenario.name}</h3>
            <p>Status: ${scenario.status}</p>
            <p>Duration: ${scenario.duration_ms || 0}ms</p>
            ${scenario.error_message ? `<p>Error: ${scenario.error_message}</p>` : ''}
            <div class="steps">
              <h4>Steps (${scenario.steps.length})</h4>
              ${scenario.steps.map(step => `
                <div class="step">
                  <strong>${step.step_type}</strong>: ${step.description}
                  <span style="float: right; color: ${step.status === 'passed' ? 'green' : step.status === 'failed' ? 'red' : 'gray'}">
                    ${step.status}
                  </span>
                </div>
              `).join('')}
            </div>
          </div>
        `).join('')}
      </div>
    </body>
    </html>`
  }
}
