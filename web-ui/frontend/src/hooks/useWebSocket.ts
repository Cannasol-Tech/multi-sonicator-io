import { useState, useEffect, useRef, useCallback } from 'react'
import { WebSocketMessage } from '../types'
import { WEBSOCKET_CONFIG } from '../../../shared/constants'

export function useWebSocket(url: string) {
  const [connected, setConnected] = useState(false)
  const [lastMessage, setLastMessage] = useState<WebSocketMessage | null>(null)
  const [error, setError] = useState<string | null>(null)
  const ws = useRef<WebSocket | null>(null)
  const reconnectTimeoutRef = useRef<NodeJS.Timeout | undefined>(undefined)
  const reconnectAttempts = useRef(0)
  const maxReconnectAttempts = WEBSOCKET_CONFIG.MAX_RECONNECT_ATTEMPTS

  const connect = useCallback(() => {
    try {
      ws.current = new WebSocket(url)
      
      ws.current.onopen = () => {
        console.log('WebSocket connected')
        setConnected(true)
        setError(null)
        reconnectAttempts.current = 0
      }
      
      ws.current.onmessage = (event) => {
        try {
          const message: WebSocketMessage = JSON.parse(event.data)
          setLastMessage(message)
        } catch (err) {
          console.error('Failed to parse WebSocket message:', err)
        }
      }
      
      ws.current.onclose = (event) => {
        console.log('WebSocket disconnected:', event.code, event.reason)
        setConnected(false)
        
        // Attempt to reconnect if not a clean close
        if (event.code !== 1000 && reconnectAttempts.current < maxReconnectAttempts) {
          const delay = Math.min(
            WEBSOCKET_CONFIG.RECONNECT_DELAY * Math.pow(WEBSOCKET_CONFIG.RECONNECT_BACKOFF_MULTIPLIER, reconnectAttempts.current),
            WEBSOCKET_CONFIG.MAX_RECONNECT_DELAY
          )
          console.log(`Attempting to reconnect in ${delay}ms (attempt ${reconnectAttempts.current + 1})`)
          
          reconnectTimeoutRef.current = setTimeout(() => {
            reconnectAttempts.current++
            connect()
          }, delay)
        }
      }
      
      ws.current.onerror = (event) => {
        console.error('WebSocket error:', event)
        setError('WebSocket connection error')
      }
      
    } catch (err) {
      console.error('Failed to create WebSocket connection:', err)
      setError('Failed to create WebSocket connection')
    }
  }, [url])

  const disconnect = useCallback(() => {
    if (reconnectTimeoutRef.current) {
      clearTimeout(reconnectTimeoutRef.current)
    }
    
    if (ws.current) {
      ws.current.close(1000, 'User initiated disconnect')
      ws.current = null
    }
    
    setConnected(false)
  }, [])

  const sendMessage = useCallback((message: any) => {
    if (ws.current && ws.current.readyState === WebSocket.OPEN) {
      try {
        ws.current.send(JSON.stringify(message))
        return true
      } catch (err) {
        console.error('Failed to send WebSocket message:', err)
        setError('Failed to send message')
        return false
      }
    } else {
      console.warn('WebSocket not connected, cannot send message')
      setError('WebSocket not connected')
      return false
    }
  }, [])

  // Connect on mount
  useEffect(() => {
    connect()
    
    return () => {
      disconnect()
    }
  }, [connect, disconnect])

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      if (reconnectTimeoutRef.current) {
        clearTimeout(reconnectTimeoutRef.current)
      }
    }
  }, [])

  return {
    connected,
    lastMessage,
    error,
    sendMessage,
    reconnect: connect,
    disconnect
  }
}
