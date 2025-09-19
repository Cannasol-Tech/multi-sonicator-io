import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, fireEvent, waitFor, within } from '@testing-library/react'
import type { ComponentProps } from 'react'
import ControlPanel from '../ControlPanel'
import { HardwareState, PinState } from '../../types'

const basePinState: PinState = {
  pin: 'D7',
  signal: 'FREQ_DIV10_4',
  direction: 'OUT',
  state: 'LOW',
  timestamp: Date.now(),
  description: 'Test pin'
}

const hardwareState: HardwareState = {
  connection: {
    connected: true,
    port: '/dev/ttyUSB0',
    lastSeen: Date.now()
  },
  pins: {
    FREQ_DIV10_4: { ...basePinState },
    FREQ_LOCK_4: { ...basePinState, signal: 'FREQ_LOCK_4', pin: 'D8' },
    OVERLOAD_4: { ...basePinState, signal: 'OVERLOAD_4', pin: 'A2' },
    START_4: { ...basePinState, signal: 'START_4', pin: 'A3' },
    RESET_4: { ...basePinState, signal: 'RESET_4', pin: 'A4' },
    POWER_SENSE_4: { ...basePinState, signal: 'POWER_SENSE_4', pin: 'A1', direction: 'ANALOG', state: 512 },
    AMPLITUDE_ALL: { ...basePinState, signal: 'AMPLITUDE_ALL', pin: 'D9', state: '50%' },
    UART_TXD: { ...basePinState, signal: 'UART_TXD', pin: 'D11' },
    STATUS_LED: { ...basePinState, signal: 'STATUS_LED', pin: 'D12', state: 'HIGH' }
  },
  lastUpdate: Date.now()
}

const defaultConfig = {
  sonicator4: {
    operatingFrequencyKHz: 20,
    outputFrequencyKHz: 2,
    enabled: true,
    manualMode: false,
    manualFrequencyKHz: 2
  }
}

const originalFetch = global.fetch
let mockFetch: ReturnType<typeof vi.fn>

beforeEach(() => {
  mockFetch = vi.fn(async (_input: RequestInfo, init?: RequestInit) => {
    const isPost = init?.method === 'POST'

    if (isPost) {
      return {
        ok: true,
        json: async () => ({ success: true })
      } as unknown as Response
    }

    return {
      ok: true,
      json: async () => ({ success: true, config: defaultConfig })
    } as unknown as Response
  })

  global.fetch = mockFetch as unknown as typeof fetch
})

afterEach(() => {
  vi.clearAllMocks()
  global.fetch = originalFetch
})

describe('ControlPanel', () => {
  const renderComponent = async (override?: Partial<ComponentProps<typeof ControlPanel>>) => {
    const onPinControl = vi.fn()
    const props: ComponentProps<typeof ControlPanel> = {
      hardwareState,
      onPinControl,
      connected: true,
      ...override
    }

    render(<ControlPanel {...props} />)
    await waitFor(() => expect(mockFetch).toHaveBeenCalled())

    return { onPinControl, props }
  }

  it('renders configurable parameters tab by default', async () => {
    await renderComponent()

    expect(screen.getByRole('button', { name: /configurable parameters/i })).toHaveClass('active')
    expect(await screen.findByText('FREQ_DIV10_4')).toBeInTheDocument()
  })

  it('switches to monitoring tab when selected', async () => {
    await renderComponent()

    const monitoringTab = screen.getByRole('button', { name: /live dut monitoring/i })
    fireEvent.click(monitoringTab)

    expect(monitoringTab).toHaveClass('active')
    expect(await screen.findByText('POWER_SENSE_4')).toBeInTheDocument()
  })

  it('toggles control pins via onPinControl callback', async () => {
    const { onPinControl } = await renderComponent()

    const card = await screen.findByText(/Frequency Lock Input/i)
    const parameterCard = card.closest('.parameter-card-compact')
    const toggleButton = parameterCard ? within(parameterCard).getByRole('button', { name: /toggle/i }) : null

    expect(toggleButton).not.toBeNull()
    fireEvent.click(toggleButton as HTMLButtonElement)

    expect(onPinControl).toHaveBeenCalledWith('FREQ_LOCK_4', 'write_pin', 'HIGH')
  })

  it('disables controls when disconnected', async () => {
    await renderComponent({ connected: false })

    const toggleButtons = await screen.findAllByRole('button', { name: /toggle/i })
    toggleButtons.forEach(button => {
      expect(button).toBeDisabled()
    })
  })

  it('updates frequency using set_frequency command and saves configuration', async () => {
    const { onPinControl } = await renderComponent()

    const frequencyInput = await screen.findByTitle('Frequency (kHz)')
    fireEvent.change(frequencyInput, { target: { value: '21' } })

    await waitFor(() => {
      expect(onPinControl).toHaveBeenCalledWith('FREQ_DIV10_4', 'set_frequency', 21)
    })

    expect(mockFetch).toHaveBeenCalledWith('/api/config', expect.objectContaining({ method: 'POST' }))
  })
})
