import { describe, it, expect, vi, beforeEach } from 'vitest'
import testAutomationAPI from '../testAutomationAPI'

// Mock fetch
global.fetch = vi.fn()

describe('Test Automation API', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    // Reset fetch mock
    vi.mocked(fetch).mockClear()
  })

  describe('getScenarios', () => {
    it('fetches scenarios successfully', async () => {
      const mockScenarios = [
        {
          id: 'scenario-1',
          name: 'Basic Pin Test',
          description: 'Test basic pin operations',
          feature: 'pin-control.feature',
          tags: ['@smoke', '@pin'],
          steps: ['Given the Arduino is connected', 'When I set pin D8 to HIGH']
        }
      ]

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockScenarios,
      } as Response)

      const result = await testAutomationAPI.getScenarios()

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/scenarios')
      expect(result).toEqual(mockScenarios)
    })

    it('handles fetch errors', async () => {
      vi.mocked(fetch).mockRejectedValueOnce(new Error('Network error'))

      await expect(testAutomationAPI.getScenarios()).rejects.toThrow('Network error')
    })

    it('handles HTTP errors', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 500,
        statusText: 'Internal Server Error',
      } as Response)

      await expect(testAutomationAPI.getScenarios()).rejects.toThrow('HTTP error! status: 500')
    })

    it('handles invalid JSON response', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => {
          throw new Error('Invalid JSON')
        },
      } as Response)

      await expect(testAutomationAPI.getScenarios()).rejects.toThrow('Invalid JSON')
    })
  })

  describe('executeScenarios', () => {
    it('starts scenario execution successfully', async () => {
      const mockResponse = {
        executionId: 'exec-123',
        status: 'started',
        scenarioIds: ['scenario-1', 'scenario-2']
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockResponse,
      } as Response)

      const scenarioIds = ['scenario-1', 'scenario-2']
      const result = await testAutomationAPI.executeScenarios(scenarioIds)

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/execute', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ scenarioIds })
      })
      expect(result).toEqual(mockResponse)
    })

    it('validates scenario IDs parameter', async () => {
      await expect(testAutomationAPI.executeScenarios([])).rejects.toThrow('No scenarios provided')
    })

    it('handles execution start errors', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 400,
        statusText: 'Bad Request',
      } as Response)

      await expect(testAutomationAPI.executeScenarios(['scenario-1'])).rejects.toThrow('HTTP error! status: 400')
    })

    it('includes optional parameters in request', async () => {
      const mockResponse = { executionId: 'exec-123', status: 'started' }
      
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockResponse,
      } as Response)

      const options = {
        parallel: true,
        timeout: 30000,
        tags: ['@smoke']
      }

      await testAutomationAPI.executeScenarios(['scenario-1'], options)

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/execute', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          scenarioIds: ['scenario-1'],
          ...options
        })
      })
    })
  })

  describe('stopExecution', () => {
    it('stops execution successfully', async () => {
      const mockResponse = {
        executionId: 'exec-123',
        status: 'stopped'
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockResponse,
      } as Response)

      const result = await testAutomationAPI.stopExecution('exec-123')

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/stop/exec-123', {
        method: 'POST'
      })
      expect(result).toEqual(mockResponse)
    })

    it('validates execution ID parameter', async () => {
      await expect(testAutomationAPI.stopExecution('')).rejects.toThrow('Execution ID is required')
    })

    it('handles stop execution errors', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 404,
        statusText: 'Not Found',
      } as Response)

      await expect(testAutomationAPI.stopExecution('invalid-id')).rejects.toThrow('HTTP error! status: 404')
    })
  })

  describe('getExecutionResults', () => {
    it('fetches execution results successfully', async () => {
      const mockResults = {
        executionId: 'exec-123',
        status: 'completed',
        startTime: Date.now(),
        endTime: Date.now() + 5000,
        results: [
          {
            scenarioId: 'scenario-1',
            status: 'passed',
            duration: 1500,
            steps: [
              { description: 'Given the Arduino is connected', status: 'passed', duration: 500 },
              { description: 'When I set pin D8 to HIGH', status: 'passed', duration: 500 },
              { description: 'Then pin D8 should be HIGH', status: 'passed', duration: 500 }
            ]
          }
        ]
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockResults,
      } as Response)

      const result = await testAutomationAPI.getExecutionResults('exec-123')

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/results/exec-123')
      expect(result).toEqual(mockResults)
    })

    it('validates execution ID parameter', async () => {
      await expect(testAutomationAPI.getExecutionResults('')).rejects.toThrow('Execution ID is required')
    })

    it('handles results fetch errors', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 404,
        statusText: 'Not Found',
      } as Response)

      await expect(testAutomationAPI.getExecutionResults('invalid-id')).rejects.toThrow('HTTP error! status: 404')
    })

    it('polls for results when execution is in progress', async () => {
      const inProgressResponse = {
        executionId: 'exec-123',
        status: 'running',
        results: []
      }

      const completedResponse = {
        executionId: 'exec-123',
        status: 'completed',
        results: [
          { scenarioId: 'scenario-1', status: 'passed', duration: 1500 }
        ]
      }

      vi.mocked(fetch)
        .mockResolvedValueOnce({
          ok: true,
          json: async () => inProgressResponse,
        } as Response)
        .mockResolvedValueOnce({
          ok: true,
          json: async () => completedResponse,
        } as Response)

      const result = await testAutomationAPI.getExecutionResults('exec-123', { poll: true, pollInterval: 100 })

      expect(fetch).toHaveBeenCalledTimes(2)
      expect(result).toEqual(completedResponse)
    })
  })

  describe('getExecutionStatus', () => {
    it('fetches execution status successfully', async () => {
      const mockStatus = {
        executionId: 'exec-123',
        status: 'running',
        progress: {
          total: 5,
          completed: 2,
          failed: 0,
          currentScenario: 'scenario-3'
        }
      }

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => mockStatus,
      } as Response)

      const result = await testAutomationAPI.getExecutionStatus('exec-123')

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/status/exec-123')
      expect(result).toEqual(mockStatus)
    })

    it('validates execution ID parameter', async () => {
      await expect(testAutomationAPI.getExecutionStatus('')).rejects.toThrow('Execution ID is required')
    })
  })

  describe('exportResults', () => {
    it('exports results in specified format', async () => {
      const mockBlob = new Blob(['test results'], { type: 'application/json' })

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        blob: async () => mockBlob,
      } as Response)

      const result = await testAutomationAPI.exportResults('exec-123', 'json')

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/export/exec-123?format=json')
      expect(result).toEqual(mockBlob)
    })

    it('defaults to JSON format', async () => {
      const mockBlob = new Blob(['test results'], { type: 'application/json' })

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        blob: async () => mockBlob,
      } as Response)

      await testAutomationAPI.exportResults('exec-123')

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/export/exec-123?format=json')
    })

    it('supports different export formats', async () => {
      const mockBlob = new Blob(['test results'], { type: 'text/html' })

      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        blob: async () => mockBlob,
      } as Response)

      await testAutomationAPI.exportResults('exec-123', 'html')

      expect(fetch).toHaveBeenCalledWith('/api/test-automation/export/exec-123?format=html')
    })
  })

  describe('Error Handling', () => {
    it('handles network timeouts', async () => {
      vi.mocked(fetch).mockImplementation(() => 
        new Promise((_, reject) => 
          setTimeout(() => reject(new Error('Request timeout')), 100)
        )
      )

      await expect(testAutomationAPI.getScenarios()).rejects.toThrow('Request timeout')
    })

    it('handles malformed API responses', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => null,
      } as Response)

      const result = await testAutomationAPI.getScenarios()
      expect(result).toBeNull()
    })

    it('provides meaningful error messages', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: false,
        status: 422,
        statusText: 'Unprocessable Entity',
        json: async () => ({ error: 'Invalid scenario configuration' }),
      } as Response)

      try {
        await testAutomationAPI.executeScenarios(['invalid-scenario'])
      } catch (error) {
        expect(error).toBeInstanceOf(Error)
        expect((error as Error).message).toContain('422')
      }
    })
  })

  describe('Request Configuration', () => {
    it('includes proper headers for JSON requests', async () => {
      vi.mocked(fetch).mockResolvedValueOnce({
        ok: true,
        json: async () => ({ executionId: 'exec-123' }),
      } as Response)

      await testAutomationAPI.executeScenarios(['scenario-1'])

      expect(fetch).toHaveBeenCalledWith(
        expect.any(String),
        expect.objectContaining({
          headers: {
            'Content-Type': 'application/json',
          }
        })
      )
    })

    it('uses correct HTTP methods', async () => {
      vi.mocked(fetch).mockResolvedValue({
        ok: true,
        json: async () => ({}),
      } as Response)

      // GET request
      await testAutomationAPI.getScenarios()
      expect(fetch).toHaveBeenLastCalledWith(
        expect.any(String),
        expect.not.objectContaining({ method: expect.any(String) })
      )

      // POST request
      await testAutomationAPI.executeScenarios(['scenario-1'])
      expect(fetch).toHaveBeenLastCalledWith(
        expect.any(String),
        expect.objectContaining({ method: 'POST' })
      )
    })
  })
})
