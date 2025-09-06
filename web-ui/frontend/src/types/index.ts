// Multi-Sonicator-IO Web UI Type Definitions
// Based on docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)

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

export interface ConnectionStatus {
  connected: boolean;
  port?: string;
  lastSeen?: number;
  error?: string;
}

export interface HardwareState {
  connection: ConnectionStatus;
  pins: Record<string, PinState>;
  lastUpdate: number;
}

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

export interface WebSocketMessage {
  type: 'pin_update' | 'connection_status' | 'error' | 'command_response' | 'test_progress' | 'test_complete' | 'test_error' | 'test_stopped' | 'initial_state' | 'arduino_command_response' | 'arduino_command_sent';
  data: any;
  timestamp: number;
}

export interface HardwareCommand {
  command: string;
  args?: string[];
  expectResponse?: boolean;
}

export interface HardwareResponse {
  success: boolean;
  data?: any;
  error?: string;
  timestamp: number;
}

export interface HelpContent {
  title: string;
  description: string;
  examples?: string[];
  relatedPins?: string[];
}

export interface TestSequence {
  name: string;
  description: string;
  steps: HardwareCommand[];
  expectedResults?: any[];
}

// Test Automation Types
export interface TestStep {
  step_type: string;
  description: string;
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
  duration_ms?: number;
  error_message?: string;
  pin_interactions: string[];
}

export interface TestScenario {
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
}

export interface TestAutomationState {
  availableScenarios: TestScenario[];
  availableTags: string[];
  availableFeatures: string[];
  currentExecution: TestExecution | null;
  isExecutionInProgress: boolean;
  selectedScenarios: string[];
  filterTags: string[];
}
