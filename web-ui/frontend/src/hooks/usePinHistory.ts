import { useState, useCallback } from 'react'
import { PinState } from '../types'

export interface PinHistoryEntry {
  signal: string
  pin: string
  previousState: 'HIGH' | 'LOW' | number | string
  newState: 'HIGH' | 'LOW' | number | string
  timestamp: number
  direction: 'IN' | 'OUT' | 'ANALOG'
}

const MAX_HISTORY_ENTRIES = 1000 // Configurable retention

export function usePinHistory() {
  const [history, setHistory] = useState<PinHistoryEntry[]>([])
  const [previousStates, setPreviousStates] = useState<Record<string, PinState>>({})

  const addHistoryEntry = useCallback((signal: string, newPinState: PinState) => {
    const previousState = previousStates[signal]
    
    // Only add to history if state actually changed
    if (previousState && previousState.state !== newPinState.state) {
      const entry: PinHistoryEntry = {
        signal,
        pin: newPinState.pin,
        previousState: previousState.state,
        newState: newPinState.state,
        timestamp: newPinState.timestamp,
        direction: newPinState.direction
      }

      setHistory(prev => {
        const newHistory = [entry, ...prev]
        // Limit history size
        return newHistory.slice(0, MAX_HISTORY_ENTRIES)
      })
    }

    // Update previous states
    setPreviousStates(prev => ({
      ...prev,
      [signal]: newPinState
    }))
  }, [previousStates])

  const getHistoryForPin = useCallback((signal: string) => {
    return history.filter(entry => entry.signal === signal)
  }, [history])

  const getRecentHistory = useCallback((count: number = 50) => {
    return history.slice(0, count)
  }, [history])

  const clearHistory = useCallback(() => {
    setHistory([])
    setPreviousStates({})
  }, [])

  const getHistoryStats = useCallback(() => {
    const stats = {
      totalEntries: history.length,
      pinCounts: {} as Record<string, number>,
      recentActivity: history.slice(0, 10)
    }

    history.forEach(entry => {
      stats.pinCounts[entry.signal] = (stats.pinCounts[entry.signal] || 0) + 1
    })

    return stats
  }, [history])

  // Export history data
  const exportHistory = useCallback((format: 'json' | 'csv' = 'json') => {
    if (format === 'csv') {
      const headers = ['Timestamp', 'Signal', 'Pin', 'Direction', 'Previous State', 'New State']
      const csvData = [
        headers.join(','),
        ...history.map(entry => [
          new Date(entry.timestamp).toISOString(),
          entry.signal,
          entry.pin,
          entry.direction,
          entry.previousState,
          entry.newState
        ].join(','))
      ].join('\n')
      
      return csvData
    } else {
      return JSON.stringify(history, null, 2)
    }
  }, [history])

  return {
    history,
    addHistoryEntry,
    getHistoryForPin,
    getRecentHistory,
    clearHistory,
    getHistoryStats,
    exportHistory
  }
}
