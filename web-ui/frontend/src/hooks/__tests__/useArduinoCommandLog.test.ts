import { describe, it, expect, vi, beforeEach } from 'vitest'
import { renderHook, act } from '@testing-library/react'
import { useArduinoCommandLog } from '../useArduinoCommandLog'
import { createMockArduinoCommand } from '../../test/utils'

describe('useArduinoCommandLog', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  describe('Initial State', () => {
    it('initializes with empty commands array', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      expect(result.current.commands).toEqual([])
    })

    it('initializes with default max entries', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      expect(result.current.getCommandCount()).toBe(0)
    })

    it('accepts custom max entries option', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 50 }))

      // Add more than 50 commands
      act(() => {
        for (let i = 0; i < 60; i++) {
          result.current.addCommand(createMockArduinoCommand({
            data: `command_${i}`,
            timestamp: Date.now() + i
          }))
        }
      })

      expect(result.current.getCommandCount()).toBe(50)
    })
  })

  describe('Adding Commands', () => {
    it('adds a single command', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      const command = createMockArduinoCommand({
        direction: 'sent',
        data: 'test command',
        type: 'test'
      })

      act(() => {
        result.current.addCommand(command)
      })

      expect(result.current.commands).toHaveLength(1)
      expect(result.current.commands[0]).toEqual(command)
    })

    it('adds multiple commands in order', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      const command1 = createMockArduinoCommand({
        data: 'command 1',
        timestamp: Date.now()
      })
      const command2 = createMockArduinoCommand({
        data: 'command 2',
        timestamp: Date.now() + 1000
      })

      act(() => {
        result.current.addCommand(command1)
        result.current.addCommand(command2)
      })

      expect(result.current.commands).toHaveLength(2)
      expect(result.current.commands[0]).toEqual(command1)
      expect(result.current.commands[1]).toEqual(command2)
    })

    it('maintains chronological order when adding commands', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      const commands = [
        createMockArduinoCommand({ data: 'cmd1', timestamp: 1000 }),
        createMockArduinoCommand({ data: 'cmd2', timestamp: 2000 }),
        createMockArduinoCommand({ data: 'cmd3', timestamp: 3000 })
      ]

      act(() => {
        commands.forEach(cmd => result.current.addCommand(cmd))
      })

      expect(result.current.commands[0].timestamp).toBeLessThan(result.current.commands[1].timestamp)
      expect(result.current.commands[1].timestamp).toBeLessThan(result.current.commands[2].timestamp)
    })
  })

  describe('Max Entries Enforcement', () => {
    it('enforces max entries limit', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 3 }))

      // Add more commands than the limit
      const commands = [
        createMockArduinoCommand({ data: 'cmd1', timestamp: 1000 }),
        createMockArduinoCommand({ data: 'cmd2', timestamp: 2000 }),
        createMockArduinoCommand({ data: 'cmd3', timestamp: 3000 }),
        createMockArduinoCommand({ data: 'cmd4', timestamp: 4000 }),
        createMockArduinoCommand({ data: 'cmd5', timestamp: 5000 })
      ]

      act(() => {
        commands.forEach(cmd => result.current.addCommand(cmd))
      })

      expect(result.current.commands).toHaveLength(3)
      // Should keep the last 3 commands
      expect(result.current.commands[0].data).toBe('cmd3')
      expect(result.current.commands[1].data).toBe('cmd4')
      expect(result.current.commands[2].data).toBe('cmd5')
    })

    it('removes oldest commands when limit is exceeded', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 2 }))

      const oldCommand = createMockArduinoCommand({ data: 'old', timestamp: 1000 })
      const newCommand1 = createMockArduinoCommand({ data: 'new1', timestamp: 2000 })
      const newCommand2 = createMockArduinoCommand({ data: 'new2', timestamp: 3000 })

      act(() => {
        result.current.addCommand(oldCommand)
        result.current.addCommand(newCommand1)
        result.current.addCommand(newCommand2)
      })

      expect(result.current.commands).toHaveLength(2)
      expect(result.current.commands.find(cmd => cmd.data === 'old')).toBeUndefined()
      expect(result.current.commands.find(cmd => cmd.data === 'new1')).toBeDefined()
      expect(result.current.commands.find(cmd => cmd.data === 'new2')).toBeDefined()
    })
  })

  describe('Clearing Commands', () => {
    it('clears all commands', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      // Add some commands
      act(() => {
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd1' }))
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd2' }))
      })

      expect(result.current.commands).toHaveLength(2)

      act(() => {
        result.current.clearCommands()
      })

      expect(result.current.commands).toHaveLength(0)
    })

    it('resets command count after clearing', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      act(() => {
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd1' }))
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd2' }))
      })

      expect(result.current.getCommandCount()).toBe(2)

      act(() => {
        result.current.clearCommands()
      })

      expect(result.current.getCommandCount()).toBe(0)
    })
  })

  describe('Command Count', () => {
    it('returns correct command count', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      expect(result.current.getCommandCount()).toBe(0)

      act(() => {
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd1' }))
      })

      expect(result.current.getCommandCount()).toBe(1)

      act(() => {
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd2' }))
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd3' }))
      })

      expect(result.current.getCommandCount()).toBe(3)
    })

    it('returns count after max entries enforcement', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 2 }))

      act(() => {
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd1' }))
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd2' }))
        result.current.addCommand(createMockArduinoCommand({ data: 'cmd3' }))
      })

      expect(result.current.getCommandCount()).toBe(2)
    })
  })

  describe('Filtered Commands', () => {
    it('returns all commands when filter is "all"', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      const sentCommand = createMockArduinoCommand({ direction: 'sent', data: 'sent' })
      const receivedCommand = createMockArduinoCommand({ direction: 'received', data: 'received' })

      act(() => {
        result.current.addCommand(sentCommand)
        result.current.addCommand(receivedCommand)
      })

      const filtered = result.current.getFilteredCommands('all')
      expect(filtered).toHaveLength(2)
      expect(filtered).toContain(sentCommand)
      expect(filtered).toContain(receivedCommand)
    })

    it('returns only sent commands when filter is "sent"', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      const sentCommand1 = createMockArduinoCommand({ direction: 'sent', data: 'sent1' })
      const sentCommand2 = createMockArduinoCommand({ direction: 'sent', data: 'sent2' })
      const receivedCommand = createMockArduinoCommand({ direction: 'received', data: 'received' })

      act(() => {
        result.current.addCommand(sentCommand1)
        result.current.addCommand(receivedCommand)
        result.current.addCommand(sentCommand2)
      })

      const filtered = result.current.getFilteredCommands('sent')
      expect(filtered).toHaveLength(2)
      expect(filtered).toContain(sentCommand1)
      expect(filtered).toContain(sentCommand2)
      expect(filtered).not.toContain(receivedCommand)
    })

    it('returns only received commands when filter is "received"', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      const sentCommand = createMockArduinoCommand({ direction: 'sent', data: 'sent' })
      const receivedCommand1 = createMockArduinoCommand({ direction: 'received', data: 'received1' })
      const receivedCommand2 = createMockArduinoCommand({ direction: 'received', data: 'received2' })

      act(() => {
        result.current.addCommand(sentCommand)
        result.current.addCommand(receivedCommand1)
        result.current.addCommand(receivedCommand2)
      })

      const filtered = result.current.getFilteredCommands('received')
      expect(filtered).toHaveLength(2)
      expect(filtered).toContain(receivedCommand1)
      expect(filtered).toContain(receivedCommand2)
      expect(filtered).not.toContain(sentCommand)
    })

    it('returns empty array when no commands match filter', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      const sentCommand = createMockArduinoCommand({ direction: 'sent', data: 'sent' })

      act(() => {
        result.current.addCommand(sentCommand)
      })

      const filtered = result.current.getFilteredCommands('received')
      expect(filtered).toHaveLength(0)
    })
  })

  describe('Performance', () => {
    it('handles rapid command additions efficiently', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 1000 }))

      const startTime = performance.now()

      act(() => {
        for (let i = 0; i < 500; i++) {
          result.current.addCommand(createMockArduinoCommand({
            data: `rapid_command_${i}`,
            timestamp: Date.now() + i
          }))
        }
      })

      const endTime = performance.now()

      expect(endTime - startTime).toBeLessThan(100) // Should complete in less than 100ms
      expect(result.current.getCommandCount()).toBe(500)
    })

    it('handles filtering large command sets efficiently', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 1000 }))

      // Add many commands
      act(() => {
        for (let i = 0; i < 500; i++) {
          result.current.addCommand(createMockArduinoCommand({
            direction: i % 2 === 0 ? 'sent' : 'received',
            data: `command_${i}`,
            timestamp: Date.now() + i
          }))
        }
      })

      const startTime = performance.now()
      const filtered = result.current.getFilteredCommands('sent')
      const endTime = performance.now()

      expect(endTime - startTime).toBeLessThan(50) // Should filter quickly
      expect(filtered).toHaveLength(250) // Half should be sent
    })
  })

  describe('Memory Management', () => {
    it('prevents memory leaks with max entries enforcement', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 10 }))

      // Add many more commands than the limit
      act(() => {
        for (let i = 0; i < 1000; i++) {
          result.current.addCommand(createMockArduinoCommand({
            data: `memory_test_${i}`,
            timestamp: Date.now() + i
          }))
        }
      })

      // Should only keep the last 10 commands
      expect(result.current.getCommandCount()).toBe(10)
      
      // Should contain the most recent commands
      const lastCommand = result.current.commands[result.current.commands.length - 1]
      expect(lastCommand.data).toBe('memory_test_999')
    })
  })

  describe('Edge Cases', () => {
    it('handles undefined command gracefully', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      expect(() => {
        act(() => {
          result.current.addCommand(undefined as any)
        })
      }).not.toThrow()

      expect(result.current.getCommandCount()).toBe(0)
    })

    it('handles null command gracefully', () => {
      const { result } = renderHook(() => useArduinoCommandLog())

      expect(() => {
        act(() => {
          result.current.addCommand(null as any)
        })
      }).not.toThrow()

      expect(result.current.getCommandCount()).toBe(0)
    })

    it('handles zero max entries', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: 0 }))

      act(() => {
        result.current.addCommand(createMockArduinoCommand({ data: 'test' }))
      })

      expect(result.current.getCommandCount()).toBe(0)
    })

    it('handles negative max entries', () => {
      const { result } = renderHook(() => useArduinoCommandLog({ maxEntries: -5 }))

      act(() => {
        result.current.addCommand(createMockArduinoCommand({ data: 'test' }))
      })

      expect(result.current.getCommandCount()).toBe(0)
    })
  })
})
