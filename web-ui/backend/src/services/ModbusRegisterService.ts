import ModbusRTU from 'modbus-serial'
import { configService } from '../config/ConfigService'

// Minimal TypeScript service to control DUT Modbus registers (RTU over serial)
// Supports: start/stop, overload reset, amplitude setpoint per sonicator
// Falls back to in-memory simulation when simulation_mode=true

export class ModbusRegisterService {
  private client: ModbusRTU
  private connected = false
  private connecting?: Promise<void>

  // Simple in-memory register storage for simulation mode
  private simRegisters = new Map<number, number>()

  // Register map constants (mirror firmware definitions)
  private static readonly REG_SONICATOR_BASE = 0x0100
  private static readonly REG_SONICATOR_STRIDE = 0x0020
  private static readonly OFF_START_STOP = 0x00
  private static readonly OFF_AMPLITUDE_SP = 0x01
  private static readonly OFF_OVERLOAD_RESET = 0x02
  private static readonly OFF_STATUS_BASE = 0x10 // power, freq, flags, amplitude_actual

  constructor() {
    this.client = new ModbusRTU()
  }

  isSimulation(): boolean {
    return configService.isSimulationMode()
  }

  isConnected(): boolean {
    return this.connected
  }

  private addrFor(sonicatorId1Based: number, offset: number): number {
    const idx = sonicatorId1Based - 1 // firmware uses 0-based indexing
    return (
      ModbusRegisterService.REG_SONICATOR_BASE +
      idx * ModbusRegisterService.REG_SONICATOR_STRIDE +
      offset
    )
  }

  async connect(): Promise<void> {
    if (this.isSimulation()) {
      // No hardware connection in simulation
      this.connected = true
      return
    }

    if (this.connected) return
    if (this.connecting) return this.connecting

    this.connecting = new Promise<void>(async (resolve, reject) => {
      try {
        const cfg = configService.getConfig()
        const modbus = cfg.communication.modbus
        const serial = cfg.communication.serial

        await this.client.connectRTUBuffered(serial.port, {
          baudRate: modbus.baud_rate,
          dataBits: modbus.data_bits as any,
          stopBits: modbus.stop_bits as any,
          parity: (modbus.parity || 'none') as any,
        })
        this.client.setID(modbus.slave_id)
        this.client.setTimeout(cfg.environment.hardware_timeout_ms || 1000)
        this.connected = true
        resolve()
      } catch (err) {
        this.connected = false
        reject(err)
      } finally {
        this.connecting = undefined
      }
    })

    return this.connecting
  }

  async disconnect(): Promise<void> {
    try {
      if (this.client && (this.client as any).isOpen) {
        await this.client.close()
      }
    } catch { /* noop */ }
    this.connected = false
  }

  private clampAmplitude(val: number): number {
    if (Number.isNaN(val)) return 20
    return Math.max(20, Math.min(100, Math.round(val)))
  }

  private validateSonicatorId(id: number) {
    if (!Number.isInteger(id) || id < 1 || id > 4) {
      throw new Error('Invalid sonicator id (must be 1..4)')
    }
  }

  // Low-level register ops
  async writeRegister(address: number, value: number): Promise<void> {
    await this.connect()

    if (this.isSimulation()) {
      this.simRegisters.set(address, value & 0xFFFF)
      return
    }

    await this.client.writeRegister(address, value & 0xFFFF)
  }

  async readRegisters(address: number, length: number): Promise<number[]> {
    await this.connect()

    if (this.isSimulation()) {
      const out: number[] = []
      for (let i = 0; i < length; i++) {
        out.push(this.simRegisters.get(address + i) ?? 0)
      }
      return out
    }

    const res = await this.client.readHoldingRegisters(address, length)
    return res.data as number[]
  }

  async readStatus(sonicatorId: number): Promise<{ address_base: number, power_watts: number, frequency_hz_div10: number, status_flags: number, amplitude_actual: number }> {
    this.validateSonicatorId(sonicatorId)
    const base = this.addrFor(sonicatorId, ModbusRegisterService.OFF_STATUS_BASE)
    const [power_watts, frequency_hz_div10, status_flags, amplitude_actual] = await this.readRegisters(base, 4)
    return { address_base: base, power_watts, frequency_hz_div10, status_flags, amplitude_actual }
  }

  // High-level control helpers
  async setStartStop(sonicatorId: number, run: boolean): Promise<{ address: number, value: number }> {
    this.validateSonicatorId(sonicatorId)
    const addr = this.addrFor(sonicatorId, ModbusRegisterService.OFF_START_STOP)
    const value = run ? 1 : 0
    await this.writeRegister(addr, value)
    return { address: addr, value }
  }

  async setAmplitude(sonicatorId: number, percent: number): Promise<{ address: number, value: number }> {
    this.validateSonicatorId(sonicatorId)
    const addr = this.addrFor(sonicatorId, ModbusRegisterService.OFF_AMPLITUDE_SP)
    const value = this.clampAmplitude(percent)
    await this.writeRegister(addr, value)
    return { address: addr, value }
  }

  async resetOverload(sonicatorId: number): Promise<{ address: number, value: number }> {
    this.validateSonicatorId(sonicatorId)
    const addr = this.addrFor(sonicatorId, ModbusRegisterService.OFF_OVERLOAD_RESET)
    const value = 1
    await this.writeRegister(addr, value)
    return { address: addr, value }
  }

  async setGlobalAmplitude(percent: number): Promise<{ address: number, value: number }> {
    const addr = 0x0011 // MODBUS_REG_GLOBAL_AMPLITUDE_SP
    const value = this.clampAmplitude(percent)
    await this.writeRegister(addr, value)
    return { address: addr, value }
  }
}

export const modbusRegisterService = new ModbusRegisterService()

