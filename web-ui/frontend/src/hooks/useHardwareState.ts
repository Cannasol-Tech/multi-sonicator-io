import { useState, useCallback } from 'react'
import { HardwareState, PinState, ConnectionStatus } from '../types'

// Initial pin states based on pin-matrix.md (SOLE SOURCE OF TRUTH)
const initialPinStates: Record<string, PinState> = {
  // Sonicator 4 (S4) - Single channel HIL mapping
  'FREQ_DIV10_4': {
    pin: 'D7',
    signal: 'FREQ_DIV10_4',
    direction: 'IN',
    state: 'LOW',
    timestamp: Date.now(),
    description: 'Frequency ÷10 input to ATmega32A PB0'
  },
  'FREQ_LOCK_4': {
    pin: 'D8', 
    signal: 'FREQ_LOCK_4',
    direction: 'IN',
    state: 'LOW',
    timestamp: Date.now(),
    description: 'Frequency lock input to ATmega32A PB4'
  },
  'OVERLOAD_4': {
    pin: 'A2',
    signal: 'OVERLOAD_4', 
    direction: 'IN',
    state: 'LOW',
    timestamp: Date.now(),
    description: 'Overload input to ATmega32A PD3'
  },
  'START_4': {
    pin: 'A3',
    signal: 'START_4',
    direction: 'OUT', 
    state: 'LOW',
    timestamp: Date.now(),
    description: 'Start output from ATmega32A PC0'
  },
  'RESET_4': {
    pin: 'A4',
    signal: 'RESET_4',
    direction: 'OUT',
    state: 'LOW', 
    timestamp: Date.now(),
    description: 'Reset output from ATmega32A PC1'
  },
  'POWER_SENSE_4': {
    pin: 'A1',
    signal: 'POWER_SENSE_4',
    direction: 'ANALOG',
    state: 0,
    timestamp: Date.now(),
    description: 'Power sense analog from ATmega32A PA7'
  },
  'AMPLITUDE_ALL': {
    pin: 'D9',
    signal: 'AMPLITUDE_ALL',
    direction: 'OUT',
    state: '0%',
    timestamp: Date.now(),
    description: 'Amplitude PWM output from ATmega32A PD7'
  },
  'UART_RXD': {
    pin: 'D10',
    signal: 'UART_RXD',
    direction: 'IN',
    state: 'HIGH',
    timestamp: Date.now(),
    description: 'UART RX to ATmega32A PD0'
  },
  'UART_TXD': {
    pin: 'D11', 
    signal: 'UART_TXD',
    direction: 'OUT',
    state: 'HIGH',
    timestamp: Date.now(),
    description: 'UART TX from ATmega32A PD1'
  },
  'STATUS_LED': {
    pin: 'D12',
    signal: 'STATUS_LED',
    direction: 'OUT',
    state: 'LOW',
    timestamp: Date.now(),
    description: 'Status LED from ATmega32A PD2'
  }
}

const initialConnectionStatus: ConnectionStatus = {
  connected: false,
  lastSeen: undefined,
  error: 'Not connected'
}

const initialHardwareState: HardwareState = {
  connection: initialConnectionStatus,
  pins: initialPinStates,
  lastUpdate: Date.now()
}

export function useHardwareState() {
  const [hardwareState, setHardwareState] = useState<HardwareState>(initialHardwareState)

  const updatePinState = useCallback((pinName: string, newState: Partial<PinState>) => {
    setHardwareState(prev => ({
      ...prev,
      pins: {
        ...prev.pins,
        [pinName]: {
          ...prev.pins[pinName],
          ...newState,
          timestamp: Date.now()
        }
      },
      lastUpdate: Date.now()
    }))
  }, [])

  const updateMultiplePins = useCallback((updates: Record<string, Partial<PinState>>) => {
    setHardwareState(prev => {
      const newPins = { ...prev.pins }
      const timestamp = Date.now()
      
      Object.entries(updates).forEach(([pinName, update]) => {
        if (newPins[pinName]) {
          newPins[pinName] = {
            ...newPins[pinName],
            ...update,
            timestamp
          }
        }
      })
      
      return {
        ...prev,
        pins: newPins,
        lastUpdate: timestamp
      }
    })
  }, [])

  const setConnectionStatus = useCallback((status: Partial<ConnectionStatus>) => {
    setHardwareState(prev => ({
      ...prev,
      connection: {
        ...prev.connection,
        ...status
      },
      lastUpdate: Date.now()
    }))
  }, [])

  const resetHardwareState = useCallback(() => {
    setHardwareState(initialHardwareState)
  }, [])

  const getPinsByDirection = useCallback((direction: 'IN' | 'OUT' | 'ANALOG') => {
    return Object.entries(hardwareState.pins)
      .filter(([_, pin]) => pin.direction === direction)
      .reduce((acc, [name, pin]) => ({ ...acc, [name]: pin }), {})
  }, [hardwareState.pins])

  const getInputPins = useCallback(() => getPinsByDirection('IN'), [getPinsByDirection])
  const getOutputPins = useCallback(() => getPinsByDirection('OUT'), [getPinsByDirection])  
  const getAnalogPins = useCallback(() => getPinsByDirection('ANALOG'), [getPinsByDirection])

  return {
    hardwareState,
    updatePinState,
    updateMultiplePins,
    setConnectionStatus,
    resetHardwareState,
    getInputPins,
    getOutputPins,
    getAnalogPins
  }
}
