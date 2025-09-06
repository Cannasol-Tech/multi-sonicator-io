import '@testing-library/jest-dom'
import { expect, afterEach, vi } from 'vitest'
import { cleanup } from '@testing-library/react'

// Cleanup after each test case (e.g. clearing jsdom)
afterEach(() => {
  cleanup()
})

// Mock WebSocket for tests
global.WebSocket = class MockWebSocket {
  static CONNECTING = 0
  static OPEN = 1
  static CLOSING = 2
  static CLOSED = 3

  readyState = MockWebSocket.OPEN  // Start as OPEN to avoid async issues
  url: string
  onopen: ((event: Event) => void) | null = null
  onclose: ((event: CloseEvent) => void) | null = null
  onmessage: ((event: MessageEvent) => void) | null = null
  onerror: ((event: Event) => void) | null = null

  constructor(url: string) {
    this.url = url
    // Immediately trigger onopen if set (synchronous for tests)
    if (this.onopen) {
      this.onopen(new Event('open'))
    }
  }

  send(data: string | ArrayBufferLike | Blob | ArrayBufferView) {
    // Mock send - do nothing in tests
  }

  close() {
    this.readyState = MockWebSocket.CLOSED
    if (this.onclose) {
      this.onclose(new CloseEvent('close'))
    }
  }

  addEventListener(type: string, listener: EventListener) {
    // Mock addEventListener
  }

  removeEventListener(type: string, listener: EventListener) {
    // Mock removeEventListener
  }

  dispatchEvent(event: Event): boolean {
    return true
  }
}

// Mock localStorage
const localStorageMock = {
  getItem: vi.fn(),
  setItem: vi.fn(),
  removeItem: vi.fn(),
  clear: vi.fn(),
}
global.localStorage = localStorageMock as any

// Mock fetch with proper responses
global.fetch = vi.fn().mockImplementation((url: string) => {
  // Mock configuration endpoint
  if (url.includes('/api/config')) {
    return Promise.resolve({
      ok: true,
      json: () => Promise.resolve({
        success: true,
        config: {
          theme: 'auto',
          keyboardShortcuts: true,
          autoSave: true,
          notifications: true,
          sonicator4: {
            operatingFrequencyKHz: 40.0,
            outputFrequencyKHz: 4.0,
            enabled: true,
            manualMode: false,
            manualFrequencyKHz: 2.0
          },
          powerLevel: 50,
          pulseMode: false,
          pulseDurationMs: 100,
          pulseIntervalMs: 200
        }
      })
    })
  }

  // Default mock response
  return Promise.resolve({
    ok: true,
    json: () => Promise.resolve({ success: true })
  })
})

// Mock matchMedia
Object.defineProperty(window, 'matchMedia', {
  writable: true,
  value: vi.fn().mockImplementation(query => ({
    matches: false,
    media: query,
    onchange: null,
    addListener: vi.fn(), // deprecated
    removeListener: vi.fn(), // deprecated
    addEventListener: vi.fn(),
    removeEventListener: vi.fn(),
    dispatchEvent: vi.fn(),
  })),
})

// Mock ResizeObserver
global.ResizeObserver = vi.fn().mockImplementation(() => ({
  observe: vi.fn(),
  unobserve: vi.fn(),
  disconnect: vi.fn(),
}))

// Mock IntersectionObserver
global.IntersectionObserver = vi.fn().mockImplementation(() => ({
  observe: vi.fn(),
  unobserve: vi.fn(),
  disconnect: vi.fn(),
}))
