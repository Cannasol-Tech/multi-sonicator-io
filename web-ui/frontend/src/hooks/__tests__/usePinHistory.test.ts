import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { renderHook, act } from '@testing-library/react'
import { usePinHistory } from '../usePinHistory'
import { PinState } from '../../types'

describe('usePinHistory', () => {
  beforeEach(() => {
    vi.useFakeTimers()
  })

  afterEach(() => {
    vi.useRealTimers()
  })

  describe('addHistoryEntry', () => {
    it('adds entry to history when state changes', () => {
      const { result } = renderHook(() => usePinHistory())
      const pinState: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }

      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL', pinState)
      })

      expect(result.current.history).toHaveLength(0) // No previous state, so no entry added

      // Update with new state
      const newPinState: PinState = {
        ...pinState,
        state: 'LOW',
        timestamp: Date.now() + 1000
      }

      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL', newPinState)
      })

      expect(result.current.history).toHaveLength(1)
      expect(result.current.history[0]).toEqual({
        signal: 'TEST_SIGNAL',
        pin: 'D7',
        previousState: 'HIGH',
        newState: 'LOW',
        timestamp: newPinState.timestamp,
        direction: 'OUT'
      })
    })

    it('does not add entry when state is the same', () => {
      const { result } = renderHook(() => usePinHistory())
      const pinState: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }

      // Add initial state
      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL', pinState)
      })

      // Try to add the same state again
      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL', pinState)
      })

      expect(result.current.history).toHaveLength(0)
    })

    it('limits history to MAX_HISTORY_ENTRIES', () => {
      const { result } = renderHook(() => usePinHistory())
      const pinState: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }

      // Add more entries than MAX_HISTORY_ENTRIES
      for (let i = 0; i < 1005; i++) {
        const statePin: PinState = {
          ...pinState,
          state: i % 2 === 0 ? 'HIGH' : 'LOW',
          timestamp: Date.now() + i
        }
        
        act(() => {
          result.current.addHistoryEntry('TEST_SIGNAL', statePin)
        })
      }

      expect(result.current.history).toHaveLength(1000)
    })
  })

  describe('getHistoryForPin', () => {
    it('returns history entries for specific pin', () => {
      const { result } = renderHook(() => usePinHistory())
      
      const pinState1: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL_1',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }
      
      const pinState2: PinState = {
        pin: 'D8',
        signal: 'TEST_SIGNAL_2',
        direction: 'OUT',
        state: 'LOW',
        timestamp: Date.now() + 1000
      }

      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL_1', { ...pinState1, state: 'LOW' })
        result.current.addHistoryEntry('TEST_SIGNAL_2', { ...pinState2, state: 'HIGH' })
      })

      const historyForPin1 = result.current.getHistoryForPin('TEST_SIGNAL_1')
      expect(historyForPin1).toHaveLength(1)
      expect(historyForPin1[0].signal).toBe('TEST_SIGNAL_1')
    })
  })

  describe('getRecentHistory', () => {
    it('returns recent history entries', () => {
      const { result } = renderHook(() => usePinHistory())
      
      const pinState: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }

      // Add multiple entries
      for (let i = 0; i < 10; i++) {
        const statePin: PinState = {
          ...pinState,
          state: i % 2 === 0 ? 'HIGH' : 'LOW',
          timestamp: Date.now() + i
        }
        
        act(() => {
          result.current.addHistoryEntry('TEST_SIGNAL', statePin)
        })
      }

      const recentHistory = result.current.getRecentHistory(5)
      expect(recentHistory).toHaveLength(5)
      expect(recentHistory[0].timestamp).toBeGreaterThan(recentHistory[4].timestamp)
    })
  })

  describe('clearHistory', () => {
    it('clears all history and previous states', () => {
      const { result } = renderHook(() => usePinHistory())
      
      const pinState: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }

      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL', { ...pinState, state: 'LOW' })
        result.current.clearHistory()
      })

      expect(result.current.history).toHaveLength(0)
      expect(result.current.getHistoryForPin('TEST_SIGNAL')).toHaveLength(0)
    })
  })

  describe('getHistoryStats', () => {
    it('returns history statistics', () => {
      const { result } = renderHook(() => usePinHistory())
      
      const pinState1: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL_1',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }
      
      const pinState2: PinState = {
        pin: 'D8',
        signal: 'TEST_SIGNAL_2',
        direction: 'OUT',
        state: 'LOW',
        timestamp: Date.now() + 1000
      }

      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL_1', { ...pinState1, state: 'LOW' })
        result.current.addHistoryEntry('TEST_SIGNAL_2', { ...pinState2, state: 'HIGH' })
      })

      const stats = result.current.getHistoryStats()
      expect(stats.totalEntries).toBe(2)
      expect(stats.pinCounts).toEqual({
        'TEST_SIGNAL_1': 1,
        'TEST_SIGNAL_2': 1
      })
      expect(stats.recentActivity).toHaveLength(2)
    })
  })

  describe('exportHistory', () => {
    it('exports history as JSON', () => {
      const { result } = renderHook(() => usePinHistory())
      
      const pinState: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }

      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL', { ...pinState, state: 'LOW' })
      })

      const jsonExport = result.current.exportHistory('json')
      expect(jsonExport).toContain('TEST_SIGNAL')
      expect(jsonExport).toContain('HIGH')
      expect(jsonExport).toContain('LOW')
    })

    it('exports history as CSV', () => {
      const { result } = renderHook(() => usePinHistory())
      
      const pinState: PinState = {
        pin: 'D7',
        signal: 'TEST_SIGNAL',
        direction: 'OUT',
        state: 'HIGH',
        timestamp: Date.now()
      }

      act(() => {
        result.current.addHistoryEntry('TEST_SIGNAL', { ...pinState, state: 'LOW' })
      })

      const csvExport = result.current.exportHistory('csv')
      expect(csvExport).toContain('Timestamp,Signal,Pin,Direction,Previous State,New State')
      expect(csvExport).toContain('TEST_SIGNAL')
      expect(csvExport).toContain('HIGH')
      expect(csvExport).toContain('LOW')
    })
  })
})