import { useState } from 'react'
import { HelpContent } from '../types'

interface HelpSystemProps {
  visible: boolean
  onClose: () => void
}

const helpContent: Record<string, HelpContent> = {
  overview: {
    title: 'Multi-Sonicator-IO Test Harness Overview',
    description: 'This web interface provides interactive control and monitoring of the ATmega32A sonicator multiplexer through an Arduino Test Wrapper. The system operates in full sandbox mode with continuous hardware monitoring.',
    examples: [
      'Click on pin connections in the diagram to interact with them',
      'Configurable parameters can be toggled HIGH/LOW to control ATmega32A inputs',
      'Live DUT monitoring shows real-time ATmega32A output states',
      'Analog pins display ADC readings from 0-1023 (0-5V)',
      'PWM pins are continuously monitored and display real-time duty cycles',
      'Frequency pins support both automatic and manual generation modes'
    ]
  },
  
  pinMapping: {
    title: 'Pin Mapping (SOLE SOURCE OF TRUTH: docs/planning/pin-matrix.md)',
    description: 'Pin connections between Arduino Test Wrapper and ATmega32A DUT for Sonicator 4 (S4) single channel HIL mapping.',
    examples: [
      'D7 → ATmega32A PB0 (FREQ_DIV10_4): Frequency ÷10 input',
      'D8 → ATmega32A PB4 (FREQ_LOCK_4): Frequency lock input',
      'A2 → ATmega32A PD3 (OVERLOAD_4): Overload detection input',
      'A3 → ATmega32A PC0 (START_4): Start control output',
      'A4 → ATmega32A PC1 (RESET_4): Reset control output',
      'A1 → ATmega32A PA7 (POWER_SENSE_4): Power sense analog input',
      'D9 ← ATmega32A PD7 (AMPLITUDE_ALL): Amplitude PWM output',
      'D10 → ATmega32A PD0 (UART_RXD): UART receive',
      'D11 → ATmega32A PD1 (UART_TXD): UART transmit',
      'D12 → ATmega32A PD2 (STATUS_LED): Status LED output'
    ]
  },

  pinConnections: {
    title: 'Complete Pin Connections Matrix',
    description: 'Comprehensive pin mapping for all four sonicator channels as defined in docs/planning/pin-matrix.md. This is the SOLE SOURCE OF TRUTH for hardware pin assignments.',
    examples: [
      '🔌 DB9 Connector Layout:',
      'DB9-1 (S1), DB9-2 (S2), DB9-3 (S3), DB9-4 (S4), DB9-0 (System)',
      '',
      '📍 Sonicator 1 (DB9-1):',
      'Pin 1: OVERLOAD_1 ← PD6 (Pin 20) - OL_1 via opto',
      'Pin 2: RESET_1 ← PC7 (Pin 29) - ULN2003A open-collector',
      'Pin 3: FREQ_LOCK_1 ← PB7 (Pin 8) - FLCK_1 via opto',
      'Pin 4: FREQ_DIV10_1 ← PB3 (Pin 4) - ÷10 frequency',
      'Pin 5: POWER_SENSE_1 ← PA4/A4 (Pin 36) - 5.44 mV/W scaling',
      'Pin 7: START_1 ← PC6 (Pin 28) - ULN2003A open-collector',
      '',
      '🔄 Continuous Monitoring:',
      'AMPLITUDE_ALL (D9): PWM duty cycle continuously measured',
      'FREQ_DIV10_4 (D7): Frequency generation with real-time display',
      'Pin 8: AMPLITUDE_ALL → PD7 (Pin 21) - Shared 0–10V output',
      '',
      '📍 Sonicator 2 (DB9-2):',
      'Pin 1: OVERLOAD_2 ← PD5 (Pin 19) - OL_2 via opto',
      'Pin 2: RESET_2 ← PC5 (Pin 27) - ULN2003A open-collector',
      'Pin 3: FREQ_LOCK_2 ← PB6 (Pin 7) - FLCK_2 via opto',
      'Pin 4: FREQ_DIV10_2 ← PB2 (Pin 3) - ÷10 frequency',
      'Pin 5: POWER_SENSE_2 ← PA5/A5 (Pin 35) - 5.44 mV/W scaling',
      'Pin 7: START_2 ← PC4 (Pin 26) - ULN2003A open-collector',
      'Pin 8: AMPLITUDE_ALL → PD7 (Pin 21) - Shared 0–10V output',
      '',
      '📍 Sonicator 3 (DB9-3):',
      'Pin 1: OVERLOAD_3 ← PD4 (Pin 18) - OL_3 via opto',
      'Pin 2: RESET_3 ← PC3 (Pin 25) - ULN2003A open-collector',
      'Pin 3: FREQ_LOCK_3 ← PB5 (Pin 6) - FLCK_3 via opto',
      'Pin 4: FREQ_DIV10_3 ← PB1 (Pin 2) - ÷10 frequency',
      'Pin 5: POWER_SENSE_3 ← PA6/A6 (Pin 34) - 5.44 mV/W scaling',
      'Pin 7: START_3 ← PC2 (Pin 24) - ULN2003A open-collector',
      'Pin 8: AMPLITUDE_ALL → PD7 (Pin 21) - Shared 0–10V output',
      '',
      '📍 Sonicator 4 (DB9-4) - HIL Test Harness:',
      'Pin 1: OVERLOAD_4 ← PD3 (Pin 17) - OL_4 via opto → A2',
      'Pin 2: RESET_4 ← PC1 (Pin 23) - ULN2003A open-collector → A4',
      'Pin 3: FREQ_LOCK_4 ← PB4 (Pin 5) - FLCK_4 via opto → D8',
      'Pin 4: FREQ_DIV10_4 ← PB0 (Pin 1) - ÷10 frequency → D7',
      'Pin 5: POWER_SENSE_4 ← PA7/A7 (Pin 33) - 5.44 mV/W scaling → A1',
      'Pin 7: START_4 ← PC0 (Pin 22) - ULN2003A open-collector → A3',
      'Pin 8: AMPLITUDE_ALL → PD7 (Pin 21) - Shared 0–10V output → D9 (PWM)',
      '',
      '📍 System Communications (DB9-0):',
      'Pin 8: UART_RXD ← PD0 (Pin 14) - MODBUS RTU RX → D2',
      'Pin 9: UART_TXD ← PD1 (Pin 15) - MODBUS RTU TX → D3',
      '',
      '📍 Status Indicator:',
      'STATUS_LED ← PD2 (Pin 16) - Status LED drive → D4',
      '',
      '⚠️ Important Notes:',
      'AMPLITUDE_ALL (PD7) is shared across all channels',
      'Only Sonicator 4 has Arduino wrapper pins for HIL testing',
      'All pin assignments verified by Product Owner',
      'Changes must be reflected in include/config.h'
    ]
  },

  inputPins: {
    title: 'Configurable Parameters',
    description: 'Configurable parameters (IN) send control signals from Arduino to ATmega32A. These can be toggled HIGH (5V) or LOW (0V).',
    examples: [
      'Click pin buttons to toggle between HIGH and LOW states',
      'Use "All LOW" / "All HIGH" buttons for bulk operations',
      'FREQ_DIV10_4: Controls frequency division input',
      'FREQ_LOCK_4: Controls frequency lock detection',
      'OVERLOAD_4: Simulates overload condition'
    ],
    relatedPins: ['FREQ_DIV10_4', 'FREQ_LOCK_4', 'OVERLOAD_4', 'UART_RXD']
  },

  outputPins: {
    title: 'Live DUT Monitoring',
    description: 'Live DUT monitoring (OUT) provides real-time read-only monitoring of all signals generated by the ATmega32A Device Under Test, including digital outputs and PWM signals.',
    examples: [
      'Digital outputs: START_4, RESET_4, UART_TXD, STATUS_LED',
      'PWM signals: AMPLITUDE_ALL (continuously monitored duty cycle)',
      'All signals update automatically in real-time',
      'PWM values display as percentages with live monitoring indicators',
      'Digital signals show HIGH/LOW states with timestamps',
      'No manual refresh needed - all values update continuously'
    ],
    relatedPins: ['START_4', 'RESET_4', 'UART_TXD', 'STATUS_LED', 'AMPLITUDE_ALL']
  },

  analogPins: {
    title: 'Analog Pin Monitoring',
    description: 'Analog pins (ANALOG) provide ADC readings from 0-1023 representing 0-5V.',
    examples: [
      'Click "Read ADC" to get current analog value',
      'Values shown as: ADC_VALUE (VOLTAGE)',
      'POWER_SENSE_4: Monitors sonicator power consumption',
      'Readings update automatically in sandbox mode'
    ],
    relatedPins: ['POWER_SENSE_4']
  },



  connection: {
    title: 'Hardware Connection Management',
    description: 'The system automatically detects and connects to the Arduino Test Harness.',
    examples: [
      'Green indicator: Connected and operational',
      'Red indicator: Disconnected or error',
      'Yellow indicator: Connecting/reconnecting',
      'Use "Reconnect" for quick retry',
      'Use "Retry (3x)" for multiple connection attempts'
    ]
  },

  troubleshooting: {
    title: 'Troubleshooting',
    description: 'Common issues and solutions for hardware testing.',
    examples: [
      'Connection failed: Check Arduino USB connection and port permissions',
      'Pin not responding: Verify wiring matches pin-matrix.md',
      'ADC readings unstable: Check power supply and ground connections',
      'PWM not updating: Check if ATmega32A is generating PWM signal on PD7',
      'PWM shows 0%: Verify AMPLITUDE_ALL pin connection and signal generation',
      'Frequency not generating: Check frequency configuration and enable status',
      'WebSocket errors: Refresh browser and check backend server'
    ]
  },

  shortcuts: {
    title: 'Interface Tips & Best Practices',
    description: 'Efficiency tips and best practices for using the test harness.',
    examples: [
      'Click pin connections in diagram for quick interaction',
      'PWM values update automatically - no manual refresh needed',
      'Monitor green pulsing indicators for active continuous monitoring',
      'Use frequency inline controls for immediate configuration changes',
      'Monitor timestamps to verify real-time updates',
      'Check connection status indicator in header before testing',
      'Use browser developer tools for advanced debugging'
    ]
  }
}

// Topic icons mapping
const topicIcons: Record<string, string> = {
  overview: '🏠',
  pinMapping: '🔌',
  pinConnections: '📋',
  inputPins: '⚙️',
  outputPins: '📡',
  analogPins: '📊',
  connection: '🔗',
  troubleshooting: '🔧',
  shortcuts: '⚡'
}

export default function HelpSystem({ visible, onClose }: HelpSystemProps) {
  const [selectedTopic, setSelectedTopic] = useState('overview')
  const [contentKey, setContentKey] = useState(0)

  if (!visible) return null

  const currentHelp = helpContent[selectedTopic]

  const handleTopicChange = (topic: string) => {
    setSelectedTopic(topic)
    setContentKey(prev => prev + 1) // Force re-render with animation
  }

  return (
    <div className="help-modal-overlay">
      <div className="help-modal-container">
        {/* Help Topics Sidebar */}
        <div className="help-sidebar">
          <div className="help-sidebar-header">
            <h3 className="help-sidebar-title">
              📚 Help Topics
            </h3>
          </div>

          <div className="help-topics-list">
            {Object.entries(helpContent).map(([key, content]) => (
              <button
                key={key}
                className={`help-topic-button ${selectedTopic === key ? 'active' : ''}`}
                onClick={() => handleTopicChange(key)}
              >
                <span className="help-topic-icon">
                  {topicIcons[key] || '📄'}
                </span>
                <span className="help-topic-text">
                  {content.title.split(':')[0]}
                </span>
                {selectedTopic === key && (
                  <span className="help-topic-indicator">▶</span>
                )}
              </button>
            ))}
          </div>
        </div>

        {/* Help Content */}
        <div key={contentKey} className="help-content">
          <div className="help-content-header">
            <div className="help-content-title">
              <span className="help-content-icon">
                {topicIcons[selectedTopic] || '📄'}
              </span>
              <h2 className="help-title">{currentHelp.title}</h2>
            </div>
            <button
              className="help-close-button"
              onClick={onClose}
              title="Close help"
            >
              <span className="help-close-icon">✕</span>
            </button>
          </div>

          <div className="help-description">
            <p>{currentHelp.description}</p>
          </div>

          {currentHelp.examples && (
            <div className="help-examples">
              <h4 className="help-examples-title">
                <span className="help-examples-icon">💡</span>
                Examples & Usage
              </h4>
              <div className="help-examples-list">
                {currentHelp.examples.map((example, index) => (
                  <div key={index} className="help-example-item">
                    <span className="help-example-bullet">▸</span>
                    <span className="help-example-text">{example}</span>
                  </div>
                ))}
              </div>
            </div>
          )}

          {currentHelp.relatedPins && (
            <div className="help-related-pins">
              <h4 className="help-related-pins-title">
                <span className="help-related-pins-icon">🔗</span>
                Related Pins
              </h4>
              <div className="help-related-pins-list">
                {currentHelp.relatedPins.map(pin => (
                  <span key={pin} className="help-pin-badge">
                    {pin}
                  </span>
                ))}
              </div>
            </div>
          )}

          {selectedTopic === 'overview' && (
            <div className="help-quick-start">
              <h4 className="help-quick-start-title">
                <span className="help-quick-start-icon">🚀</span>
                Quick Start Guide
              </h4>
              <div className="help-quick-start-steps">
                <div className="help-step">
                  <span className="help-step-number">1</span>
                  <span className="help-step-text">Verify hardware connection (green indicator in header)</span>
                </div>
                <div className="help-step">
                  <span className="help-step-number">2</span>
                  <span className="help-step-text">Click on pin connections in the diagram to interact</span>
                </div>
                <div className="help-step">
                  <span className="help-step-number">3</span>
                  <span className="help-step-text">Use control panel for advanced operations</span>
                </div>
                <div className="help-step">
                  <span className="help-step-number">4</span>
                  <span className="help-step-text">Monitor real-time pin states and history</span>
                </div>
                <div className="help-step">
                  <span className="help-step-number">5</span>
                  <span className="help-step-text">Use help tooltips throughout the interface</span>
                </div>
              </div>
            </div>
          )}

          {selectedTopic === 'pinConnections' && (
            <div className="help-special-sections">
              <div className="help-warning-card">
                <h4 className="help-warning-title">
                  <span className="help-warning-icon">⚠️</span>
                  SOLE SOURCE OF TRUTH
                </h4>
                <p className="help-warning-text">
                  This pin matrix is the authoritative reference for all hardware connections.
                  Any changes must be reflected in <code className="help-code">include/config.h</code> and
                  the HIL wrapper under <code className="help-code">test/acceptance/sketches/arduino_test_wrapper/</code>.
                </p>
              </div>

              <div className="help-success-card">
                <h4 className="help-success-title">
                  <span className="help-success-icon">🔧</span>
                  HIL Test Harness
                </h4>
                <p className="help-success-text">
                  Currently only Sonicator 4 (S4) is connected to the Arduino Test Wrapper for Hardware-in-the-Loop testing.
                  The web interface controls this single channel through the Arduino Uno R4 WiFi.
                </p>
              </div>

              <div className="help-info-card">
                <h4 className="help-info-title">
                  <span className="help-info-icon">📋</span>
                  Legend
                </h4>
                <div className="help-legend-list">
                  <div className="help-legend-item">
                    <strong>DUT:</strong> ATmega32A pin identifier (port/pin and header reference)
                  </div>
                  <div className="help-legend-item">
                    <strong>Signal:</strong> Functional name (OVERLOAD, FREQ_DIV10, etc.)
                  </div>
                  <div className="help-legend-item">
                    <strong>Dir:</strong> Direction from DUT perspective (IN/OUT/ANALOG)
                  </div>
                  <div className="help-legend-item">
                    <strong>Scale/Notes:</strong> Electrical characteristics and scaling
                  </div>
                  <div className="help-legend-item">
                    <strong>→ Pin:</strong> Arduino wrapper pin for HIL testing
                  </div>
                </div>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
