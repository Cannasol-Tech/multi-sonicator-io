import { spawn, ChildProcess } from 'child_process'
import { EventEmitter } from 'events'
import path from 'path'

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

  constructor() {
    super()
    this.pythonServicePath = path.join(__dirname, 'TestAutomationService.py')
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
   * Get available test scenarios by calling Python service
   */
  async getAvailableScenarios(): Promise<TestScenario[]> {
    if (this.cachedScenarios.length > 0) {
      return this.cachedScenarios
    }

    return new Promise((resolve, reject) => {
      const pythonProcess = spawn('python3', [this.pythonServicePath, 'get_scenarios'])
      
      let output = ''
      let errorOutput = ''

      pythonProcess.stdout?.on('data', (data) => {
        output += data.toString()
      })

      pythonProcess.stderr?.on('data', (data) => {
        errorOutput += data.toString()
      })

      pythonProcess.on('close', (code) => {
        if (code === 0) {
          try {
            const scenarios = JSON.parse(output)
            this.cachedScenarios = scenarios
            this.extractAvailableTags(scenarios)
            resolve(scenarios)
          } catch (error) {
            reject(new Error(`Failed to parse scenarios: ${error}`))
          }
        } else {
          reject(new Error(`Python process failed: ${errorOutput}`))
        }
      })

      pythonProcess.on('error', (error) => {
        reject(new Error(`Failed to start Python process: ${error.message}`))
      })
    })
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
    if (this.pythonProcess) {
      console.log('Test execution already in progress')
      return false
    }

    try {
      // Start Python service for test execution
      this.pythonProcess = spawn('python3', [
        this.pythonServicePath, 
        'execute_scenarios',
        JSON.stringify(scenarioNames),
        executionId
      ])

      let output = ''
      let errorOutput = ''

      this.pythonProcess.stdout?.on('data', (data) => {
        const dataStr = data.toString()
        output += dataStr
        
        // Try to parse progress updates
        const lines = dataStr.split('\n')
        for (const line of lines) {
          if (line.trim().startsWith('{')) {
            try {
              const progressUpdate = JSON.parse(line.trim())
              this.handleProgressUpdate(progressUpdate)
            } catch (e) {
              // Not JSON, ignore
            }
          }
        }
      })

      this.pythonProcess.stderr?.on('data', (data) => {
        errorOutput += data.toString()
        console.error('Python service error:', data.toString())
      })

      this.pythonProcess.on('close', (code) => {
        console.log(`Test execution completed with code: ${code}`)
        this.pythonProcess = null
        
        if (this.currentExecution) {
          this.currentExecution.status = code === 0 ? 'passed' : 'failed'
          this.currentExecution.end_time = Date.now()
          this.emit('execution_complete', this.currentExecution)
        }
      })

      this.pythonProcess.on('error', (error) => {
        console.error('Python process error:', error)
        this.pythonProcess = null
        this.emit('execution_error', error.message)
      })

      return true

    } catch (error) {
      console.error('Failed to start test execution:', error)
      return false
    }
  }

  /**
   * Handle progress updates from Python service
   */
  private handleProgressUpdate(progressData: any) {
    try {
      this.currentExecution = progressData as TestExecution
      this.emit('progress_update', this.currentExecution)
      
      // Add to history when execution completes
      if (this.currentExecution.status === 'passed' || 
          this.currentExecution.status === 'failed' || 
          this.currentExecution.status === 'error') {
        this.addExecutionToHistory({ ...this.currentExecution })
      }
    } catch (error) {
      console.error('Error handling progress update:', error)
    }
  }

  /**
   * Stop current test execution
   */
  stopExecution(): boolean {
    if (this.pythonProcess) {
      this.pythonProcess.kill('SIGTERM')
      this.pythonProcess = null
      
      if (this.currentExecution) {
        this.currentExecution.status = 'error'
        this.currentExecution.error_message = 'Execution stopped by user'
        this.currentExecution.end_time = Date.now()
        this.emit('execution_stopped', this.currentExecution)
      }
      
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
   * Add execution to history
   */
  private addExecutionToHistory(execution: TestExecution) {
    this.executionHistory.push(execution)
    // Keep only last 50 executions
    if (this.executionHistory.length > 50) {
      this.executionHistory = this.executionHistory.slice(-50)
    }
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
