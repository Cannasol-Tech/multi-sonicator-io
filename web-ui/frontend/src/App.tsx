import { useState, useEffect } from 'react'
import HardwareDiagram from './components/HardwareDiagram'
import ControlPanel from './components/ControlPanel'
import Header from './components/Header'
import HelpSystem from './components/HelpSystem'
import { useWebSocket } from './hooks/useWebSocket'
import { useHardwareState } from './hooks/useHardwareState'
import { usePinHistory } from './hooks/usePinHistory'

function App() {
  const [helpVisible, setHelpVisible] = useState(false)
  const { connected, sendMessage, lastMessage } = useWebSocket('ws://localhost:3001/ws')
  const { hardwareState, updatePinState, updateMultiplePins, setConnectionStatus } = useHardwareState()
  const { addHistoryEntry } = usePinHistory()

  // Handle WebSocket messages
  useEffect(() => {
    if (!lastMessage) return

    switch (lastMessage.type) {
      case 'initial_state' as any:
        // Update all pin states from initial state
        if (lastMessage.data.pins) {
          updateMultiplePins(lastMessage.data.pins)
        }
        setConnectionStatus({
          connected: lastMessage.data.connected,
          port: lastMessage.data.port,
          lastSeen: Date.now()
        })
        break

      case 'pin_update':
        // Update individual pin state and add to history
        const { signal, pinState } = lastMessage.data
        updatePinState(signal, pinState)
        addHistoryEntry(signal, pinState)
        break

      case 'connection_status':
        setConnectionStatus({
          connected: lastMessage.data.connected,
          port: lastMessage.data.port,
          lastSeen: Date.now(),
          error: lastMessage.data.connected ? undefined : 'Hardware disconnected'
        })
        break

      case 'error':
        console.error('Hardware error:', lastMessage.data.error)
        break
    }
  }, [lastMessage, updatePinState, updateMultiplePins, setConnectionStatus])

  // Update connection status when WebSocket connection changes
  useEffect(() => {
    setConnectionStatus({
      connected,
      lastSeen: connected ? Date.now() : hardwareState.connection.lastSeen,
      error: connected ? undefined : 'WebSocket disconnected'
    })
  }, [connected, setConnectionStatus, hardwareState.connection.lastSeen])

  const handlePinControl = (pin: string, action: string, value?: any) => {
    const command = {
      type: 'hardware_command',
      data: {
        command: action,
        pin,
        value,
        timestamp: Date.now()
      }
    }
    sendMessage(command)
  }

  return (
    <div className="app">
      <Header
        connectionStatus={hardwareState.connection}
        onReconnect={() => window.location.reload()}
        onShowHelp={() => setHelpVisible(true)}
      />
      
      <main className="app-main">
        <div className="app-content">
          <HardwareDiagram 
            hardwareState={hardwareState}
            onPinClick={handlePinControl}
          />
        </div>
        
        <ControlPanel
          hardwareState={hardwareState}
          onPinControl={handlePinControl}
          connected={connected}
        />
      </main>

      <HelpSystem
        visible={helpVisible}
        onClose={() => setHelpVisible(false)}
      />
    </div>
  )
}

export default App
