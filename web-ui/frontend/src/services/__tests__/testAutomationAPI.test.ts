import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { TestAutomationAPI } from '../testAutomationAPI'

// Mock fetch
global.fetch = vi.fn()

// Mock environment to force non-mock behavior for testing
const originalEnv = process.env.NODE_ENV
const mockShouldUseMockData = vi.fn()

// Mock the shouldUseMockData function
vi.mock('../testAutomationAPI', async () => {
  const actual = await vi.importActual('../testAutomationAPI')
  return {
    ...actual,
    shouldUseMockData: () => mockShouldUseMockData()
  }
})

describe('Test Automation API', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    // Reset fetch mock
    vi.mocked(fetch).mockClear()
    // Default to not using mock data for testing API behavior
    mockShouldUseMockData.mockReturnValue(false)
  })

  afterEach(() => {
    process.env.NODE_ENV = originalEnv
  })

  describe('getAvailableScenarios', () => {
    it('fetches scenarios successfully', async () => {
      const mockResponse = {
        scenarios: [
          {
            id: 'scenario-1',
            name: 'Basic Pin Test',
            description: 'Test basic pin operations',
            feature_file: 'pin-control.feature',
            tags: ['@smoke', '@pin'],
            steps: [
              {
                step_type: 'Given',
                description: 'the Arduino is connected',
                pin_interactions: [],
                status: 'pending'
              }
            ]
          }
        ]
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockResponse,
      } as Response)

      const result = await TestAutomationAPI.getAvailableScenarios()

      expect(fetch).toHaveBeenCalledWith('/api/test/scenarios')
      expect(result).toEqual(mockResponse.scenarios)
    })

    it('falls back to mock data on fetch errors', async () => {
      vi.mocked(fetch).mockRejectedValueOnce(new Error('Network error'))

      const result = await TestAutomationAPI.getAvailableScenarios()

      // Should return mock data instead of throwing
      expect(result).toBeInstanceOf(Array)
      expect(result.length).toBeGreaterThan(0)
    })

    it('falls back to mock data on HTTP errors', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 500,
        statusText: 'Internal Server Error',
      } as Response)

      const result = await TestAutomationAPI.getAvailableScenarios()

      // Should return mock data instead of throwing
      expect(result).toBeInstanceOf(Array)
      expect(result.length).toBeGreaterThan(0)
    })

    it('falls back to mock data on invalid JSON response', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => {
          throw new Error('Invalid JSON')
        },
      } as Response)

      const result = await TestAutomationAPI.getAvailableScenarios()

      // Should return mock data instead of throwing
      expect(result).toBeInstanceOf(Array)
      expect(result.length).toBeGreaterThan(0)
    })
  })

  describe('executeScenarios', () => {
    it('starts scenario execution successfully', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => ({ success: true }),
      } as Response)

      const scenarioNames = ['scenario-1', 'scenario-2']
      const executionId = 'exec-123'
      const result = await TestAutomationAPI.executeScenarios(scenarioNames, executionId)

      expect(fetch).toHaveBeenCalledWith('/api/test/execute', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ scenarios: scenarioNames, execution_id: executionId })
      })
      expect(result).toBe(true)
    })

    it('falls back to mock execution with empty scenarios', async () => {
      // The actual API doesn't validate empty arrays, it falls back to mock execution
      const result = await TestAutomationAPI.executeScenarios([], 'exec-123')

      // Should return false from mock execution manager
      expect(result).toBe(false)
    })

    it('falls back to mock execution on HTTP errors', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 400,
        statusText: 'Bad Request',
      } as Response)

      const result = await TestAutomationAPI.executeScenarios(['scenario-1'], 'exec-123')

      // Should fall back to mock execution instead of throwing
      expect(result).toBe(false)
    })

    it('uses correct request format', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => ({ success: true }),
      } as Response)

      await TestAutomationAPI.executeScenarios(['scenario-1'], 'exec-123')

      expect(fetch).toHaveBeenCalledWith('/api/test/execute', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          scenarios: ['scenario-1'],
          execution_id: 'exec-123'
        })
      })
    })
  })

  describe('stopExecution', () => {
    it('stops execution successfully', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => ({ success: true }),
      } as Response)

      const result = await TestAutomationAPI.stopExecution()

      expect(fetch).toHaveBeenCalledWith('/api/test/stop', {
        method: 'POST'
      })
      expect(result).toBe(true)
    })

    it('falls back to mock execution on errors', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 404,
        statusText: 'Not Found',
      } as Response)

      const result = await TestAutomationAPI.stopExecution()

      // Should fall back to mock execution instead of throwing
      expect(result).toBe(false)
    })
  })

  describe('getScenariosByTags', () => {
    it('fetches scenarios by tags successfully', async () => {
      const mockResponse = {
        scenarios: [
          {
            id: 'scenario-1',
            name: 'Smoke Test',
            description: 'Basic smoke test',
            feature_file: 'smoke.feature',
            tags: ['@smoke', '@basic'],
            steps: []
          }
        ]
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockResponse,
      } as Response)

      const result = await TestAutomationAPI.getScenariosByTags(['@smoke'])

      expect(fetch).toHaveBeenCalledWith('/api/test/scenarios/tags/@smoke')
      expect(result).toEqual(mockResponse.scenarios)
    })

    it('returns all scenarios when no tags provided', async () => {
      const result = await TestAutomationAPI.getScenariosByTags([])

      // Should call getAvailableScenarios instead
      expect(result).toBeInstanceOf(Array)
    })

    it('falls back to mock data on fetch errors', async () => {
      vi.mocked(fetch).mockRejectedValueOnce(new Error('Network error'))

      const result = await TestAutomationAPI.getScenariosByTags(['@smoke'])

      // Should return filtered mock data
      expect(result).toBeInstanceOf(Array)
    })
  })

  describe('getExecutionStatus', () => {
    it('fetches execution status successfully', async () => {
      const mockStatus = {
        execution: {
          id: 'exec-123',
          status: 'running',
          scenarios: ['scenario-1', 'scenario-2']
        },
        in_progress: true
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockStatus,
      } as Response)

      const result = await TestAutomationAPI.getExecutionStatus()

      expect(fetch).toHaveBeenCalledWith('/api/test/execution')
      expect(result).toEqual(mockStatus)
    })

    it('falls back to mock status on errors', async () => {
      vi.mocked(fetch).mockRejectedValueOnce(new Error('Network error'))

      const result = await TestAutomationAPI.getExecutionStatus()

      expect(result).toHaveProperty('execution')
      expect(result).toHaveProperty('in_progress')
    })
  })

  describe('getAvailableTags', () => {
    it('fetches available tags successfully', async () => {
      const mockResponse = {
        tags: ['@smoke', '@integration', '@hil']
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockResponse,
      } as Response)

      const result = await TestAutomationAPI.getAvailableTags()

      expect(fetch).toHaveBeenCalledWith('/api/test/tags')
      expect(result).toEqual(mockResponse.tags)
    })

    it('falls back to mock tags on errors', async () => {
      vi.mocked(fetch).mockRejectedValueOnce(new Error('Network error'))

      const result = await TestAutomationAPI.getAvailableTags()

      expect(result).toBeInstanceOf(Array)
      expect(result.length).toBeGreaterThan(0)
    })
  })

  describe('Utility Methods', () => {
    it('generates unique execution IDs', () => {
      const id1 = TestAutomationAPI.generateExecutionId()
      const id2 = TestAutomationAPI.generateExecutionId()

      expect(id1).toMatch(/^exec_\d+_[a-z0-9]+$/)
      expect(id2).toMatch(/^exec_\d+_[a-z0-9]+$/)
      expect(id1).not.toBe(id2)
    })

    it('formats duration correctly', () => {
      expect(TestAutomationAPI.formatDuration(0)).toBe('0s')
      expect(TestAutomationAPI.formatDuration(500)).toBe('500ms')
      expect(TestAutomationAPI.formatDuration(1500)).toBe('1s')
      expect(TestAutomationAPI.formatDuration(65000)).toBe('1m 5s')
      expect(TestAutomationAPI.formatDuration(3665000)).toBe('1h 1m 5s')
    })

    it('provides correct status colors', () => {
      expect(TestAutomationAPI.getStatusColor('passed')).toBe('#22c55e')
      expect(TestAutomationAPI.getStatusColor('failed')).toBe('#ef4444')
      expect(TestAutomationAPI.getStatusColor('running')).toBe('#3b82f6')
      expect(TestAutomationAPI.getStatusColor('unknown')).toBe('#6b7280')
    })

    it('provides correct status icons', () => {
      expect(TestAutomationAPI.getStatusIcon('passed')).toBe('✅')
      expect(TestAutomationAPI.getStatusIcon('failed')).toBe('❌')
      expect(TestAutomationAPI.getStatusIcon('running')).toBe('🔄')
      expect(TestAutomationAPI.getStatusIcon('unknown')).toBe('❓')
    })
  })

  describe('Filtering Methods', () => {
    const mockScenarios = [
      {
        id: 'scenario-1',
        name: 'Pin Test',
        description: 'Test pins',
        feature_file: 'pin-control.feature',
        tags: ['@smoke', '@pin'],
        steps: [
          {
            step_type: 'Given',
            description: 'setup',
            pin_interactions: ['D8', 'D9'],
            status: 'pending'
          }
        ]
      },
      {
        id: 'scenario-2',
        name: 'Power Test',
        description: 'Test power',
        feature_file: 'power.feature',
        tags: ['@power'],
        steps: [
          {
            step_type: 'When',
            description: 'test',
            pin_interactions: ['A0'],
            status: 'pending'
          }
        ]
      }
    ]

    it('filters scenarios by feature file', () => {
      const result = TestAutomationAPI.filterScenariosByFeature(mockScenarios, ['pin-control.feature'])
      expect(result).toHaveLength(1)
      expect(result[0].id).toBe('scenario-1')
    })

    it('filters scenarios by tags', () => {
      const result = TestAutomationAPI.filterScenariosByTags(mockScenarios, ['@smoke'])
      expect(result).toHaveLength(1)
      expect(result[0].id).toBe('scenario-1')
    })

    it('gets scenarios by pins', () => {
      const result = TestAutomationAPI.getScenariosByPins(mockScenarios, ['D8'])
      expect(result).toHaveLength(1)
      expect(result[0].id).toBe('scenario-1')
    })

    it('gets scenario pins', () => {
      const pins = TestAutomationAPI.getScenarioPins(mockScenarios[0])
      expect(pins).toEqual(['D8', 'D9'])
    })
  })
})
