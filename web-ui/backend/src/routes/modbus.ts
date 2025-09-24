import { Router, Request, Response } from 'express'
import { modbusRegisterService } from '../services/ModbusRegisterService'

const router = Router()

// POST /api/modbus/sonicator/:id/start
// GET /api/modbus/sonicator/:id/status
router.get('/sonicator/:id/status', async (req: Request, res: Response) => {
  try {
    const id = parseInt(req.params.id, 10)
    const status = await modbusRegisterService.readStatus(id)
    res.json({ success: true, id, status, simulation: modbusRegisterService.isSimulation(), timestamp: Date.now() })
  } catch (error: any) {
    res.status(400).json({ success: false, error: error?.message || 'Failed to read status', timestamp: Date.now() })
  }
})

router.post('/sonicator/:id/start', async (req: Request, res: Response) => {
  try {
    const id = parseInt(req.params.id, 10)
    const result = await modbusRegisterService.setStartStop(id, true)
    res.json({ success: true, action: 'start', id, result, simulation: modbusRegisterService.isSimulation(), timestamp: Date.now() })
  } catch (error: any) {
    res.status(400).json({ success: false, error: error?.message || 'Failed to start sonicator', timestamp: Date.now() })
  }
})

// POST /api/modbus/sonicator/:id/stop
router.post('/sonicator/:id/stop', async (req: Request, res: Response) => {
  try {
    const id = parseInt(req.params.id, 10)
    const result = await modbusRegisterService.setStartStop(id, false)
    res.json({ success: true, action: 'stop', id, result, simulation: modbusRegisterService.isSimulation(), timestamp: Date.now() })
  } catch (error: any) {
    res.status(400).json({ success: false, error: error?.message || 'Failed to stop sonicator', timestamp: Date.now() })
  }
})

// POST /api/modbus/sonicator/:id/amplitude { value: number }
router.post('/sonicator/:id/amplitude', async (req: Request, res: Response) => {
  try {
    const id = parseInt(req.params.id, 10)
    const { value } = req.body || {}
    if (value === undefined) {
      return res.status(400).json({ success: false, error: 'Missing body field: value (percent)' })
    }
    const result = await modbusRegisterService.setAmplitude(id, Number(value))
    res.json({ success: true, action: 'amplitude', id, result, simulation: modbusRegisterService.isSimulation(), timestamp: Date.now() })
  } catch (error: any) {
    res.status(400).json({ success: false, error: error?.message || 'Failed to set amplitude', timestamp: Date.now() })
  }
})

// POST /api/modbus/sonicator/:id/overload_reset
router.post('/sonicator/:id/overload_reset', async (req: Request, res: Response) => {
  try {
    const id = parseInt(req.params.id, 10)
    const result = await modbusRegisterService.resetOverload(id)
    res.json({ success: true, action: 'overload_reset', id, result, simulation: modbusRegisterService.isSimulation(), timestamp: Date.now() })
  } catch (error: any) {
    res.status(400).json({ success: false, error: error?.message || 'Failed to reset overload', timestamp: Date.now() })
  }
})

export default router

