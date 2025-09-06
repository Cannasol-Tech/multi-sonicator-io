import { useState, useCallback } from 'react'

export interface ArduinoCommand {
  direction: 'sent' | 'received'
  data: string
  timestamp: number
  type: string
}

export interface UseArduinoCommandLogOptions {
  maxEntries?: number
}

export interface UseArduinoCommandLogReturn {
  commands: ArduinoCommand[]
  addCommand: (command: ArduinoCommand) => void
  clearCommands: () => void
  getCommandCount: () => number
  getFilteredCommands: (filter: 'all' | 'sent' | 'received') => ArduinoCommand[]
}

export function useArduinoCommandLog(
  options: UseArduinoCommandLogOptions = {}
): UseArduinoCommandLogReturn {
  const { maxEntries = 200 } = options
  const [commands, setCommands] = useState<ArduinoCommand[]>([])

  const addCommand = useCallback((command: ArduinoCommand) => {
    if (!command || typeof command !== 'object') {
      return
    }

    setCommands(prevCommands => {
      const newCommands = [...prevCommands, command]

      // Enforce max entries limit - if maxEntries is 0 or negative, don't add anything
      if (maxEntries <= 0) {
        return []
      }

      if (newCommands.length > maxEntries) {
        return newCommands.slice(-maxEntries)
      }

      return newCommands
    })
  }, [maxEntries])

  const clearCommands = useCallback(() => {
    setCommands([])
  }, [])

  const getCommandCount = useCallback(() => {
    return commands.length
  }, [commands.length])

  const getFilteredCommands = useCallback((filter: 'all' | 'sent' | 'received') => {
    if (filter === 'all') {
      return commands
    }
    return commands.filter(command => command.direction === filter)
  }, [commands])

  return {
    commands,
    addCommand,
    clearCommands,
    getCommandCount,
    getFilteredCommands
  }
}
