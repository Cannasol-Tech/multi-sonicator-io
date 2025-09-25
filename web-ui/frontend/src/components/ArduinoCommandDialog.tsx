import React, { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import { useWebSocket } from '../hooks/useWebSocket'
import { useArduinoCommandLog } from '../hooks/useArduinoCommandLog'
import { WebSocketMessage } from '../types'

interface ArduinoCommandDialogProps {
  visible: boolean
  onClose: () => void
}

const WS_URL = 'ws://localhost:3001/ws'

const COMMON_COMMANDS = [
  'PING',
  'GET_STATUS',
  'READ_PIN OVERLOAD_4',
  'READ_PIN FREQ_LOCK_4',
  'WRITE_PIN START_4 HIGH',
  'WRITE_PIN START_4 LOW',
  'SET_FREQUENCY FREQ_DIV10_4 1000'
]

export default function ArduinoCommandDialog({ visible, onClose }: ArduinoCommandDialogProps) {
  const [input, setInput] = useState('')
  const [suggestions, setSuggestions] = useState<string[]>([])
  const [status, setStatus] = useState<'idle'|'pending'|'success'|'error'>('idle')
  const [error, setError] = useState<string | null>(null)
  const [lastResponse, setLastResponse] = useState<any>(null)
  const [connectionAware, setConnectionAware] = useState(true)
  const [sending, setSending] = useState(false)
  const inputRef = useRef<HTMLInputElement>(null)

  const { connected, lastMessage, error: wsError, sendMessage } = useWebSocket(WS_URL)
  const { commands, addCommand, addCommandPair, clearCommands, getRecentPairs } = useArduinoCommandLog({ maxEntries: 300 })

  const connectionStatus = useMemo(() => {
    if (wsError) return { text: 'Error', color: 'var(--color-error)' }
    if (connected) return { text: 'Connected', color: 'var(--color-success)' }
    return { text: 'Disconnected', color: 'var(--color-error)' }
  }, [connected, wsError])

  useEffect(() => {
    if (!visible) return
    // Autofocus input when dialog opens
    const t = setTimeout(() => inputRef.current?.focus(), 50)
    return () => clearTimeout(t)
  }, [visible])

  // Suggestions
  useEffect(() => {
    const q = input.trim().toUpperCase()
    if (!q) {
      setSuggestions([])
      return
    }
    const list = COMMON_COMMANDS.filter(c => c.startsWith(q)).slice(0, 5)
    setSuggestions(list)
  }, [input])

  // Handle incoming websocket messages
  useEffect(() => {
    if (!lastMessage) return
    const msg = lastMessage as WebSocketMessage
    if (msg.type === 'command_response' || msg.type === 'arduino_command_response' || msg.type === 'error') {
      setLastResponse(msg)
      setStatus(msg.type === 'error' ? 'error' : 'success')
      setSending(false)
      const responseText = JSON.stringify(msg.data?.response ?? msg.data ?? {}, null, 0)
      addCommand({ direction: 'received', data: responseText, timestamp: Date.now(), type: msg.type, status: msg.type === 'error' ? 'error' : 'success', error: msg.data?.error })
    }
  }, [lastMessage, addCommand])

  const doSend = useCallback((commandText: string) => {
    if (!commandText.trim()) return

    if (connectionAware && !connected) {
      setError('WebSocket not connected')
      setStatus('error')
      return
    }

    setError(null)
    setStatus('pending')
    setSending(true)

    const payload = {
      type: 'hardware_command',
      data: {
        command: commandText.trim()
      },
      timestamp: Date.now()
    }

    const sendStart = performance.now()
    const ok = sendMessage(payload)
    addCommand({ direction: 'sent', data: commandText.trim(), timestamp: Date.now(), type: 'hardware_command', status: ok ? 'pending' : 'error', error: ok ? undefined : 'Send failed' })

    if (!ok) {
      setError('Failed to send command')
      setStatus('error')
      setSending(false)
      return
    }

    // Fallback timeout if no response comes back via WS quickly
    const timeout = setTimeout(() => {
      if (sending) {
        setStatus('error')
        setError('Timeout waiting for response')
        addCommand({ direction: 'received', data: 'Timeout', timestamp: Date.now(), type: 'error', status: 'timeout', error: 'Timeout' })
        setSending(false)
      }
    }, 7000)

    // Pairing is handled when response arrives; we still compute response time here if lastResponse updates
    const handleResponse = (evt: MessageEvent) => {
      try {
        const msg = JSON.parse((evt as any).data) as WebSocketMessage
        if (msg?.type === 'command_response') {
          const responseTime = Math.round(performance.now() - sendStart)
          addCommandPair(commandText.trim(), JSON.stringify(msg.data?.response ?? msg.data ?? {}), responseTime, msg.data?.error)
          clearTimeout(timeout)
          window.removeEventListener('message', handleResponse as any)
        }
      } catch (_) {
        // ignore
      }
    }

    // Note: we don't have direct event injects; pairing is done in lastMessage effect as well.
    // The window listener is a lightweight fallback for environments that re-dispatch messages.
    window.addEventListener('message', handleResponse as any)
  }, [connected, connectionAware, sendMessage, addCommand, addCommandPair])

  const handleSubmit = useCallback((e: React.FormEvent) => {
    e.preventDefault()
    doSend(input)
  }, [doSend, input])

  const handleSuggestionClick = (s: string) => {
    setInput(s)
    doSend(s)
  }

  const handleQuick = (cmd: string) => () => handleSuggestionClick(cmd)

  const copyToClipboard = (text: string) => {
    navigator.clipboard?.writeText(text).catch(() => {})
  }

  if (!visible) return null

  return (
    <div className="modal-overlay" onClick={onClose} role="dialog" aria-modal="true" aria-label="Arduino Command Dialog">
      <div className="modal-content arduino-command-dialog" onClick={e => e.stopPropagation()}>
        <div className="modal-header">
          <h2>
            🧪 Arduino Command Dialog
            <span className="connection-chip" style={{ background: connected ? 'var(--color-success)' : 'var(--color-error)' }}>
              {connectionStatus.text}
            </span>
          </h2>
          <button className="modal-close-btn" onClick={onClose} aria-label="Close dialog">×</button>
        </div>

        {/* Status/Error banner */}
        {(status === 'error' || error) && (
          <div className="error-banner" role="alert">
            <span>⚠️ {error || 'An error occurred'}</span>
            <button className="error-close" onClick={() => { setError(null); setStatus('idle') }} aria-label="Dismiss error">×</button>
          </div>
        )}

        {/* Command input */}
        <form onSubmit={handleSubmit} className="cmd-form">
          <div className="cmd-row">
            <input
              ref={inputRef}
              className="input cmd-input"
              placeholder="Type a command (e.g., PING, READ_PIN OVERLOAD_4)"
              value={input}
              onChange={e => setInput(e.target.value)}
              aria-label="Command input"
            />
            <button type="submit" className={`btn primary ${sending ? 'loading' : ''}`} disabled={sending} aria-label="Send command">
              {sending ? 'Sending…' : 'Send'}
            </button>
          </div>
          {suggestions.length > 0 && (
            <div className="cmd-suggestions" role="listbox" aria-label="Suggestions">
              {suggestions.map(s => (
                <button key={s} type="button" className="suggestion" onClick={() => handleSuggestionClick(s)}>
                  {s}
                </button>
              ))}
            </div>
          )}
        </form>

        {/* Quick commands */}
        <div className="quick-commands">
          <div className="qc-row">
            <button className="btn" onClick={handleQuick('PING')}>PING</button>
            <button className="btn" onClick={handleQuick('GET_STATUS')}>GET_STATUS</button>
            <button className="btn" onClick={handleQuick('READ_PIN OVERLOAD_4')}>READ OVERLOAD_4</button>
            <button className="btn" onClick={handleQuick('READ_PIN FREQ_LOCK_4')}>READ FREQ_LOCK_4</button>
          </div>
          <div className="qc-row">
            <button className="btn" onClick={handleQuick('WRITE_PIN START_4 HIGH')}>START_4 HIGH</button>
            <button className="btn" onClick={handleQuick('WRITE_PIN START_4 LOW')}>START_4 LOW</button>
            <button className="btn" onClick={handleQuick('SET_FREQUENCY FREQ_DIV10_4 1000')}>FREQ ÷10 @1kHz</button>
          </div>
        </div>

        {/* Response/History */}
        <div className="cmd-panels">
          <div className="panel">
            <div className="panel-header">
              <h3>Response</h3>
              {lastResponse && (
                <button className="btn" onClick={() => copyToClipboard(JSON.stringify(lastResponse, null, 2))}>Copy</button>
              )}
            </div>
            <pre className="response-box" aria-live="polite">{lastResponse ? JSON.stringify(lastResponse, null, 2) : 'No response yet'}</pre>
          </div>

          <div className="panel">
            <div className="panel-header">
              <h3>Command History</h3>
              <div className="panel-actions">
                <button className="btn" onClick={() => copyToClipboard(commands.map(c => `${c.direction.toUpperCase()}: ${c.data}`).join('\n'))}>Copy</button>
                <button className="btn danger" onClick={() => clearCommands()}>Clear</button>
              </div>
            </div>
            <div className="history-list scrollable">
              {getRecentPairs(10).map((pair, idx) => (
                <div key={idx} className="history-pair">
                  <div className="history-item sent">
                    <span className="hi-meta">→</span>
                    <span className="hi-text">{pair.sent.data}</span>
                    {pair.sent.responseTime !== undefined && (
                      <span className="hi-rt">{pair.sent.responseTime}ms</span>
                    )}
                  </div>
                  {pair.received && (
                    <div className={`history-item received ${pair.received.status}`}>
                      <span className="hi-meta">←</span>
                      <span className="hi-text mono">{pair.received.data}</span>
                    </div>
                  )}
                </div>
              ))}
            </div>
          </div>
        </div>

        <div className="modal-footer">
          <label className="checkbox">
            <input type="checkbox" checked={connectionAware} onChange={e => setConnectionAware(e.target.checked)} />
            Require connection to send
          </label>
          <div className="spacer" />
          <button className="btn" onClick={onClose}>Close</button>
        </div>
      </div>
    </div>
  )
}
