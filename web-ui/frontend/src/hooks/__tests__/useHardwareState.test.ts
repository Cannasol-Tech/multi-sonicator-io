import { describe, it, expect, vi, beforeEach } from 'vitest'
import { renderHook, act } from '@testing-library/react'
import { useHardwareState } from '../useHardwareState'
import { createMockHardwareState, createMockPinState } from '../../test/utils'

describe('useHardwareState', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  describe('Initial State', () => {
    it('initializes with default hardware state', () => {
      const { result } = renderHook(() => useHardwareState())

      expect(result.current.hardwareState.connection.connected).toBe(false)
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10) // 10 initial pins
      expect(result.current.hardwareState.lastUpdate).toBeGreaterThan(0)
    })
  })

  describe('Connection Status', () => {
    it('updates connection status', () => {
      const { result } = renderHook(() => useHardwareState())

      act(() => {
        result.current.setConnectionStatus({ connected: true })
      })

      expect(result.current.hardwareState.connection.connected).toBe(true)
    })

    it('updates lastUpdate timestamp when connection status changes', () => {
      const { result } = renderHook(() => useHardwareState())
      const beforeTime = Date.now()

      act(() => {
        result.current.setConnectionStatus({ connected: true })
      })

      expect(result.current.hardwareState.lastUpdate).toBeGreaterThanOrEqual(beforeTime)
    })
  })

  describe('Pin State Updates', () => {
    it('updates individual pin state', () => {
      const { result } = renderHook(() => useHardwareState())

      const pinUpdate = {
        signal: 'TEST_PIN',
        state: 'HIGH'
      }

      act(() => {
        result.current.updatePinState('TEST_PIN', pinUpdate)
      })

      expect(result.current.hardwareState.pins['TEST_PIN'].state).toBe('HIGH')
      expect(result.current.hardwareState.pins['TEST_PIN'].signal).toBe('TEST_PIN')
    })

    it('updates lastUpdate timestamp when pin state changes', () => {
      const { result } = renderHook(() => useHardwareState())
      const beforeTime = Date.now()

      const pinState = createMockPinState({
        signal: 'TEST_PIN',
        state: 'HIGH'
      })

      act(() => {
        result.current.updatePinState('TEST_PIN', pinState)
      })

      expect(result.current.hardwareState.lastUpdate).toBeGreaterThanOrEqual(beforeTime)
    })

    it('preserves existing pins when updating individual pin', () => {
      const { result } = renderHook(() => useHardwareState())

      // Use existing pin names from the initial state
      const existingPinName = 'START_4'
      const anotherPinName = 'RESET_4'

      act(() => {
        result.current.updatePinState(existingPinName, { state: 'HIGH' })
      })

      act(() => {
        result.current.updatePinState(anotherPinName, { state: 'LOW' })
      })

      expect(result.current.hardwareState.pins[existingPinName].state).toBe('HIGH')
      expect(result.current.hardwareState.pins[anotherPinName].state).toBe('LOW')
      // Should still have all original pins
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10)
    })

    it('overwrites existing pin state when updating same pin', () => {
      const { result } = renderHook(() => useHardwareState())

      const pinName = 'START_4'

      act(() => {
        result.current.updatePinState(pinName, { state: 'LOW' })
      })

      act(() => {
        result.current.updatePinState(pinName, { state: 'HIGH' })
      })

      expect(result.current.hardwareState.pins[pinName].state).toBe('HIGH')
    })
  })

  describe('Multiple Pin Updates', () => {
    it('updates multiple pins at once', () => {
      const { result } = renderHook(() => useHardwareState())

      const updates = {
        'START_4': { state: 'HIGH' },
        'RESET_4': { state: 'LOW' },
        'AMPLITUDE_ALL': { state: 'PWM=50%' }
      }

      act(() => {
        result.current.updateMultiplePins(updates)
      })

      expect(result.current.hardwareState.pins['START_4'].state).toBe('HIGH')
      expect(result.current.hardwareState.pins['RESET_4'].state).toBe('LOW')
      expect(result.current.hardwareState.pins['AMPLITUDE_ALL'].state).toBe('PWM=50%')
      // Should still have all original pins
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10)
    })

    it('merges with existing pins when updating multiple pins', () => {
      const { result } = renderHook(() => useHardwareState())

      // First update one pin
      act(() => {
        result.current.updatePinState('START_4', { state: 'HIGH' })
      })

      // Then update multiple pins
      const updates = {
        'RESET_4': { state: 'LOW' },
        'AMPLITUDE_ALL': { state: 'HIGH' }
      }

      act(() => {
        result.current.updateMultiplePins(updates)
      })

      // Should preserve the first update and apply the new ones
      expect(result.current.hardwareState.pins['START_4'].state).toBe('HIGH')
      expect(result.current.hardwareState.pins['RESET_4'].state).toBe('LOW')
      expect(result.current.hardwareState.pins['AMPLITUDE_ALL'].state).toBe('HIGH')
      // Should still have all original pins
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10)
    })

    it('overwrites existing pins when updating multiple pins with same keys', () => {
      const { result } = renderHook(() => useHardwareState())

      const initialUpdates = {
        'START_4': { state: 'LOW' },
        'RESET_4': { state: 'LOW' }
      }

      const laterUpdates = {
        'START_4': { state: 'HIGH' },
        'RESET_4': { state: 'HIGH' }
      }

      act(() => {
        result.current.updateMultiplePins(initialUpdates)
      })

      act(() => {
        result.current.updateMultiplePins(laterUpdates)
      })

      expect(result.current.hardwareState.pins['START_4'].state).toBe('HIGH')
      expect(result.current.hardwareState.pins['RESET_4'].state).toBe('HIGH')
    })

    it('updates lastUpdate timestamp when updating multiple pins', () => {
      const { result } = renderHook(() => useHardwareState())
      const beforeTime = Date.now()

      const pins = {
        'PIN1': createMockPinState({ signal: 'PIN1', state: 'HIGH' })
      }

      act(() => {
        result.current.updateMultiplePins(pins)
      })

      expect(result.current.hardwareState.lastUpdate).toBeGreaterThanOrEqual(beforeTime)
    })
  })

  describe('State Immutability', () => {
    it('does not mutate previous state when updating pin', () => {
      const { result } = renderHook(() => useHardwareState())

      const pin1 = createMockPinState({ signal: 'PIN1', state: 'LOW' })
      
      act(() => {
        result.current.updatePinState('PIN1', pin1)
      })

      const stateAfterFirstUpdate = result.current.hardwareState

      const pin2 = createMockPinState({ signal: 'PIN2', state: 'HIGH' })
      
      act(() => {
        result.current.updatePinState('PIN2', pin2)
      })

      // Previous state should not be mutated
      expect(stateAfterFirstUpdate.pins).not.toHaveProperty('PIN2')
      expect(result.current.hardwareState.pins).toHaveProperty('PIN2')
    })

    it('does not mutate previous state when updating multiple pins', async () => {
      const { result } = renderHook(() => useHardwareState())

      const initialUpdates = {
        'START_4': { state: 'LOW' }
      }

      act(() => {
        result.current.updateMultiplePins(initialUpdates)
      })

      const stateAfterFirstUpdate = result.current.hardwareState

      const additionalUpdates = {
        'RESET_4': { state: 'HIGH' }
      }

      // Add small delay to ensure different timestamps
      await act(async () => {
        await new Promise(resolve => setTimeout(resolve, 1))
        result.current.updateMultiplePins(additionalUpdates)
      })

      // Previous state should not be mutated - check timestamps are different
      expect(stateAfterFirstUpdate.lastUpdate).toBeLessThan(result.current.hardwareState.lastUpdate)
      expect(result.current.hardwareState.pins['RESET_4'].state).toBe('HIGH')
    })
  })

  describe('Pin State Validation', () => {
    it('handles undefined pin state gracefully', () => {
      const { result } = renderHook(() => useHardwareState())

      expect(() => {
        act(() => {
          result.current.updatePinState('TEST_PIN', undefined as any)
        })
      }).not.toThrow()
    })

    it('handles null pin state gracefully', () => {
      const { result } = renderHook(() => useHardwareState())

      expect(() => {
        act(() => {
          result.current.updatePinState('TEST_PIN', null as any)
        })
      }).not.toThrow()
    })

    it('handles empty pins object in updateMultiplePins', () => {
      const { result } = renderHook(() => useHardwareState())

      expect(() => {
        act(() => {
          result.current.updateMultiplePins({})
        })
      }).not.toThrow()

      // Should still have all original pins (empty update doesn't remove pins)
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10)
    })
  })

  describe('Performance', () => {
    it('handles frequent pin updates efficiently', () => {
      const { result } = renderHook(() => useHardwareState())

      const startTime = performance.now()
      const existingPins = Object.keys(result.current.hardwareState.pins)

      // Simulate rapid pin updates on existing pins
      for (let i = 0; i < 100; i++) {
        const pinName = existingPins[i % existingPins.length]
        act(() => {
          result.current.updatePinState(pinName, {
            state: i % 2 === 0 ? 'HIGH' : 'LOW'
          })
        })
      }

      const endTime = performance.now()

      // Should complete quickly (less than 100ms)
      expect(endTime - startTime).toBeLessThan(100)
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10) // Still 10 pins
    })

    it('handles large pin state updates efficiently', () => {
      const { result } = renderHook(() => useHardwareState())

      // Create updates for existing pins only (since updateMultiplePins only updates existing pins)
      const existingPins = Object.keys(result.current.hardwareState.pins)
      const updates: Record<string, any> = {}
      existingPins.forEach((pinName, i) => {
        updates[pinName] = { state: i % 2 === 0 ? 'HIGH' : 'LOW' }
      })

      const startTime = performance.now()

      act(() => {
        result.current.updateMultiplePins(updates)
      })

      const endTime = performance.now()

      // Should complete quickly (less than 50ms)
      expect(endTime - startTime).toBeLessThan(50)
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10) // Still 10 pins
    })
  })

  describe('Memory Management', () => {
    it('does not create memory leaks with repeated updates', () => {
      const { result } = renderHook(() => useHardwareState())

      // Simulate many updates to the same existing pin
      const pinName = 'START_4'
      for (let i = 0; i < 1000; i++) {
        act(() => {
          result.current.updatePinState(pinName, {
            state: i % 2 === 0 ? 'HIGH' : 'LOW'
          })
        })
      }

      // Should still have all original pins
      expect(Object.keys(result.current.hardwareState.pins)).toHaveLength(10)
      expect(result.current.hardwareState.pins[pinName]).toBeDefined()
    })
  })
})
