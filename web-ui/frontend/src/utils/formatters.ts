/**
 * Utility functions for formatting data in the Multi-Sonicator IO application
 */

/**
 * Formats a timestamp to a human-readable string
 */
export function formatTimestamp(timestamp: number | null | undefined): string {
  if (!timestamp) return 'Never'
  
  const date = new Date(timestamp)
  return date.toLocaleTimeString()
}

/**
 * Formats pin state for display
 */
export function formatPinState(state: string): string {
  if (!state) return 'UNKNOWN'
  
  // Preserve PWM states as-is
  if (state.startsWith('PWM=')) {
    return state
  }
  
  // Convert digital states to uppercase
  return state.toUpperCase()
}

/**
 * Formats connection status for display
 */
export function formatConnectionStatus(connected: boolean): string {
  return connected ? 'Connected' : 'Disconnected'
}

/**
 * Formats duration in milliseconds to human-readable string
 */
export function formatDuration(milliseconds: number): string {
  if (milliseconds < 1000) {
    return `${milliseconds}ms`
  } else if (milliseconds < 60000) {
    return `${(milliseconds / 1000).toFixed(1)}s`
  } else {
    const minutes = Math.floor(milliseconds / 60000)
    const seconds = Math.floor((milliseconds % 60000) / 1000)
    return `${minutes}m ${seconds}s`
  }
}

/**
 * Formats bytes to human-readable string
 */
export function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 Bytes'
  
  const k = 1024
  const sizes = ['Bytes', 'KB', 'MB', 'GB']
  const i = Math.floor(Math.log(bytes) / Math.log(k))
  
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i]
}

/**
 * Safely parses JSON string, returns null if invalid
 */
export function parseJsonSafely<T = any>(jsonString: string): T | null {
  try {
    return JSON.parse(jsonString)
  } catch {
    return null
  }
}

/**
 * Validates if a string is a valid pin signal name
 */
export function validatePinSignal(signal: string): boolean {
  // Valid pin signals: D0-D13, A0-A7, RESET_4, etc.
  const pinPattern = /^(D\d{1,2}|A\d|RESET_\d+|PWM_\d+)$/
  return pinPattern.test(signal)
}

/**
 * Sanitizes input by removing potentially dangerous characters
 */
export function sanitizeInput(input: string): string {
  if (!input) return ''
  
  // Remove HTML tags and dangerous characters
  return input
    .replace(/<[^>]*>/g, '') // Remove HTML tags
    .replace(/['"]/g, '') // Remove quotes
    .replace(/[<>]/g, '') // Remove angle brackets
    .trim()
}

/**
 * Formats test execution statistics
 */
export function formatTestStats(stats: {
  total: number
  passed: number
  failed: number
  duration: number
}): string {
  const { total, passed, failed, duration } = stats
  return `${total} tests, ${passed} passed, ${failed} failed (${formatDuration(duration)})`
}

/**
 * Formats pin direction for display
 */
export function formatPinDirection(direction: 'IN' | 'OUT'): string {
  return direction === 'IN' ? 'Input' : 'Output'
}

/**
 * Formats frequency value with appropriate units
 */
export function formatFrequency(frequency: number): string {
  if (frequency < 1000) {
    return `${frequency.toFixed(2)} Hz`
  } else if (frequency < 1000000) {
    return `${(frequency / 1000).toFixed(2)} kHz`
  } else {
    return `${(frequency / 1000000).toFixed(2)} MHz`
  }
}

/**
 * Formats PWM value as percentage
 */
export function formatPwmPercentage(value: number): string {
  const percentage = Math.round((value / 255) * 100)
  return `${percentage}%`
}

/**
 * Formats scenario execution time
 */
export function formatExecutionTime(startTime: number, endTime?: number): string {
  if (!endTime) {
    const elapsed = Date.now() - startTime
    return `Running for ${formatDuration(elapsed)}`
  }
  
  const duration = endTime - startTime
  return formatDuration(duration)
}

/**
 * Formats error message for display
 */
export function formatErrorMessage(error: Error | string): string {
  if (typeof error === 'string') {
    return error
  }
  
  return error.message || 'An unknown error occurred'
}

/**
 * Formats file size for display
 */
export function formatFileSize(size: number): string {
  return formatBytes(size)
}

/**
 * Formats version string
 */
export function formatVersion(version: string): string {
  // Ensure version follows semantic versioning format
  const versionPattern = /^\d+\.\d+\.\d+/
  if (versionPattern.test(version)) {
    return `v${version}`
  }
  return version
}

/**
 * Formats URL for display (truncates if too long)
 */
export function formatUrl(url: string, maxLength: number = 50): string {
  if (url.length <= maxLength) {
    return url
  }
  
  return url.substring(0, maxLength - 3) + '...'
}

/**
 * Formats command for display in logs
 */
export function formatCommand(command: any): string {
  if (typeof command === 'string') {
    return command
  }
  
  if (typeof command === 'object') {
    return JSON.stringify(command, null, 2)
  }
  
  return String(command)
}

/**
 * Formats tag for display
 */
export function formatTag(tag: string): string {
  // Ensure tag starts with @
  if (!tag.startsWith('@')) {
    return `@${tag}`
  }
  return tag
}

/**
 * Formats scenario name for display
 */
export function formatScenarioName(name: string): string {
  // Capitalize first letter and replace underscores with spaces
  return name
    .replace(/_/g, ' ')
    .replace(/\b\w/g, l => l.toUpperCase())
}

/**
 * Formats step description for display
 */
export function formatStepDescription(description: string): string {
  // Trim and ensure proper capitalization
  return description.trim().replace(/^\w/, c => c.toUpperCase())
}

/**
 * Formats connection info for display
 */
export function formatConnectionInfo(info: {
  url?: string
  status: string
  lastConnected?: number
}): string {
  const { url, status, lastConnected } = info
  let result = `Status: ${status}`
  
  if (url) {
    result += ` | URL: ${formatUrl(url)}`
  }
  
  if (lastConnected) {
    result += ` | Last: ${formatTimestamp(lastConnected)}`
  }
  
  return result
}

/**
 * Formats settings value for display
 */
export function formatSettingsValue(value: any): string {
  if (typeof value === 'boolean') {
    return value ? 'Enabled' : 'Disabled'
  }
  
  if (typeof value === 'number') {
    return value.toString()
  }
  
  if (typeof value === 'string') {
    return value
  }
  
  if (Array.isArray(value)) {
    return value.join(', ')
  }
  
  return JSON.stringify(value)
}
