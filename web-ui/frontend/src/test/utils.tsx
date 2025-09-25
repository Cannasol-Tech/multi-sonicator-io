import React, { ReactElement } from 'react'
import { render, RenderOptions } from '@testing-library/react'
import { vi, expect } from 'vitest'
import { HardwareState, PinState } from '../types'

// Mock hardware state for testing
export const createMockHardwareState = (overrides?: Partial<HardwareState>): HardwareState => ({
  connection: {
    connected: true,
    lastSeen: Date.now()
  },
  lastUpdate: Date.now(),
  pins: {
    'START_STOP_4': {
      pin: 'D3',
      signal: 'START_STOP_4',
      direction: 'IN',
      state: 'LOW',
      timestamp: Date.now()
    },
    'RESET_4': {
      pin: 'A4',
      signal: 'RESET_4',
      direction: 'IN',
      state: 'LOW',
      timestamp: Date.now()
    },
    'POWER_SENSE_4': {
      pin: 'A1',
      signal: 'POWER_SENSE_4',
      direction: 'IN',
      state: 'LOW',
      timestamp: Date.now()
    },
    'FREQ_DIV10_4': {
      pin: 'D5',
      signal: 'FREQ_DIV10_4',
      direction: 'OUT',
      state: 'LOW',
      timestamp: Date.now()
    },
    'AMPLITUDE_ALL': {
      pin: 'D6',
      signal: 'AMPLITUDE_ALL',
      direction: 'OUT',
      state: 'PWM=50%',
      timestamp: Date.now()
    }
  },
  ...overrides
})

// Mock pin state
export const createMockPinState = (overrides?: Partial<PinState>): PinState => ({
  pin: 'D3',
  signal: 'TEST_PIN',
  direction: 'IN',
  state: 'LOW',
  timestamp: Date.now(),
  ...overrides
})

// Mock WebSocket message
export const createMockWebSocketMessage = (type: string, data: any) => ({
  type,
  data,
  timestamp: Date.now()
})

// Mock test execution data
export const createMockTestExecution = (overrides?: any) => ({
  id: 'test-123',
  name: 'Test Execution',
  status: 'running',
  startTime: Date.now(),
  steps: [
    {
      id: 'step-1',
      name: 'Set START_STOP_4 to HIGH',
      status: 'completed',
      duration: 100
    },
    {
      id: 'step-2',
      name: 'Wait 1000ms',
      status: 'running',
      duration: 0
    }
  ],
  progress: {
    current: 1,
    total: 5,
    percentage: 20
  },
  ...overrides
})

// Mock user preferences
export const createMockUserPreferences = (overrides?: any) => ({
  theme: 'auto' as const,
  animationsEnabled: true,
  soundEnabled: false,
  autoRefreshInterval: 1000,
  defaultTimeRange: '5m' as const,
  compactMode: false,
  showTooltips: true,
  keyboardShortcuts: true,
  exportFormat: 'csv' as const,
  maxHistoryEntries: 1000,
  ...overrides
})

// Mock Arduino command
export const createMockArduinoCommand = (overrides?: any) => ({
  direction: 'sent' as const,
  data: '{"type":"pin_read","pin":"D7"}',
  timestamp: Date.now(),
  type: 'pin_read',
  ...overrides
})

// Custom render function that includes common providers
interface CustomRenderOptions extends Omit<RenderOptions, 'wrapper'> {
  // Add any custom options here
}

export const renderWithProviders = (
  ui: ReactElement,
  options?: CustomRenderOptions
) => {
  // If we had providers (like Redux, Context, etc.), we'd wrap them here
  const Wrapper = ({ children }: { children: React.ReactNode }) => {
    return <>{children}</>
  }

  return render(ui, { wrapper: Wrapper, ...options })
}

// Helper to simulate user interactions
export const simulateKeyPress = (key: string, element?: Element) => {
  const event = new KeyboardEvent('keydown', {
    key,
    code: key,
    bubbles: true,
    cancelable: true
  })
  
  const target = element || document.body
  target.dispatchEvent(event)
}

// Helper to simulate WebSocket messages
export const simulateWebSocketMessage = (mockWebSocket: any, message: any) => {
  if (mockWebSocket.onmessage) {
    mockWebSocket.onmessage({
      data: JSON.stringify(message),
      type: 'message'
    } as MessageEvent)
  }
}

// Helper to wait for async operations
export const waitFor = (ms: number) => new Promise(resolve => setTimeout(resolve, ms))

// Mock fetch responses
export const mockFetchResponse = (data: any, ok = true, status = 200) => {
  return Promise.resolve({
    ok,
    status,
    json: () => Promise.resolve(data),
    text: () => Promise.resolve(JSON.stringify(data))
  } as Response)
}

// Helper to create mock event handlers
export const createMockHandlers = () => ({
  onPinControl: vi.fn(),
  onPreferencesChange: vi.fn(),
  onTestStart: vi.fn(),
  onTestStop: vi.fn(),
  onBatchOperation: vi.fn(),
  onPinClick: vi.fn(),
  onConnect: vi.fn(),
  onDisconnect: vi.fn()
})

// Helper to assert element visibility
export const expectElementToBeVisible = (element: HTMLElement | null) => {
  expect(element).toBeInTheDocument()
  expect(element).toBeVisible()
}

// Helper to assert element text content
export const expectElementToHaveText = (element: HTMLElement | null, text: string) => {
  expect(element).toBeInTheDocument()
  expect(element).toHaveTextContent(text)
}

// Helper to assert button state
export const expectButtonToBeEnabled = (button: HTMLElement | null) => {
  expect(button).toBeInTheDocument()
  expect(button).toBeEnabled()
}

export const expectButtonToBeDisabled = (button: HTMLElement | null) => {
  expect(button).toBeInTheDocument()
  expect(button).toBeDisabled()
}

// Re-export everything from testing library
export * from '@testing-library/react'
export { default as userEvent } from '@testing-library/user-event'
