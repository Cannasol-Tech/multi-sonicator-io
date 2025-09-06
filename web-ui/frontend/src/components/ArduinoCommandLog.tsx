import React, { useState, useEffect, useRef } from 'react'

interface ArduinoCommand {
  direction: 'sent' | 'received'
  data: string
  timestamp: number
  type: string
}

interface ArduinoCommandLogProps {
  commands: ArduinoCommand[]
  maxEntries?: number
}

const ArduinoCommandLog: React.FC<ArduinoCommandLogProps> = ({ 
  commands, 
  maxEntries = 100 
}) => {
  const [isAutoScroll, setIsAutoScroll] = useState(true)
  const [filter, setFilter] = useState<'all' | 'sent' | 'received'>('all')
  const logContainerRef = useRef<HTMLDivElement>(null)

  // Auto-scroll to bottom when new commands arrive
  useEffect(() => {
    if (isAutoScroll && logContainerRef.current) {
      logContainerRef.current.scrollTop = logContainerRef.current.scrollHeight
    }
  }, [commands, isAutoScroll])

  // Handle manual scroll to detect if user scrolled up
  const handleScroll = () => {
    if (logContainerRef.current) {
      const { scrollTop, scrollHeight, clientHeight } = logContainerRef.current
      const isAtBottom = scrollTop + clientHeight >= scrollHeight - 10
      setIsAutoScroll(isAtBottom)
    }
  }

  const filteredCommands = commands
    .filter(cmd => filter === 'all' || cmd.direction === filter)
    .slice(-maxEntries)

  const formatTimestamp = (timestamp: number) => {
    const date = new Date(timestamp)
    return date.toLocaleTimeString('en-US', { 
      hour12: false, 
      hour: '2-digit', 
      minute: '2-digit', 
      second: '2-digit',
      fractionalSecondDigits: 3
    })
  }

  const formatData = (data: string, direction: string) => {
    try {
      // Try to parse as JSON for better formatting
      const parsed = JSON.parse(data)
      return JSON.stringify(parsed, null, 2)
    } catch {
      // If not JSON, return as-is
      return data
    }
  }

  const clearLog = () => {
    // This would need to be implemented by the parent component
    // For now, we'll just show a message
    console.log('Clear log requested')
  }

  return (
    <div className="arduino-command-log">
      <div className="log-header">
        <h3>Arduino Command Log</h3>
        <div className="log-controls">
          <select 
            value={filter} 
            onChange={(e) => setFilter(e.target.value as 'all' | 'sent' | 'received')}
            className="filter-select"
          >
            <option value="all">All Commands</option>
            <option value="sent">Sent Only</option>
            <option value="received">Received Only</option>
          </select>
          
          <button 
            onClick={() => setIsAutoScroll(!isAutoScroll)}
            className={`auto-scroll-btn ${isAutoScroll ? 'active' : ''}`}
            title={isAutoScroll ? 'Disable auto-scroll' : 'Enable auto-scroll'}
          >
            {isAutoScroll ? '📌' : '📌'}
          </button>
          
          <button 
            onClick={clearLog}
            className="clear-btn"
            title="Clear log"
          >
            🗑️
          </button>
        </div>
      </div>

      <div 
        ref={logContainerRef}
        className="log-container"
        onScroll={handleScroll}
      >
        {filteredCommands.length === 0 ? (
          <div className="no-commands">
            No commands to display. Commands will appear here when the Arduino communicates.
          </div>
        ) : (
          filteredCommands.map((command, index) => (
            <div 
              key={`${command.timestamp}-${index}`}
              className={`log-entry ${command.direction}`}
            >
              <div className="log-entry-header">
                <span className="timestamp">{formatTimestamp(command.timestamp)}</span>
                <span className={`direction ${command.direction}`}>
                  {command.direction === 'sent' ? '→ SENT' : '← RECEIVED'}
                </span>
                <span className="type">{command.type}</span>
              </div>
              <div className="log-entry-data">
                <pre>{formatData(command.data, command.direction)}</pre>
              </div>
            </div>
          ))
        )}
      </div>

      <div className="log-footer">
        <span className="entry-count">
          {filteredCommands.length} / {maxEntries} entries
        </span>
        {!isAutoScroll && (
          <button 
            onClick={() => {
              setIsAutoScroll(true)
              if (logContainerRef.current) {
                logContainerRef.current.scrollTop = logContainerRef.current.scrollHeight
              }
            }}
            className="scroll-to-bottom"
          >
            ↓ Scroll to bottom
          </button>
        )}
      </div>

      <style jsx>{`
        .arduino-command-log {
          display: flex;
          flex-direction: column;
          height: 100%;
          background: #1a1a1a;
          border: 1px solid #333;
          border-radius: 8px;
          overflow: hidden;
        }

        .log-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 12px 16px;
          background: #2a2a2a;
          border-bottom: 1px solid #333;
        }

        .log-header h3 {
          margin: 0;
          color: #fff;
          font-size: 16px;
          font-weight: 600;
        }

        .log-controls {
          display: flex;
          gap: 8px;
          align-items: center;
        }

        .filter-select {
          background: #333;
          color: #fff;
          border: 1px solid #555;
          border-radius: 4px;
          padding: 4px 8px;
          font-size: 12px;
        }

        .auto-scroll-btn, .clear-btn {
          background: #333;
          color: #fff;
          border: 1px solid #555;
          border-radius: 4px;
          padding: 4px 8px;
          cursor: pointer;
          font-size: 12px;
        }

        .auto-scroll-btn.active {
          background: #0066cc;
          border-color: #0066cc;
        }

        .auto-scroll-btn:hover, .clear-btn:hover {
          background: #444;
        }

        .log-container {
          flex: 1;
          overflow-y: auto;
          padding: 8px;
          font-family: 'Courier New', monospace;
          font-size: 12px;
          line-height: 1.4;
        }

        .no-commands {
          color: #888;
          text-align: center;
          padding: 40px 20px;
          font-style: italic;
        }

        .log-entry {
          margin-bottom: 12px;
          border: 1px solid #333;
          border-radius: 4px;
          overflow: hidden;
        }

        .log-entry.sent {
          border-left: 3px solid #ff6b35;
        }

        .log-entry.received {
          border-left: 3px solid #4caf50;
        }

        .log-entry-header {
          display: flex;
          gap: 12px;
          align-items: center;
          padding: 6px 12px;
          background: #2a2a2a;
          border-bottom: 1px solid #333;
        }

        .timestamp {
          color: #888;
          font-size: 11px;
        }

        .direction {
          font-weight: bold;
          font-size: 11px;
        }

        .direction.sent {
          color: #ff6b35;
        }

        .direction.received {
          color: #4caf50;
        }

        .type {
          color: #66b3ff;
          font-size: 11px;
          background: #1a1a1a;
          padding: 2px 6px;
          border-radius: 3px;
        }

        .log-entry-data {
          padding: 8px 12px;
          background: #1a1a1a;
        }

        .log-entry-data pre {
          margin: 0;
          color: #e0e0e0;
          white-space: pre-wrap;
          word-break: break-all;
        }

        .log-footer {
          display: flex;
          justify-content: space-between;
          align-items: center;
          padding: 8px 16px;
          background: #2a2a2a;
          border-top: 1px solid #333;
          font-size: 12px;
        }

        .entry-count {
          color: #888;
        }

        .scroll-to-bottom {
          background: #0066cc;
          color: #fff;
          border: none;
          border-radius: 4px;
          padding: 4px 8px;
          cursor: pointer;
          font-size: 11px;
        }

        .scroll-to-bottom:hover {
          background: #0052a3;
        }
      `}</style>
    </div>
  )
}

export default ArduinoCommandLog
