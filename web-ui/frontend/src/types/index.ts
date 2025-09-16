/**
 * @fileoverview Multi-Sonicator-IO Web UI Type Definitions
 * 
 * This file contains all TypeScript type definitions for the Multi-Sonicator-IO Web UI.
 * All pin mappings and hardware configurations are based on docs/planning/pin-matrix.md
 * which serves as the SOLE SOURCE OF TRUTH for hardware interface specifications.
 * 
 * @author Stephen Boyett
 * @version 1.0.0
 * @since 2024-01-01
 */

/**
 * Represents the state of a hardware pin at a specific point in time.
 * 
 * @interface PinState
 * @property {string} pin - Physical pin identifier (e.g., 'D7', 'A2')
 * @property {string} signal - Signal name as defined in pin-matrix.md
 * @property {'IN' | 'OUT' | 'ANALOG'} direction - Pin direction/mode
 * @property {'HIGH' | 'LOW' | number | string} state - Current pin state value
 * @property {number} timestamp - Unix timestamp when state was last updated
 * @property {string} [description] - Optional human-readable description
 * @property {number} [frequency] - Frequency value in Hz for frequency pins
 * @property {string} [frequencyDisplay] - Formatted frequency display string
 * @property {string} [operatingFrequency] - Operating frequency display string
 * @property {boolean} [enabled] - Whether the pin/signal is enabled
 * @property {boolean} [isActive] - Whether the pin is currently active
 */
export interface PinState {
  pin: string;
  signal: string;
  direction: 'IN' | 'OUT' | 'ANALOG';
  state: 'HIGH' | 'LOW' | number | string; // number for analog values, string for frequency displays
  timestamp: number;
  description?: string;
  frequency?: number;  // For frequency pins
  frequencyDisplay?: string;  // For frequency pins
  operatingFrequency?: string;  // For frequency pins
  enabled?: boolean;  // For frequency pins
  isActive?: boolean;  // For frequency pins
}

/**
 * Represents the connection status between the web UI and hardware interface.
 * 
 * @interface ConnectionStatus
 * @property {boolean} connected - Whether connection is established
 * @property {string} [port] - Serial port identifier (e.g., '/dev/ttyUSB0')
 * @property {number} [lastSeen] - Unix timestamp of last successful communication
 * @property {string} [error] - Error message if connection failed
 */
export interface ConnectionStatus {
  connected: boolean;
  port?: string;
  lastSeen?: number;
  error?: string;
}

/**
 * Complete hardware state including connection status and all pin states.
 * 
 * @interface HardwareState
 * @property {ConnectionStatus} connection - Current connection status
 * @property {Record<string, PinState>} pins - Map of signal names to pin states
 * @property {number} lastUpdate - Unix timestamp of last state update
 */
export interface HardwareState {
  connection: ConnectionStatus;
  pins: Record<string, PinState>;
  lastUpdate: number;
}

/**
 * Arduino Uno Test Wrapper pin configuration.
 * Maps Arduino pins to their corresponding ATmega32A DUT pins.
 * Based on pin-matrix.md hardware interface specification.
 * 
 * @interface ArduinoTestWrapper
 * @property {Object} pins - Pin mapping configuration for Sonicator #4 and system pins
 */
export interface ArduinoTestWrapper {
  // Arduino Uno pins connected to ATmega32A DUT
  pins: {
    // Sonicator 4 (S4) - Single channel HIL mapping
    FREQ_DIV10_4: 'D7';    // -> ATmega32A PB0 Pin 1
    FREQ_LOCK_4: 'D8';     // -> ATmega32A PB4 Pin 5  
    OVERLOAD_4: 'A2';      // -> ATmega32A PD3 Pin 17
    START_4: 'A3';         // -> ATmega32A PC0 Pin 22
    RESET_4: 'A4';         // -> ATmega32A PC1 Pin 23
    POWER_SENSE_4: 'A1';   // -> ATmega32A PA7 Pin 33
    AMPLITUDE_ALL: 'D9';   // -> ATmega32A PD7 Pin 21
    
    // System communication
    UART_TXD: 'D11';       // -> ATmega32A PD1 Pin 15
    STATUS_LED: 'D12';     // -> ATmega32A PD2 Pin 16
  };
}

/**
 * ATmega32A Device Under Test (DUT) pin configuration.
 * Defines all ATmega32A pins organized by port with their physical pin numbers,
 * signal assignments, and directions. Supports multi-sonicator configuration.
 * 
 * @interface ATmega32ADUT
 * @property {Object} pins - Complete pin mapping for all ATmega32A ports (A, B, C, D)
 */
export interface ATmega32ADUT {
  // ATmega32A pins as Device Under Test
  pins: {
    // Port B
    PB0: { pin: 1, signal: 'FREQ_DIV10_4', direction: 'IN' };
    PB1: { pin: 2, signal: 'FREQ_DIV10_3', direction: 'IN' };
    PB2: { pin: 3, signal: 'FREQ_DIV10_2', direction: 'IN' };
    PB3: { pin: 4, signal: 'FREQ_DIV10_1', direction: 'IN' };
    PB4: { pin: 5, signal: 'FREQ_LOCK_4', direction: 'IN' };
    PB5: { pin: 6, signal: 'FREQ_LOCK_3', direction: 'IN' };
    PB6: { pin: 7, signal: 'FREQ_LOCK_2', direction: 'IN' };
    PB7: { pin: 8, signal: 'FREQ_LOCK_1', direction: 'IN' };
    
    // Port C  
    PC0: { pin: 22, signal: 'START_4', direction: 'OUT' };
    PC1: { pin: 23, signal: 'RESET_4', direction: 'OUT' };
    PC2: { pin: 24, signal: 'START_3', direction: 'OUT' };
    PC3: { pin: 25, signal: 'START_2', direction: 'OUT' };
    PC4: { pin: 26, signal: 'START_1', direction: 'OUT' };
    PC5: { pin: 27, signal: 'RESET_3', direction: 'OUT' };
    PC6: { pin: 28, signal: 'RESET_2', direction: 'OUT' };
    PC7: { pin: 29, signal: 'RESET_1', direction: 'OUT' };
    
    // Port D
    PD0: { pin: 14, signal: 'UART_RXD', direction: 'IN' };
    PD1: { pin: 15, signal: 'UART_TXD', direction: 'OUT' };
    PD2: { pin: 16, signal: 'STATUS_LED', direction: 'OUT' };
    PD3: { pin: 17, signal: 'OVERLOAD_4', direction: 'IN' };
    PD4: { pin: 18, signal: 'OVERLOAD_3', direction: 'IN' };
    PD5: { pin: 19, signal: 'OVERLOAD_2', direction: 'IN' };
    PD6: { pin: 20, signal: 'OVERLOAD_1', direction: 'IN' };
    PD7: { pin: 21, signal: 'AMPLITUDE_ALL', direction: 'OUT' };
    
    // Port A (Analog)
    PA7: { pin: 33, signal: 'POWER_SENSE_4', direction: 'ANALOG' };
    PA6: { pin: 34, signal: 'POWER_SENSE_3', direction: 'ANALOG' };
    PA5: { pin: 35, signal: 'POWER_SENSE_2', direction: 'ANALOG' };
    PA4: { pin: 36, signal: 'POWER_SENSE_1', direction: 'ANALOG' };
  };
}

/**
 * WebSocket message structure for real-time communication between frontend and backend.
 * 
 * @interface WebSocketMessage
 * @property {string} type - Message type identifier
 * @property {any} data - Message payload data
 * @property {number} timestamp - Unix timestamp when message was created
 */
export interface WebSocketMessage {
  type: 'pin_update' | 'connection_status' | 'error' | 'command_response' | 'test_progress' | 'test_complete' | 'test_error' | 'test_stopped' | 'initial_state' | 'arduino_command_response' | 'arduino_command_sent';
  data: any;
  timestamp: number;
}

/**
 * Hardware command structure for controlling pins and hardware interface.
 * 
 * @interface HardwareCommand
 * @property {string} command - Command identifier/name
 * @property {string[]} [args] - Optional command arguments
 * @property {boolean} [expectResponse] - Whether command expects a response
 */
export interface HardwareCommand {
  command: string;
  args?: string[];
  expectResponse?: boolean;
}

/**
 * Hardware command response structure.
 * 
 * @interface HardwareResponse
 * @property {boolean} success - Whether command executed successfully
 * @property {any} [data] - Response data payload
 * @property {string} [error] - Error message if command failed
 * @property {number} timestamp - Unix timestamp of response
 */
export interface HardwareResponse {
  success: boolean;
  data?: any;
  error?: string;
  timestamp: number;
}

/**
 * Help content structure for UI help system.
 * 
 * @interface HelpContent
 * @property {string} title - Help topic title
 * @property {string} description - Detailed description
 * @property {string[]} [examples] - Usage examples
 * @property {string[]} [relatedPins] - Related pin/signal names
 */
export interface HelpContent {
  title: string;
  description: string;
  examples?: string[];
  relatedPins?: string[];
}

/**
 * Test sequence definition for automated testing.
 * 
 * @interface TestSequence
 * @property {string} name - Test sequence name
 * @property {string} description - Test sequence description
 * @property {HardwareCommand[]} steps - Sequence of hardware commands
 * @property {any[]} [expectedResults] - Expected results for each step
 */
export interface TestSequence {
  name: string;
  description: string;
  steps: HardwareCommand[];
  expectedResults?: any[];
}

/**
 * Individual test step within a test scenario.
 * 
 * @interface TestStep
 * @property {string} step_type - Type of test step
 * @property {string} description - Human-readable step description
 * @property {'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error'} status - Current step status
 * @property {number} [duration_ms] - Step execution duration in milliseconds
 * @property {string} [error_message] - Error message if step failed
 * @property {string[]} pin_interactions - List of pins/signals involved in this step
 */
export interface TestStep {
  step_type: string;
  description: string;
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
  duration_ms?: number;
  error_message?: string;
  pin_interactions: string[];
}

/**
 * Test scenario containing multiple test steps.
 * 
 * @interface TestScenario
 * @property {string} [id] - Unique scenario identifier
 * @property {string} name - Scenario name
 * @property {string} description - Scenario description
 * @property {string} feature_file - Associated feature file path
 * @property {string} [feature_name] - Feature name from feature file
 * @property {string[]} tags - Associated tags for filtering
 * @property {TestStep[]} steps - Array of test steps
 * @property {'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error'} status - Overall scenario status
 * @property {number} [duration_ms] - Total scenario execution duration
 * @property {string} [error_message] - Error message if scenario failed
 */
export interface TestScenario {
  id?: string;  // Added missing id property
  name: string;
  description: string;
  feature_file: string;
  feature_name?: string;
  tags: string[];
  steps: TestStep[];
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
  duration_ms?: number;
  error_message?: string;
}

/**
 * Test execution session containing multiple scenarios and execution state.
 * 
 * @interface TestExecution
 * @property {string} execution_id - Unique execution session identifier
 * @property {TestScenario[]} scenarios - Array of scenarios in this execution
 * @property {'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error'} status - Overall execution status
 * @property {number} [start_time] - Execution start timestamp
 */
// === Extended Types for UI Components and Test Automation ===

// Test execution session containing multiple scenarios and execution state.
export interface TestExecution {
  execution_id: string;
  scenarios: TestScenario[];
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
  start_time?: number;
  end_time?: number;
  total_scenarios: number;
  passed_scenarios: number;
  failed_scenarios: number;
  current_scenario_index: number;
  current_step_index?: number;
  results?: any[];
}

// Centralized state for test automation UI
export interface TestAutomationState {
  availableScenarios: TestScenario[];
  availableTags: string[];
  availableFeatures: string[];
  currentExecution: TestExecution | null;
  isExecutionInProgress: boolean;
  selectedScenarios: string[];
  filterTags: string[];
}

// Pin connection interface for UI visualization
export interface PinConnection {
  arduino: string;
  atmega: string;
  signal: string;
  direction: 'IN' | 'OUT' | 'ANALOG' | 'COMM';
  description: string;
  arduinoPos?: { x: number; y: number };
  atmegaPos?: { x: number; y: number };
  readonly?: boolean;
}

// Enhanced pin state for UI components with additional visual properties
export interface EnhancedPinState extends PinState {
  connection?: PinConnection;
  isActive: boolean;
  isSelected: boolean;
  lastUpdated: string;
}