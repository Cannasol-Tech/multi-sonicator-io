import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import type { ComponentProps } from 'react'
import AdvancedControlPanel from '../AdvancedControlPanel'
import { HardwareState } from '../../types'

const hardwareState: HardwareState = {
  connection: {
    connected: true,
    port: '/dev/ttyUSB0',
    lastSeen: Date.now()
  },
  pins: {
    START_4: {
      pin: 'A3',
      signal: 'START_4',
      direction: 'OUT',
      state: 'LOW',
      timestamp: Date.now()
    },
    RESET_4: {
      pin: 'A4',
      signal: 'RESET_4',
      direction: 'OUT',
      state: 'LOW',
      timestamp: Date.now()
    },
    POWER_SENSE_4: {
      pin: 'A1',
      signal: 'POWER_SENSE_4',
      direction: 'ANALOG',
      state: 512,
      timestamp: Date.now()
    },
    FREQ_DIV10_4: {
      pin: 'D7',
      signal: 'FREQ_DIV10_4',
      direction: 'OUT',
      state: 'LOW',
      timestamp: Date.now()
    },
    FREQ_LOCK_4: {
      pin: 'D8',
      signal: 'FREQ_LOCK_4',
      direction: 'IN',
      state: 'HIGH',
      timestamp: Date.now()
    },
    OVERLOAD_4: {
      pin: 'A2',
      signal: 'OVERLOAD_4',
      direction: 'IN',
      state: 'LOW',
      timestamp: Date.now()
    }
  },
  lastUpdate: Date.now()
}

const originalFetch = global.fetch
const originalLocalStorage = global.localStorage
let mockFetch: ReturnType<typeof vi.fn>
let localStorageMock: Storage

const renderComponent = (override?: Partial<ComponentProps<typeof AdvancedControlPanel>>) => {
  const onPinClick = vi.fn()
  const onBatchOperation = vi.fn()

  const props: ComponentProps<typeof AdvancedControlPanel> = {
    hardwareState,
    onPinClick,
    onBatchOperation,
    enableDelays: false,
    ...override
  }

  render(<AdvancedControlPanel {...props} />)
  return { onPinClick, onBatchOperation }
}

beforeEach(() => {
  mockFetch = vi.fn(async (input: RequestInfo, init?: RequestInit) => {
    if (typeof input === 'string' && input === '/api/ping') {
      return {
        ok: true,
        json: async () => ({ success: true })
      } as unknown as Response
    }

    return {
      ok: true,
      json: async () => ({})
    } as unknown as Response
  })

  localStorageMock = {
    getItem: vi.fn().mockReturnValue(null),
    setItem: vi.fn(),
    removeItem: vi.fn(),
    clear: vi.fn(),
    key: vi.fn(),
    length: 0
  }

  global.fetch = mockFetch as unknown as typeof fetch
  Object.defineProperty(global, 'localStorage', {
    value: localStorageMock,
    configurable: true
  })
})

afterEach(() => {
  vi.clearAllMocks()
  global.fetch = originalFetch
  Object.defineProperty(global, 'localStorage', {
    value: originalLocalStorage,
    configurable: true
  })
})

describe('AdvancedControlPanel', () => {
  it('renders tab buttons and defaults to presets tab', async () => {
    renderComponent()

    const presetsTab = screen.getByRole('button', { name: /presets/i })
    expect(presetsTab).toHaveClass('active')
    expect(await screen.findByText('All Pins LOW')).toBeInTheDocument()
  })

  it('applies preset and sends pin commands in order', async () => {
    const { onPinClick } = renderComponent()

    const applyButton = await screen.findAllByText('▶️ Apply')
    fireEvent.click(applyButton[0])

    await waitFor(() => {
      expect(onPinClick).toHaveBeenCalledWith('START_4', 'set', 'LOW')
      expect(onPinClick).toHaveBeenCalledWith('RESET_4', 'set', 'LOW')
    })
  })

  it('switches to batch operations tab and executes batch with progress', async () => {
    const { onPinClick, onBatchOperation } = renderComponent()

    fireEvent.click(screen.getByRole('button', { name: /batch ops/i }))
    fireEvent.click(screen.getAllByText('▶️ Execute')[0])

    await waitFor(() => {
      expect(onPinClick).toHaveBeenCalled()
    })

    expect(mockFetch).toHaveBeenCalledWith('/api/ping', { method: 'POST' })
    expect(onBatchOperation).toHaveBeenCalled()
  })

  it('saves custom presets to localStorage', async () => {
    renderComponent()

    fireEvent.click(screen.getByRole('button', { name: /custom/i }))

    const nameInput = screen.getByPlaceholderText('Enter preset name...')
    const descInput = screen.getByPlaceholderText('Describe what this preset does...')

    fireEvent.change(nameInput, { target: { value: 'Test Preset' } })
    fireEvent.change(descInput, { target: { value: 'Test Description' } })

    fireEvent.click(screen.getByText('💾 Save as Preset'))

    await waitFor(() => {
      expect(localStorageMock.setItem).toHaveBeenCalledWith(
        'multi-sonicator-presets',
        expect.any(String)
      )
    })
  })

  it('adjusts power level using slider and preset buttons', () => {
    const { onPinClick } = renderComponent()

    const slider = screen.getByRole('slider')
    fireEvent.change(slider, { target: { value: '75' } })

    expect(onPinClick).toHaveBeenCalledWith('POWER_SENSE_4', '767')

    fireEvent.click(screen.getByText('25%'))
    expect(onPinClick).toHaveBeenCalledWith('POWER_SENSE_4', '256')
  })

  it('handles localStorage read errors gracefully', () => {
    const error = new Error('Storage error')
    localStorageMock.getItem = vi.fn(() => {
      throw error
    })
    const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {})

    expect(() => renderComponent()).not.toThrow()
    expect(consoleSpy).toHaveBeenCalledWith('Failed to load saved presets:', error)

    consoleSpy.mockRestore()
  })
})
