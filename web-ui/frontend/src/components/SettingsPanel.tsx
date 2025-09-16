import React, { useState, useEffect } from 'react'

interface UserPreferences {
  animationsEnabled: boolean
  soundEnabled: boolean
  autoRefreshInterval: number
  defaultTimeRange: '1m' | '5m' | '15m' | '1h'
  compactMode: boolean
  showTooltips: boolean
  keyboardShortcuts: boolean
  exportFormat: 'json' | 'csv'
  maxHistoryEntries: number
}

interface SettingsPanelProps {
  onPreferencesChange?: (preferences: UserPreferences) => void
}

const DEFAULT_PREFERENCES: UserPreferences = {
  animationsEnabled: true,
  soundEnabled: false,
  autoRefreshInterval: 1000,
  defaultTimeRange: '5m',
  compactMode: false,
  showTooltips: true,
  keyboardShortcuts: true,
  exportFormat: 'csv',
  maxHistoryEntries: 1000
}

const SettingsPanel: React.FC<SettingsPanelProps> = ({ onPreferencesChange }) => {
  const [preferences, setPreferences] = useState<UserPreferences>(DEFAULT_PREFERENCES)
  const [hasUnsavedChanges, setHasUnsavedChanges] = useState(false)
  const [activeSection, setActiveSection] = useState<'appearance' | 'behavior' | 'data' | 'shortcuts'>('appearance')

  // Load preferences from localStorage on mount
  useEffect(() => {
    const savedPreferences = localStorage.getItem('multi-sonicator-preferences')
    if (savedPreferences) {
      try {
        const parsed = JSON.parse(savedPreferences)
        setPreferences({ ...DEFAULT_PREFERENCES, ...parsed })
      } catch (error) {
        console.error('Failed to parse saved preferences:', error)
      }
    }
  }, [])

  // Apply preference changes immediately
  useEffect(() => {
    const root = document.documentElement

    // Apply animations setting
    if (!preferences.animationsEnabled) {
      root.classList.add('no-animations')
    } else {
      root.classList.remove('no-animations')
    }

    // Apply compact mode
    if (preferences.compactMode) {
      root.classList.add('compact-mode')
    } else {
      root.classList.remove('compact-mode')
    }
  }, [preferences.animationsEnabled, preferences.compactMode])

  const updatePreference = <K extends keyof UserPreferences>(
    key: K,
    value: UserPreferences[K]
  ) => {
    const newPreferences = { ...preferences, [key]: value }
    setPreferences(newPreferences)
    setHasUnsavedChanges(true)
    // Persist immediately so state is preserved when navigating away
    try {
      localStorage.setItem('multi-sonicator-preferences', JSON.stringify(newPreferences))
    } catch {}
    onPreferencesChange?.(newPreferences)
  }

  const savePreferences = () => {
    localStorage.setItem('multi-sonicator-preferences', JSON.stringify(preferences))
    setHasUnsavedChanges(false)
  }

  const resetPreferences = () => {
    setPreferences(DEFAULT_PREFERENCES)
    setHasUnsavedChanges(true)
  }

  const exportPreferences = () => {
    const dataStr = JSON.stringify(preferences, null, 2)
    const blob = new Blob([dataStr], { type: 'application/json' })
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a')
    a.href = url
    a.download = 'multi-sonicator-preferences.json'
    document.body.appendChild(a)
    a.click()
    document.body.removeChild(a)
    URL.revokeObjectURL(url)
  }

  const importPreferences = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0]
    if (!file) return

    const reader = new FileReader()
    reader.onload = (e) => {
      try {
        const imported = JSON.parse(e.target?.result as string)
        setPreferences({ ...DEFAULT_PREFERENCES, ...imported })
        setHasUnsavedChanges(true)
      } catch (error) {
        alert('Failed to import preferences: Invalid JSON file')
      }
    }
    reader.readAsText(file)
  }

  const keyboardShortcuts = [
    { key: 'Ctrl + H', action: 'Toggle Hardware tab' },
    { key: 'Ctrl + T', action: 'Toggle Testing tab' },
    { key: 'Ctrl + S', action: 'Toggle Settings tab' },
    { key: 'Space', action: 'Ping hardware' },
    { key: 'Ctrl + E', action: 'Export data' },
    { key: 'Ctrl + R', action: 'Refresh connection' },
    { key: 'Esc', action: 'Close modals' }
  ]

  return (
    <div className="settings-panel">
      <div className="settings-header">
        <h3>⚙️ Settings & Preferences</h3>
        <div className="settings-actions">
          {hasUnsavedChanges && (
            <button className="save-btn" onClick={savePreferences}>
              💾 Save Changes
            </button>
          )}
          <button className="export-btn" onClick={exportPreferences}>
            📤 Export
          </button>
          <label className="import-btn">
            📥 Import
            <input 
              type="file" 
              accept=".json" 
              onChange={importPreferences}
              style={{ display: 'none' }}
            />
          </label>
          <button className="reset-btn" onClick={resetPreferences}>
            🔄 Reset
          </button>
        </div>
      </div>

      <div className="settings-content">
        {/* Section Navigation */}
        <div className="settings-nav">
          {[
            { id: 'appearance', label: 'Appearance', icon: '🎨' },
            { id: 'behavior', label: 'Behavior', icon: '⚡' },
            { id: 'data', label: 'Data', icon: '📊' },
            { id: 'shortcuts', label: 'Shortcuts', icon: '⌨️' }
          ].map(section => (
            <button
              key={section.id}
              className={`nav-item ${activeSection === section.id ? 'active' : ''}`}
              onClick={() => setActiveSection(section.id as any)}
            >
              {section.icon} {section.label}
            </button>
          ))}
        </div>

        {/* Settings Sections */}
        <div className="settings-sections">
          {activeSection === 'appearance' && (
            <div className="settings-section">
              <h4>Appearance Settings</h4>
              
              <div className="setting-note">
                <p>💡 <strong>Theme:</strong> Use the theme toggle button (🌓) in the header to switch between light, dark, and auto modes.</p>
              </div>

              <div className="setting-group">
                <label>
                  <input 
                    type="checkbox"
                    checked={preferences.animationsEnabled}
                    onChange={(e) => updatePreference('animationsEnabled', e.target.checked)}
                  />
                  Enable Animations
                </label>
              </div>

              <div className="setting-group">
                <label>
                  <input 
                    type="checkbox"
                    checked={preferences.compactMode}
                    onChange={(e) => updatePreference('compactMode', e.target.checked)}
                  />
                  Compact Mode
                </label>
              </div>

              <div className="setting-group">
                <label>
                  <input 
                    type="checkbox"
                    checked={preferences.showTooltips}
                    onChange={(e) => updatePreference('showTooltips', e.target.checked)}
                  />
                  Show Tooltips
                </label>
              </div>
            </div>
          )}

          {activeSection === 'behavior' && (
            <div className="settings-section">
              <h4>Behavior Settings</h4>
              
              <div className="setting-group">
                <label>Auto Refresh Interval (ms)</label>
                <input 
                  type="number"
                  min="100"
                  max="10000"
                  step="100"
                  value={preferences.autoRefreshInterval}
                  onChange={(e) => updatePreference('autoRefreshInterval', parseInt(e.target.value))}
                />
              </div>

              <div className="setting-group">
                <label>Default Time Range</label>
                <select 
                  value={preferences.defaultTimeRange}
                  onChange={(e) => updatePreference('defaultTimeRange', e.target.value as any)}
                >
                  <option value="1m">1 minute</option>
                  <option value="5m">5 minutes</option>
                  <option value="15m">15 minutes</option>
                  <option value="1h">1 hour</option>
                </select>
              </div>

              <div className="setting-group">
                <label>
                  <input 
                    type="checkbox"
                    checked={preferences.soundEnabled}
                    onChange={(e) => updatePreference('soundEnabled', e.target.checked)}
                  />
                  Enable Sound Notifications
                </label>
              </div>

              <div className="setting-group">
                <label>
                  <input 
                    type="checkbox"
                    checked={preferences.keyboardShortcuts}
                    onChange={(e) => updatePreference('keyboardShortcuts', e.target.checked)}
                  />
                  Enable Keyboard Shortcuts
                </label>
              </div>
            </div>
          )}

          {activeSection === 'data' && (
            <div className="settings-section">
              <h4>Data Settings</h4>
              
              <div className="setting-group">
                <label>Default Export Format</label>
                <select 
                  value={preferences.exportFormat}
                  onChange={(e) => updatePreference('exportFormat', e.target.value as any)}
                >
                  <option value="csv">CSV</option>
                  <option value="json">JSON</option>
                </select>
              </div>

              <div className="setting-group">
                <label>Max History Entries</label>
                <input 
                  type="number"
                  min="100"
                  max="10000"
                  step="100"
                  value={preferences.maxHistoryEntries}
                  onChange={(e) => updatePreference('maxHistoryEntries', parseInt(e.target.value))}
                />
                <small>Higher values use more memory but provide more historical data</small>
              </div>
            </div>
          )}

          {activeSection === 'shortcuts' && (
            <div className="settings-section">
              <h4>Keyboard Shortcuts</h4>
              <div className="shortcuts-list">
                {keyboardShortcuts.map((shortcut, index) => (
                  <div key={index} className="shortcut-item">
                    <kbd className="shortcut-key">{shortcut.key}</kbd>
                    <span className="shortcut-action">{shortcut.action}</span>
                  </div>
                ))}
              </div>
              <div className="shortcut-note">
                <small>
                  💡 Keyboard shortcuts can be disabled in the Behavior settings.
                  Some shortcuts may not work in all browsers or operating systems.
                </small>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}

export default SettingsPanel
