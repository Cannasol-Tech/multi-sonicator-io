import { useState, useCallback } from 'react'

export interface ArduinoCommand {
  direction: 'sent' | 'received'
  data: string
  timestamp: number
  type: string
}

interface UseArduinoCommandLogOptions {
  maxEntries?: number
}

export function useArduinoCommandLog(options: UseArduinoCommandLogOptions = {}) {
  const { maxEntries = 100 } = options
  const [commands, setCommands] = useState<ArduinoCommand[]>([])

  const addCommand = useCallback((command: ArduinoCommand) => {
    setCommands(prev => {
      const newCommands = [...prev, command]
      // Keep only the last maxEntries commands
      return newCommands.slice(-maxEntries)
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
    return commands.filter(cmd => cmd.direction === filter)
  }, [commands])

  return {
    commands,
    addCommand,
    clearCommands,
    getCommandCount,
    getFilteredCommands
  }
}
