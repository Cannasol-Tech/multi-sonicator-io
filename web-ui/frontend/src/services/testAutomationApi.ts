// Test Automation API Service
// Provides mock data and API integration for test automation features

import { TestExecution, TestScenario, TestStep } from '../types'

export class TestAutomationAPI {
  // Mock execution state for development
  private static MockExecutionManager = class {
    private static currentExecution: TestExecution | null = null;
    private static scenarios: TestScenario[] = [
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
    ];

    static getAvailableScenarios(): TestScenario[] {
      return this.scenarios;
    }

    static getAvailableTags(): string[] {
      const tags = new Set<string>();
      this.scenarios.forEach(scenario => {
        scenario.tags.forEach(tag => tags.add(tag));
      });
      return Array.from(tags);
    }

    static getAvailableFeatures(): string[] {
      const features = new Set<string>();
      this.scenarios.forEach(scenario => {
        if (scenario.feature_name) {
          features.add(scenario.feature_name);
        }
      });
      return Array.from(features);
    }

    static startExecution(scenarioIds: string[]): TestExecution {
      const selectedScenarios = this.scenarios.filter(s => scenarioIds.includes(s.id || ''));

      this.currentExecution = {
        execution_id: `exec-${Date.now()}`,
        scenarios: selectedScenarios,
        status: 'running',
        start_time: Date.now(),
        total_scenarios: selectedScenarios.length,
        passed_scenarios: 0,
        failed_scenarios: 0,
        current_scenario_index: 0
      };

      return this.currentExecution;
    }

    static getCurrentExecution(): TestExecution | null {
      return this.currentExecution;
    }

    static stopExecution(): void {
      if (this.currentExecution) {
        this.currentExecution.status = 'error';
        this.currentExecution.end_time = Date.now();
      }
    }
  };

  // API Methods
  static async getAvailableScenarios(): Promise<TestScenario[]> {
    // In development, return mock data
    return Promise.resolve(this.MockExecutionManager.getAvailableScenarios());
  }

  static async getAvailableTags(): Promise<string[]> {
    return Promise.resolve(this.MockExecutionManager.getAvailableTags());
  }

  static async getAvailableFeatures(): Promise<string[]> {
    return Promise.resolve(this.MockExecutionManager.getAvailableFeatures());
  }

  static async startExecution(scenarioIds: string[]): Promise<TestExecution> {
    return Promise.resolve(this.MockExecutionManager.startExecution(scenarioIds));
  }

  static async getCurrentExecution(): Promise<TestExecution | null> {
    return Promise.resolve(this.MockExecutionManager.getCurrentExecution());
  }

  static async stopExecution(): Promise<void> {
    this.MockExecutionManager.stopExecution();
    return Promise.resolve();
  }

  static async getExecutionHistory(): Promise<TestExecution[]> {
    // Mock history data
    return Promise.resolve([]);
  }
}
