import { describe, it, expect } from 'vitest'
import { formatFrequency } from '../formatters'

// Utility functions for formatting data
export const formatTimestamp = (timestamp: number): string => {
  const date = new Date(timestamp)
  return date.toLocaleTimeString('en-US', { 
    hour12: false, 
    hour: '2-digit', 
    minute: '2-digit', 
    second: '2-digit',
    fractionalSecondDigits: 3
  })
}

export const formatPinState = (state: string): string => {
  if (state.startsWith('PWM=')) {
    return state
  }
  return state.toUpperCase()
}

export const formatConnectionStatus = (connected: boolean): string => {
  return connected ? 'Connected' : 'Disconnected'
}

export const formatDuration = (ms: number): string => {
  if (ms < 1000) {
    return `${ms}ms`
  }
  if (ms < 60000) {
    return `${(ms / 1000).toFixed(1)}s`
  }
  return `${(ms / 60000).toFixed(1)}m`
}

export const formatBytes = (bytes: number): string => {
  if (bytes === 0) return '0 B'
  
  const k = 1024
  const sizes = ['B', 'KB', 'MB', 'GB']
  const i = Math.floor(Math.log(bytes) / Math.log(k))
  
  return `${parseFloat((bytes / Math.pow(k, i)).toFixed(2))} ${sizes[i]}`
}

export const parseJsonSafely = (jsonString: string): any => {
  try {
    return JSON.parse(jsonString)
  } catch {
    return null
  }
}

export const validatePinSignal = (signal: string): boolean => {
  // Valid pin signals should match the pattern: SIGNAL_NAME_NUMBER or SIGNAL_NAME
  const validPattern = /^[A-Z_]+(_\d+)?$/
  return validPattern.test(signal)
}

export const sanitizeInput = (input: string): string => {
  return input.replace(/<[^>]*>/g, '').replace(/[\"'&]/g, '')
}

describe('Formatter Utilities', () => {
  describe('formatTimestamp', () => {
    it('formats timestamp correctly', () => {
      const timestamp = new Date('2023-01-01T12:30:45.123Z').getTime()
      const formatted = formatTimestamp(timestamp)
      
      expect(formatted).toMatch(/\d{2}:\d{2}:\d{2}\.\d{3}/)
    })

    it('handles current timestamp', () => {
      const now = Date.now()
      const formatted = formatTimestamp(now)
      
      expect(formatted).toMatch(/\d{2}:\d{2}:\d{2}\.\d{3}/)
    })
  })

  describe('formatPinState', () => {
    it('formats digital states correctly', () => {
      expect(formatPinState('high')).toBe('HIGH')
      expect(formatPinState('low')).toBe('LOW')
      expect(formatPinState('HIGH')).toBe('HIGH')
    })

    it('preserves PWM states', () => {
      expect(formatPinState('PWM=50%')).toBe('PWM=50%')
      expect(formatPinState('PWM=100%')).toBe('PWM=100%')
    })

    it('handles empty states', () => {
      expect(formatPinState('')).toBe('')
    })
  })

  describe('formatConnectionStatus', () => {
    it('formats connection status correctly', () => {
      expect(formatConnectionStatus(true)).toBe('Connected')
      expect(formatConnectionStatus(false)).toBe('Disconnected')
    })
  })

  describe('formatDuration', () => {
    it('formats milliseconds correctly', () => {
      expect(formatDuration(500)).toBe('500ms')
      expect(formatDuration(999)).toBe('999ms')
    })

    it('formats seconds correctly', () => {
      expect(formatDuration(1000)).toBe('1.0s')
      expect(formatDuration(1500)).toBe('1.5s')
      expect(formatDuration(59999)).toBe('60.0s')
    })

    it('formats minutes correctly', () => {
      expect(formatDuration(60000)).toBe('1.0m')
      expect(formatDuration(90000)).toBe('1.5m')
    })
  })

  describe('formatBytes', () => {
    it('formats bytes correctly', () => {
      expect(formatBytes(0)).toBe('0 B')
      expect(formatBytes(512)).toBe('512 B')
      expect(formatBytes(1024)).toBe('1 KB')
      expect(formatBytes(1536)).toBe('1.5 KB')
      expect(formatBytes(1048576)).toBe('1 MB')
    })
  })

  describe('parseJsonSafely', () => {
    it('parses valid JSON', () => {
      const validJson = '{"type":"test","value":123}'
      const result = parseJsonSafely(validJson)
      
      expect(result).toEqual({ type: 'test', value: 123 })
    })

    it('returns null for invalid JSON', () => {
      const invalidJson = '{"invalid": json}'
      const result = parseJsonSafely(invalidJson)
      
      expect(result).toBeNull()
    })

    it('handles empty string', () => {
      const result = parseJsonSafely('')
      expect(result).toBeNull()
    })
  })

  describe('validatePinSignal', () => {
    it('validates correct pin signals', () => {
      expect(validatePinSignal('START_STOP_4')).toBe(true)
      expect(validatePinSignal('RESET_4')).toBe(true)
      expect(validatePinSignal('POWER_SENSE')).toBe(true)
      expect(validatePinSignal('AMPLITUDE_ALL')).toBe(true)
    })

    it('rejects invalid pin signals', () => {
      expect(validatePinSignal('invalid-signal')).toBe(false)
      expect(validatePinSignal('123_INVALID')).toBe(false)
      expect(validatePinSignal('signal with spaces')).toBe(false)
      expect(validatePinSignal('')).toBe(false)
    })
  })

  describe('sanitizeInput', () => {
    it('removes dangerous characters', () => {
      expect(sanitizeInput('test<script>alert("xss")</script>')).toBe('testalert(xss)')
      expect(sanitizeInput('normal text')).toBe('normal text')
      expect(sanitizeInput('test"quote\'test')).toBe('testquotetest')
    })

    it('handles empty input', () => {
      expect(sanitizeInput('')).toBe('')
    })
  })

  describe('formatFrequency', () => {
    it('formats Hz frequencies with two decimal places', () => {
      expect(formatFrequency(100)).toBe('100.00 Hz')
      expect(formatFrequency(500.5)).toBe('500.50 Hz')
      expect(formatFrequency(999.99)).toBe('999.99 Hz')
    })

    it('formats kHz frequencies with two decimal places', () => {
      expect(formatFrequency(1000)).toBe('1.00 kHz')
      expect(formatFrequency(1500)).toBe('1.50 kHz')
      expect(formatFrequency(20000)).toBe('20.00 kHz')
      expect(formatFrequency(22500.75)).toBe('22.50 kHz')
    })

    it('formats MHz frequencies with two decimal places', () => {
      expect(formatFrequency(1000000)).toBe('1.00 MHz')
      expect(formatFrequency(1500000)).toBe('1.50 MHz')
      expect(formatFrequency(2500000.5)).toBe('2.50 MHz')
    })

    it('handles edge cases correctly', () => {
      expect(formatFrequency(0)).toBe('0.00 Hz')
      expect(formatFrequency(999)).toBe('999.00 Hz')
      expect(formatFrequency(999999)).toBe('1000.00 kHz')
    })
  })
})
