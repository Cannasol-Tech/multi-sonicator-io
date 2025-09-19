import { describe, it, expect } from 'vitest'
import { renderHook, act } from '@testing-library/react'
import { usePinHistory } from '../usePinHistory'
import { PinState } from '../../types'

const baseTimestamp = 1_700_000_000_000

const buildPinState = (overrides: Partial<PinState> = {}): PinState => ({
  pin: 'D7',
  signal: 'TEST_SIGNAL',
  direction: 'OUT',
  state: 'HIGH',
  timestamp: baseTimestamp,
  ...overrides
})

describe('usePinHistory', () => {
  describe('addHistoryEntry', () => {
    it('adds entry to history when state changes', () => {
      const { result } = renderHook(() => usePinHistory())
      const initialState = buildPinState()
      const newState = buildPinState({ state: 'LOW', timestamp: baseTimestamp + 1000 })

      act(() => {
        result.current.addHistoryEntry(initialState.signal, initialState)
      })
      act(() => {
        result.current.addHistoryEntry(initialState.signal, newState)
      })

      expect(result.current.history).toHaveLength(1)
      expect(result.current.history[0]).toEqual({
        signal: initialState.signal,
        pin: initialState.pin,
        previousState: initialState.state,
        newState: newState.state,
        timestamp: newState.timestamp,
        direction: initialState.direction
      })
    })

    it('does not add entry when state is the same', () => {
      const { result } = renderHook(() => usePinHistory())
      const state = buildPinState()

      act(() => {
        result.current.addHistoryEntry(state.signal, state)
      })
      act(() => {
        result.current.addHistoryEntry(state.signal, { ...state, timestamp: state.timestamp + 1 })
      })

      expect(result.current.history).toHaveLength(0)
    })

    it('limits history to MAX_HISTORY_ENTRIES', () => {
      const { result } = renderHook(() => usePinHistory())
      const signal = 'TEST_SIGNAL'

      act(() => {
        result.current.addHistoryEntry(signal, buildPinState({ signal }))
      })

      for (let i = 0; i < 1005; i++) {
        act(() => {
          result.current.addHistoryEntry(signal, buildPinState({
            signal,
            state: i % 2 === 0 ? 'LOW' : 'HIGH',
            timestamp: baseTimestamp + i + 1
          }))
        })
      }

      expect(result.current.history).toHaveLength(1000)
    })
  })

  describe('getHistoryForPin', () => {
    it('returns history entries for specific pin', () => {
      const { result } = renderHook(() => usePinHistory())
      const state1 = buildPinState({ signal: 'TEST_SIGNAL_1', pin: 'D7' })
      const state2 = buildPinState({ signal: 'TEST_SIGNAL_2', pin: 'D8', state: 'LOW' })

      act(() => {
        result.current.addHistoryEntry(state1.signal, state1)
      })
      act(() => {
        result.current.addHistoryEntry(state1.signal, { ...state1, state: 'LOW', timestamp: state1.timestamp + 1 })
      })
      act(() => {
        result.current.addHistoryEntry(state2.signal, state2)
      })
      act(() => {
        result.current.addHistoryEntry(state2.signal, { ...state2, state: 'HIGH', timestamp: state2.timestamp + 1 })
      })

      const historyForPin1 = result.current.getHistoryForPin('TEST_SIGNAL_1')
      expect(historyForPin1).toHaveLength(1)
      expect(historyForPin1[0].signal).toBe('TEST_SIGNAL_1')
    })
  })

  describe('getRecentHistory', () => {
    it('returns recent history entries', () => {
      const { result } = renderHook(() => usePinHistory())
      const signal = 'TEST_SIGNAL'

      act(() => {
        result.current.addHistoryEntry(signal, buildPinState({ signal }))
      })

      for (let i = 0; i < 10; i++) {
        act(() => {
          result.current.addHistoryEntry(signal, buildPinState({
            signal,
            state: i % 2 === 0 ? 'LOW' : 'HIGH',
            timestamp: baseTimestamp + i + 1
          }))
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
      const signal = 'TEST_SIGNAL'

      act(() => {
        result.current.addHistoryEntry(signal, buildPinState({ signal }))
      })
      act(() => {
        result.current.addHistoryEntry(signal, buildPinState({ signal, state: 'LOW', timestamp: baseTimestamp + 1 }))
      })

      act(() => {
        result.current.clearHistory()
      })

      expect(result.current.history).toHaveLength(0)
      expect(result.current.getHistoryForPin(signal)).toHaveLength(0)
    })
  })

  describe('getHistoryStats', () => {
    it('returns history statistics', () => {
      const { result } = renderHook(() => usePinHistory())
      const signal1 = 'TEST_SIGNAL_1'
      const signal2 = 'TEST_SIGNAL_2'

      const baseline1 = buildPinState({ signal: signal1, pin: 'D7' })
      const baseline2 = buildPinState({ signal: signal2, pin: 'D8', state: 'LOW' })

      act(() => {
        result.current.addHistoryEntry(signal1, baseline1)
      })
      act(() => {
        result.current.addHistoryEntry(signal1, { ...baseline1, state: 'LOW', timestamp: baseline1.timestamp + 1 })
      })
      act(() => {
        result.current.addHistoryEntry(signal2, baseline2)
      })
      act(() => {
        result.current.addHistoryEntry(signal2, { ...baseline2, state: 'HIGH', timestamp: baseline2.timestamp + 1 })
      })

      const stats = result.current.getHistoryStats()
      expect(stats.totalEntries).toBe(2)
      expect(stats.pinCounts).toEqual({
        [signal1]: 1,
        [signal2]: 1
      })
      expect(stats.recentActivity).toHaveLength(2)
    })
  })

  describe('exportHistory', () => {
    it('exports history as JSON', () => {
      const { result } = renderHook(() => usePinHistory())
      const baseline = buildPinState()

      act(() => {
        result.current.addHistoryEntry(baseline.signal, baseline)
      })
      act(() => {
        result.current.addHistoryEntry(baseline.signal, { ...baseline, state: 'LOW', timestamp: baseline.timestamp + 1 })
      })

      const jsonExport = result.current.exportHistory('json')
      expect(jsonExport).toContain('TEST_SIGNAL')
      expect(jsonExport).toContain('HIGH')
      expect(jsonExport).toContain('LOW')
    })

    it('exports history as CSV', () => {
      const { result } = renderHook(() => usePinHistory())
      const baseline = buildPinState()

      act(() => {
        result.current.addHistoryEntry(baseline.signal, baseline)
      })
      act(() => {
        result.current.addHistoryEntry(baseline.signal, { ...baseline, state: 'LOW', timestamp: baseline.timestamp + 1 })
      })

      const csvExport = result.current.exportHistory('csv')
      expect(csvExport).toContain('Timestamp,Signal,Pin,Direction,Previous State,New State')
      expect(csvExport).toContain('TEST_SIGNAL')
      expect(csvExport).toContain('HIGH')
      expect(csvExport).toContain('LOW')
    })
  })
})
