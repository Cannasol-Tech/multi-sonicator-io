import { useState, useCallback } from 'react'

export interface ArduinoCommand {
  direction: 'sent' | 'received'
  data: string
  timestamp: number
  type: string
  id?: string // Unique identifier for command/response pairing
  responseTime?: number // Time taken for response (in ms)
  status?: 'pending' | 'success' | 'error' | 'timeout'
  error?: string // Error message if status is 'error'
}

export interface UseArduinoCommandLogOptions {
  maxEntries?: number
}

export interface UseArduinoCommandLogReturn {
  commands: ArduinoCommand[]
  addCommand: (command: ArduinoCommand) => void
  addCommandPair: (sentCommand: string, response: string, responseTime?: number, error?: string) => void
  clearCommands: () => void
  getCommandCount: () => number
  getFilteredCommands: (filter: 'all' | 'sent' | 'received') => ArduinoCommand[]
  getRecentPairs: (count?: number) => Array<{sent: ArduinoCommand, received?: ArduinoCommand}>
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

  const addCommandPair = useCallback((sentCommand: string, response: string, responseTime?: number, error?: string) => {
    const commandId = `cmd_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`
    const timestamp = Date.now()

    const sentCmd: ArduinoCommand = {
      direction: 'sent',
      data: sentCommand,
      timestamp,
      type: 'command',
      id: commandId,
      status: error ? 'error' : 'success',
      responseTime,
      error
    }

    const receivedCmd: ArduinoCommand = {
      direction: 'received',
      data: response,
      timestamp: timestamp + (responseTime || 0),
      type: 'response',
      id: commandId,
      status: error ? 'error' : 'success',
      responseTime,
      error
    }

    setCommands(prevCommands => {
      const newCommands = [...prevCommands, sentCmd, receivedCmd]

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

  const getRecentPairs = useCallback((count: number = 5) => {
    const commandMap = new Map<string, {sent?: ArduinoCommand, received?: ArduinoCommand}>()

    // Group commands by ID
    commands.forEach(command => {
      if (command.id) {
        if (!commandMap.has(command.id)) {
          commandMap.set(command.id, {})
        }
        const pair = commandMap.get(command.id)!
        if (command.direction === 'sent') {
          pair.sent = command
        } else {
          pair.received = command
        }
      }
    })

    // Convert to array and sort by timestamp (most recent first)
    const sortedPairs = Array.from(commandMap.values())
      .filter(pair => pair.sent) // Only include pairs with sent commands
      .sort((a, b) => (b.sent?.timestamp || 0) - (a.sent?.timestamp || 0))
      .slice(0, count)
      .map(pair => ({
        sent: pair.sent!,
        received: pair.received
      }))

    return sortedPairs
  }, [commands])

  return {
    commands,
    addCommand,
    addCommandPair,
    clearCommands,
    getCommandCount,
    getFilteredCommands,
    getRecentPairs
  }
}
