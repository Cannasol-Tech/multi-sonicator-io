import { useState, useEffect } from 'react'
import HardwareDiagram from './components/HardwareDiagram'
import ControlPanel from './components/ControlPanel'
import TestAutomationPanel from './components/TestAutomationPanel'
import ArduinoCommandsPanel from './components/ArduinoCommandsPanel'
import TestExecutionIndicator from './components/TestExecutionIndicator'
import Header from './components/Header'
import HelpSystem from './components/HelpSystem'
import SettingsPanel from './components/SettingsPanel'
import ProgressBarDemo from './components/ProgressBarDemo'


import { useWebSocket } from './hooks/useWebSocket'
import { useHardwareState } from './hooks/useHardwareState'
import { usePinHistory } from './hooks/usePinHistory'
import { useTestAutomation } from './hooks/useTestAutomation'
import { useKeyboardShortcuts, createAppShortcuts } from './hooks/useKeyboardShortcuts'


function App() {
  const [helpVisible, setHelpVisible] = useState(false)
  const [highlightedPins, setHighlightedPins] = useState<string[]>([])
  const [activeTab, setActiveTab] = useState<'hardware' | 'testing' | 'arduino-commands' | 'settings'>('hardware')
  const [currentTestExecution, setCurrentTestExecution] = useState<any>(null)
  const [keyboardShortcutsEnabled, setKeyboardShortcutsEnabled] = useState(true)

  const { connected, sendMessage, lastMessage } = useWebSocket('ws://localhost:3001/ws')
  const { hardwareState, updatePinState, updateMultiplePins, setConnectionStatus } = useHardwareState()
  const { addHistoryEntry } = usePinHistory()
  const {
    updateExecutionProgress,
    handleExecutionComplete,
    handleExecutionError
  } = useTestAutomation()


  // Keyboard shortcuts
  const shortcuts = createAppShortcuts({
    toggleHardwareTab: () => setActiveTab('hardware'),
    toggleTestingTab: () => setActiveTab('testing'),
    toggleAnalyticsTab: () => {}, // Disabled - Analytics removed
    toggleSettingsTab: () => setActiveTab('settings'),
    pingHardware: () => {
      // Trigger ping command
      fetch('/api/ping', { method: 'POST' })
        .then(res => res.json())
        .then(data => console.log('Ping result:', data))
        .catch(err => console.error('Ping failed:', err))
    },
    exportData: () => {
      // Export current data
      console.log('Export data shortcut triggered')
    },
    refreshConnection: () => {
      // Refresh WebSocket connection
      window.location.reload()
    },
    closeModals: () => {
      setHelpVisible(false)
      setCurrentTestExecution(null)
    }
  })

  useKeyboardShortcuts({
    shortcuts,
    enabled: keyboardShortcutsEnabled
  })

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

      case 'test_progress':
        // Update test execution progress
        updateExecutionProgress(lastMessage.data)
        setCurrentTestExecution(lastMessage.data)
        break

      case 'test_complete':
        // Handle test execution completion
        handleExecutionComplete(lastMessage.data)
        setCurrentTestExecution(lastMessage.data)
        break

      case 'test_error':
        // Handle test execution error
        handleExecutionError(lastMessage.data.error)
        break

      case 'test_stopped':
        // Handle test execution stopped
        handleExecutionComplete(lastMessage.data)
        setCurrentTestExecution(lastMessage.data)
        break


    }
  }, [lastMessage, updatePinState, updateMultiplePins, setConnectionStatus, updateExecutionProgress, handleExecutionComplete, handleExecutionError])

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
            highlightedPins={highlightedPins}
            onPinHighlight={setHighlightedPins}
          />
        </div>

        <div className="control-panel">
          {/* Tab Navigation */}
          <div className="tab-navigation">
            <button
              className={`tab-button ${activeTab === 'hardware' ? 'active' : ''}`}
              onClick={() => setActiveTab('hardware')}
            >
              🔧 Hardware Control
            </button>
            <button
              className={`tab-button ${activeTab === 'testing' ? 'active' : ''}`}
              onClick={() => setActiveTab('testing')}
            >
              🧪 Test Automation
            </button>
            <button
              className={`tab-button ${activeTab === 'arduino-commands' ? 'active' : ''}`}
              onClick={() => setActiveTab('arduino-commands')}
            >
              🔧 Arduino Commands
            </button>
            <button
              className={`tab-button ${activeTab === 'settings' ? 'active' : ''}`}
              onClick={() => setActiveTab('settings')}
            >
              ⚙️ Settings
            </button>
          </div>

          {/* Tab Content */}
          <div className="tab-content">
            {activeTab === 'hardware' && (
              <div className="hardware-tab">
                <div className="control-panels">
                  <ControlPanel
                    hardwareState={hardwareState}
                    onPinControl={handlePinControl}
                    connected={connected}
                  />
                </div>
              </div>
            )}

            {activeTab === 'testing' && (
              <TestAutomationPanel
                onPinHighlight={setHighlightedPins}
                onTestProgress={(execution) => {
                  console.log('Test progress:', execution)
                }}
              />
            )}

            {activeTab === 'arduino-commands' && (
              <ArduinoCommandsPanel
                hardwareState={hardwareState}
                connected={connected}
              />
            )}

            {activeTab === 'settings' && (
              <div className="settings-tab">
                <SettingsPanel
                  onPreferencesChange={(preferences) => {
                    console.log('Preferences updated:', preferences)
                    setKeyboardShortcutsEnabled(preferences.keyboardShortcuts)
                  }}
                />
                <ProgressBarDemo />
              </div>
            )}
          </div>
        </div>
      </main>

      <HelpSystem
        visible={helpVisible}
        onClose={() => setHelpVisible(false)}
      />

      <TestExecutionIndicator
        execution={currentTestExecution}
        visible={currentTestExecution !== null}
      />
    </div>
  )
}

export default App
