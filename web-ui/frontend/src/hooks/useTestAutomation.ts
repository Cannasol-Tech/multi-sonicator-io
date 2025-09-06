import { useState, useEffect, useCallback } from 'react'
import { TestExecution, TestAutomationState } from '../types'
import { TestAutomationAPI } from '../services/testAutomationApi'

export function useTestAutomation() {
  const [state, setState] = useState<TestAutomationState>({
    availableScenarios: [],
    availableTags: [],
    availableFeatures: [],
    currentExecution: null,
    isExecutionInProgress: false,
    selectedScenarios: [],
    filterTags: []
  })

  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  // Load initial data
  const loadInitialData = useCallback(async () => {
    setLoading(true)
    setError(null)

    try {
      const [scenarios, tags, features, executionStatus] = await Promise.all([
        TestAutomationAPI.getAvailableScenarios(),
        TestAutomationAPI.getAvailableTags(),
        TestAutomationAPI.getAvailableFeatures(),
        TestAutomationAPI.getExecutionStatus()
      ])

      setState(prev => ({
        ...prev,
        availableScenarios: scenarios,
        availableTags: tags,
        availableFeatures: features,
        currentExecution: executionStatus.execution,
        isExecutionInProgress: executionStatus.in_progress
      }))

    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load test data')
      console.error('Failed to load test automation data:', err)
    } finally {
      setLoading(false)
    }
  }, [])

  // Load scenarios by tags
  const loadScenariosByTags = useCallback(async (tags: string[]) => {
    setLoading(true)
    setError(null)

    try {
      const scenarios = await TestAutomationAPI.getScenariosByTags(tags)
      setState(prev => ({
        ...prev,
        availableScenarios: scenarios,
        filterTags: tags
      }))
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load scenarios by tags')
      console.error('Failed to load scenarios by tags:', err)
    } finally {
      setLoading(false)
    }
  }, [])

  // Execute selected scenarios
  const executeScenarios = useCallback(async (scenarioNames: string[]) => {
    if (state.isExecutionInProgress) {
      setError('Test execution already in progress')
      return false
    }

    setError(null)

    try {
      const executionId = TestAutomationAPI.generateExecutionId()
      const success = await TestAutomationAPI.executeScenarios(scenarioNames, executionId)

      if (success) {
        setState(prev => ({
          ...prev,
          isExecutionInProgress: true,
          selectedScenarios: scenarioNames
        }))
      }

      return success

    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to start test execution')
      console.error('Failed to execute scenarios:', err)
      return false
    }
  }, [state.isExecutionInProgress])

  // Stop current execution
  const stopExecution = useCallback(async () => {
    setError(null)

    try {
      const success = await TestAutomationAPI.stopExecution()
      
      if (success) {
        setState(prev => ({
          ...prev,
          isExecutionInProgress: false
        }))
      }

      return success

    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to stop test execution')
      console.error('Failed to stop execution:', err)
      return false
    }
  }, [])

  // Update execution progress (called from WebSocket updates)
  const updateExecutionProgress = useCallback((execution: TestExecution) => {
    setState(prev => ({
      ...prev,
      currentExecution: execution,
      isExecutionInProgress: execution.status === 'running'
    }))
  }, [])

  // Handle execution completion
  const handleExecutionComplete = useCallback((execution: TestExecution) => {
    setState(prev => ({
      ...prev,
      currentExecution: execution,
      isExecutionInProgress: false
    }))
  }, [])

  // Handle execution error
  const handleExecutionError = useCallback((errorMessage: string) => {
    setError(errorMessage)
    setState(prev => ({
      ...prev,
      isExecutionInProgress: false
    }))
  }, [])

  // Select/deselect scenarios
  const toggleScenarioSelection = useCallback((scenarioName: string) => {
    setState(prev => ({
      ...prev,
      selectedScenarios: prev.selectedScenarios.includes(scenarioName)
        ? prev.selectedScenarios.filter(name => name !== scenarioName)
        : [...prev.selectedScenarios, scenarioName]
    }))
  }, [])

  // Select all scenarios
  const selectAllScenarios = useCallback(() => {
    setState(prev => ({
      ...prev,
      selectedScenarios: prev.availableScenarios.map(s => s.name)
    }))
  }, [])

  // Clear scenario selection
  const clearScenarioSelection = useCallback(() => {
    setState(prev => ({
      ...prev,
      selectedScenarios: []
    }))
  }, [])

  // Filter scenarios by tags
  const setFilterTags = useCallback((tags: string[]) => {
    setState(prev => ({
      ...prev,
      filterTags: tags
    }))
    
    // Reload scenarios with new filter
    loadScenariosByTags(tags)
  }, [loadScenariosByTags])

  // Get filtered scenarios
  const getFilteredScenarios = useCallback(() => {
    let filtered = state.availableScenarios

    if (state.filterTags.length > 0) {
      filtered = TestAutomationAPI.filterScenariosByTags(filtered, state.filterTags)
    }

    return filtered
  }, [state.availableScenarios, state.filterTags])

  // Get scenarios by pin interactions
  const getScenariosByPins = useCallback((pins: string[]) => {
    return TestAutomationAPI.getScenariosByPins(state.availableScenarios, pins)
  }, [state.availableScenarios])

  // Get execution progress
  const getExecutionProgress = useCallback(() => {
    if (!state.currentExecution) {
      return { percentage: 0, current: 0, total: 0 }
    }

    return {
      percentage: TestAutomationAPI.getExecutionProgress(state.currentExecution),
      current: state.currentExecution.passed_scenarios + state.currentExecution.failed_scenarios,
      total: state.currentExecution.total_scenarios
    }
  }, [state.currentExecution])

  // Get current step progress
  const getCurrentStepProgress = useCallback(() => {
    if (!state.currentExecution) {
      return { current: 0, total: 0 }
    }

    return TestAutomationAPI.getCurrentStepProgress(state.currentExecution)
  }, [state.currentExecution])

  // Clear error
  const clearError = useCallback(() => {
    setError(null)
  }, [])

  // Initialize on mount
  useEffect(() => {
    loadInitialData()
  }, [loadInitialData])

  return {
    // State
    ...state,
    loading,
    error,

    // Actions
    loadInitialData,
    loadScenariosByTags,
    executeScenarios,
    stopExecution,
    updateExecutionProgress,
    handleExecutionComplete,
    handleExecutionError,
    toggleScenarioSelection,
    selectAllScenarios,
    clearScenarioSelection,
    setFilterTags,
    clearError,

    // Computed values
    getFilteredScenarios,
    getScenariosByPins,
    getExecutionProgress,
    getCurrentStepProgress,

    // Helper functions
    formatDuration: TestAutomationAPI.formatDuration,
    getStatusColor: TestAutomationAPI.getStatusColor,
    getStatusIcon: TestAutomationAPI.getStatusIcon
  }
}
