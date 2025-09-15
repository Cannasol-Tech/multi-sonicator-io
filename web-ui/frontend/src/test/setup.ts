import '@testing-library/jest-dom'
import { expect, afterEach, vi } from 'vitest'
import { cleanup } from '@testing-library/react'
import * as matchers from '@testing-library/jest-dom/matchers'

// Extend expect with jest-dom matchers
expect.extend(matchers)

afterEach(() => {
  cleanup()
})

// Mock WebSocket for tests
class MockWebSocket {
  static CONNECTING = 0
  static OPEN = 1
  static CLOSING = 2
  static CLOSED = 3

  url: string
  readyState: number = MockWebSocket.CONNECTING
  binaryType: BinaryType = 'blob'
  bufferedAmount: number = 0
  extensions: string = ''
  protocol: string = ''
  onopen: ((event: Event) => void) | null = null
  onmessage: ((event: MessageEvent) => void) | null = null
  onclose: ((event: CloseEvent) => void) | null = null
  onerror: ((event: Event) => void) | null = null

  constructor(url: string | URL, protocols?: string | string[]) {
    this.url = url.toString()
    this.protocol = Array.isArray(protocols) ? protocols[0] || '' : protocols || ''
    // Simulate immediate connection for tests
    setTimeout(() => {
      this.readyState = MockWebSocket.OPEN
      if (this.onopen) {
        this.onopen(new Event('open'))
      }
    }, 0)
  }

  send(data: string | ArrayBufferLike | Blob | ArrayBufferView): void {
    // Mock send - do nothing
  }

  close(code?: number, reason?: string): void {
    this.readyState = MockWebSocket.CLOSED
    if (this.onclose) {
      this.onclose(new CloseEvent('close', { code: code || 1000, reason: reason || '' }))
    }
  }

  addEventListener(type: string, listener: EventListener): void {
    // Mock implementation - simplified
  }

  removeEventListener(type: string, listener: EventListener): void {
    // Mock implementation - simplified
  }

  dispatchEvent(event: Event): boolean {
    return true
  }
}

global.WebSocket = MockWebSocket as any

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

// Mock URL methods for file downloads and URL constructor
global.URL = class MockURL {
  static createObjectURL = vi.fn(() => 'mock-object-url')
  static revokeObjectURL = vi.fn()

  href: string
  protocol: string
  hostname: string
  port: string
  pathname: string
  search: string
  hash: string

  constructor(url: string, base?: string) {
    // Simple URL validation for test purposes
    const validProtocols = ['http:', 'https:', 'ws:', 'wss:', 'ftp:', 'file:']
    const hasValidProtocol = validProtocols.some(protocol => url.startsWith(protocol))

    // Check for obviously invalid URLs
    if (!hasValidProtocol || url === 'not-a-url' || url === 'http://' || url === '://example.com' || url.length < 8) {
      throw new TypeError('Invalid URL')
    }

    // Create a basic valid URL structure
    this.href = url
    this.protocol = url.startsWith('https:') ? 'https:' : url.startsWith('http:') ? 'http:' : url.startsWith('ws:') ? 'ws:' : url.startsWith('wss:') ? 'wss:' : 'http:'
    this.hostname = url.includes('localhost') ? 'localhost' : 'example.com'
    this.port = url.includes(':3000') ? '3000' : ''
    this.pathname = '/'
    this.search = ''
    this.hash = ''
  }
} as any
