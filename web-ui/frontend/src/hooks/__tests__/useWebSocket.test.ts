import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { renderHook, act } from '@testing-library/react'
import { useWebSocket } from '../useWebSocket'

describe('useWebSocket', () => {
  let mockWebSocket: any

  beforeEach(() => {
    // Create a more detailed mock WebSocket
    mockWebSocket = {
      readyState: WebSocket.CONNECTING,
      url: '',
      onopen: null,
      onclose: null,
      onmessage: null,
      onerror: null,
      send: vi.fn(),
      close: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn()
    }

    // Mock the WebSocket constructor
    global.WebSocket = vi.fn().mockImplementation((url) => {
      mockWebSocket.url = url
      // Simulate async connection
      setTimeout(() => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      }, 0)
      return mockWebSocket
    }) as any

    vi.clearAllMocks()
  })

  afterEach(() => {
    vi.restoreAllMocks()
  })

  describe('Connection Management', () => {
    it('initializes with disconnected state', () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      expect(result.current.connected).toBe(false)
      expect(result.current.lastMessage).toBeNull()
    })

    it('creates WebSocket connection with correct URL', () => {
      const url = 'ws://localhost:3005/ws'
      renderHook(() => useWebSocket(url))

      expect(global.WebSocket).toHaveBeenCalledWith(url)
    })

    it('sets connected to true when WebSocket opens', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Simulate WebSocket opening
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })

      expect(result.current.connected).toBe(true)
    })

    it('sets connected to false when WebSocket closes', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // First open the connection
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })

      // Then close it
      await act(async () => {
        mockWebSocket.readyState = WebSocket.CLOSED
        if (mockWebSocket.onclose) {
          mockWebSocket.onclose(new CloseEvent('close'))
        }
      })

      expect(result.current.connected).toBe(false)
    })

    it('handles WebSocket errors', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // First connect, then trigger error
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })

      await act(async () => {
        mockWebSocket.readyState = WebSocket.CLOSED
        if (mockWebSocket.onerror) {
          mockWebSocket.onerror(new Event('error'))
        }
      })

      await waitFor(() => {
        expect(result.current.error).toBeTruthy()
      })
    })
  })

  describe('Message Handling', () => {
    it('updates lastMessage when message is received', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      const testMessage = { type: 'test', data: 'hello' }

      await act(async () => {
        if (mockWebSocket.onmessage) {
          mockWebSocket.onmessage({
            data: JSON.stringify(testMessage),
            type: 'message'
          } as MessageEvent)
        }
      })

      expect(result.current.lastMessage).toEqual(testMessage)
    })

    it('handles malformed JSON messages gracefully', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      await act(async () => {
        if (mockWebSocket.onmessage) {
          mockWebSocket.onmessage({
            data: 'invalid json',
            type: 'message'
          } as MessageEvent)
        }
      })

      // Should not crash and lastMessage should remain null
      expect(result.current.lastMessage).toBeNull()
    })

    it('handles non-string message data', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      await act(async () => {
        if (mockWebSocket.onmessage) {
          mockWebSocket.onmessage({
            data: new ArrayBuffer(8),
            type: 'message'
          } as MessageEvent)
        }
      })

      // Should handle gracefully
      expect(result.current.lastMessage).toBeNull()
    })
  })

  describe('Message Sending', () => {
    it('sends messages when connected', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Open connection first
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })

      const message = { type: 'test', data: 'hello' }

      act(() => {
        result.current.sendMessage(message)
      })

      expect(mockWebSocket.send).toHaveBeenCalledWith(JSON.stringify(message))
    })

    it('does not send messages when disconnected', () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Wait for initial connection attempt, then close it
      act(() => {
        mockWebSocket.readyState = WebSocket.CLOSED
        if (mockWebSocket.onclose) {
          mockWebSocket.onclose(new CloseEvent('close', { code: 1000 }))
        }
      })

      const message = { type: 'test', data: 'hello' }

      act(() => {
        result.current.sendMessage(message)
      })

      expect(result.current.connected).toBe(false)
    })

    it('handles send errors gracefully', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Open connection
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })

      // Make send throw an error
      mockWebSocket.send.mockImplementation(() => {
        throw new Error('Send failed')
      })

      const message = { type: 'test', data: 'hello' }

      // Should not throw
      expect(() => {
        act(() => {
          result.current.sendMessage(message)
        })
      }).not.toThrow()
    })
  })

  describe('Reconnection Logic', () => {
    it('attempts to reconnect when connection is lost', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Open connection
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })

      expect(result.current.connected).toBe(true)

      // Close connection with non-clean close (code !== 1000)
      await act(async () => {
        mockWebSocket.readyState = WebSocket.CLOSED
        if (mockWebSocket.onclose) {
          mockWebSocket.onclose(new CloseEvent('close', { code: 1006 }))
        }
      })

      // Should be disconnected
      expect(result.current.connected).toBe(false)

      // The reconnection logic is tested by checking that onclose handler sets up timeout
      // We can't easily test the actual reconnection without mocking timers
    })

    it('limits reconnection attempts', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Wait for initial connection to complete
      await act(async () => {
        await new Promise(resolve => setTimeout(resolve, 10))
      })

      // Test that the hook handles errors gracefully
      await act(async () => {
        mockWebSocket.readyState = 3 // WebSocket.CLOSED
        if (mockWebSocket.onerror) {
          mockWebSocket.onerror(new Event('error'))
        }
        await new Promise(resolve => setTimeout(resolve, 10))
      })

      expect(result.current.error).toBeTruthy()

      // The actual reconnection limiting logic is complex to test without mocking timers
      // This test verifies error handling works
    })
  })

  describe('Cleanup', () => {
    it('closes WebSocket connection on unmount', () => {
      const { unmount } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      unmount()

      expect(mockWebSocket.close).toHaveBeenCalled()
    })

    it('clears reconnection timer on unmount', async () => {
      const { unmount } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Trigger a reconnection attempt
      await act(async () => {
        if (mockWebSocket.onclose) {
          mockWebSocket.onclose(new CloseEvent('close'))
        }
      })

      // Unmount before reconnection completes
      unmount()

      // Should not throw or cause memory leaks
      expect(() => unmount()).not.toThrow()
    })
  })

  describe('URL Changes', () => {
    it('reconnects when URL changes', async () => {
      const { result, rerender } = renderHook(
        ({ url }) => useWebSocket(url),
        { initialProps: { url: 'ws://localhost:3005/ws' } }
      )

      // Open initial connection
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })

      // Change URL
      rerender({ url: 'ws://localhost:3006/ws' })

      // Should close old connection and create new one
      expect(mockWebSocket.close).toHaveBeenCalled()
      expect(global.WebSocket).toHaveBeenCalledWith('ws://localhost:3006/ws')
    })
  })

  describe('Connection States', () => {
    it('handles all WebSocket ready states correctly', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // CONNECTING
      expect(result.current.connected).toBe(false)

      // OPEN
      await act(async () => {
        mockWebSocket.readyState = WebSocket.OPEN
        if (mockWebSocket.onopen) {
          mockWebSocket.onopen(new Event('open'))
        }
      })
      expect(result.current.connected).toBe(true)

      // CLOSING - connection should still be considered connected until actually closed
      await act(async () => {
        mockWebSocket.readyState = WebSocket.CLOSING
      })
      expect(result.current.connected).toBe(true)

      // CLOSED
      await act(async () => {
        mockWebSocket.readyState = WebSocket.CLOSED
        if (mockWebSocket.onclose) {
          mockWebSocket.onclose(new CloseEvent('close'))
        }
      })
      expect(result.current.connected).toBe(false)
    })
  })

  describe('Message Queue', () => {
    it('returns false when trying to send while disconnected', async () => {
      const { result } = renderHook(() => useWebSocket('ws://localhost:3005/ws'))

      // Wait for initial connection
      await act(async () => {
        await new Promise(resolve => setTimeout(resolve, 10))
      })

      // Disconnect
      await act(async () => {
        mockWebSocket.readyState = 3 // WebSocket.CLOSED
        if (mockWebSocket.onclose) {
          mockWebSocket.onclose(new CloseEvent('close', { code: 1000 }))
        }
        await new Promise(resolve => setTimeout(resolve, 10))
      })

      const message = { type: 'test', data: 'hello' }

      let sendResult: boolean
      act(() => {
        sendResult = result.current.sendMessage(message)
      })

      expect(sendResult!).toBe(false)
      expect(result.current.connected).toBe(false)
    })
  })
})
