/**
 * Hardware Test API Routes
 * Provides endpoints for running hardware tests using configuration
 */

import { Router, Request, Response } from 'express';
import { hardwareTestService } from '../services/HardwareTestService';

const router = Router();

/**
 * POST /api/tests/configuration
 * Run hardware configuration validation tests
 */
router.post('/configuration', async (req: Request, res: Response) => {
  try {
    console.log('🧪 Running hardware configuration tests...');
    const testSuite = await hardwareTestService.runConfigurationTests();
    
    res.json({
      success: true,
      data: testSuite,
      timestamp: Date.now()
    });
  } catch (error) {
    console.error('Configuration tests failed:', error);
    res.status(500).json({
      success: false,
      error: 'Configuration tests failed',
      message: error instanceof Error ? error.message : 'Unknown error'
    });
  }
});

/**
 * POST /api/tests/sonicator/:id
 * Run tests for specific sonicator channel
 */
router.post('/sonicator/:id', async (req: Request, res: Response) => {
  try {
    const channelId = parseInt(req.params.id);
    if (isNaN(channelId) || channelId < 1 || channelId > 4) {
      return res.status(400).json({
        success: false,
        error: 'Invalid sonicator channel ID',
        message: 'Channel ID must be between 1 and 4'
      });
    }

    console.log(`🧪 Running sonicator ${channelId} tests...`);
    const testSuite = await hardwareTestService.runSonicatorTests(channelId);
    
    res.json({
      success: true,
      data: testSuite,
      timestamp: Date.now()
    });
  } catch (error) {
    console.error(`Sonicator ${req.params.id} tests failed:`, error);
    res.status(500).json({
      success: false,
      error: 'Sonicator tests failed',
      message: error instanceof Error ? error.message : 'Unknown error'
    });
  }
});

/**
 * POST /api/tests/modbus
 * Run MODBUS communication tests
 */
router.post('/modbus', async (req: Request, res: Response) => {
  try {
    console.log('🧪 Running MODBUS communication tests...');
    const testSuite = await hardwareTestService.runModbusTests();
    
    res.json({
      success: true,
      data: testSuite,
      timestamp: Date.now()
    });
  } catch (error) {
    console.error('MODBUS tests failed:', error);
    res.status(500).json({
      success: false,
      error: 'MODBUS tests failed',
      message: error instanceof Error ? error.message : 'Unknown error'
    });
  }
});

/**
 * POST /api/tests/all
 * Run all available hardware tests
 */
router.post('/all', async (req: Request, res: Response) => {
  try {
    console.log('🧪 Running all hardware tests...');
    const startTime = Date.now();
    
    // Run all test suites
    const configTests = await hardwareTestService.runConfigurationTests();
    const modbusTests = await hardwareTestService.runModbusTests();
    
    // Run tests for connected sonicators only
    const sonicatorTests = [];
    for (let i = 1; i <= 4; i++) {
      const sonicatorTestSuite = await hardwareTestService.runSonicatorTests(i);
      sonicatorTests.push(sonicatorTestSuite);
    }
    
    const totalDuration = Date.now() - startTime;
    
    // Aggregate results
    const allSuites = [configTests, modbusTests, ...sonicatorTests];
    const totalTests = allSuites.reduce((sum, suite) => sum + suite.tests.length, 0);
    const totalPassed = allSuites.reduce((sum, suite) => sum + suite.passed, 0);
    const totalFailed = allSuites.reduce((sum, suite) => sum + suite.failed, 0);
    const totalSkipped = allSuites.reduce((sum, suite) => sum + suite.skipped, 0);
    
    const summary = {
      name: 'Complete Hardware Test Suite',
      description: 'All hardware configuration and functionality tests',
      suites: allSuites,
      summary: {
        totalTests,
        totalPassed,
        totalFailed,
        totalSkipped,
        passRate: totalTests > 0 ? Math.round((totalPassed / totalTests) * 100) : 0,
        duration: totalDuration
      }
    };
    
    res.json({
      success: true,
      data: summary,
      timestamp: Date.now()
    });
  } catch (error) {
    console.error('All hardware tests failed:', error);
    res.status(500).json({
      success: false,
      error: 'Hardware tests failed',
      message: error instanceof Error ? error.message : 'Unknown error'
    });
  }
});

/**
 * GET /api/tests/status
 * Get test capabilities and status
 */
router.get('/status', async (req: Request, res: Response) => {
  try {
    // This would integrate with your existing test automation service
    const status = {
      available_tests: [
        {
          name: 'configuration',
          description: 'Hardware configuration validation tests',
          endpoint: '/api/tests/configuration',
          estimated_duration: '2 seconds'
        },
        {
          name: 'modbus',
          description: 'MODBUS RTU communication tests',
          endpoint: '/api/tests/modbus',
          estimated_duration: '5 seconds'
        },
        {
          name: 'sonicator_1',
          description: 'Sonicator channel 1 tests',
          endpoint: '/api/tests/sonicator/1',
          estimated_duration: '3 seconds',
          status: 'not_connected'
        },
        {
          name: 'sonicator_2',
          description: 'Sonicator channel 2 tests',
          endpoint: '/api/tests/sonicator/2',
          estimated_duration: '3 seconds',
          status: 'not_connected'
        },
        {
          name: 'sonicator_3',
          description: 'Sonicator channel 3 tests',
          endpoint: '/api/tests/sonicator/3',
          estimated_duration: '3 seconds',
          status: 'not_connected'
        },
        {
          name: 'sonicator_4',
          description: 'Sonicator channel 4 tests',
          endpoint: '/api/tests/sonicator/4',
          estimated_duration: '3 seconds',
          status: 'connected'
        },
        {
          name: 'all',
          description: 'Complete hardware test suite',
          endpoint: '/api/tests/all',
          estimated_duration: '15 seconds'
        }
      ],
      integration: {
        existing_unity_tests: true,
        existing_pytest_tests: true,
        existing_behave_tests: true,
        configuration_driven: true
      }
    };
    
    res.json({
      success: true,
      data: status,
      timestamp: Date.now()
    });
  } catch (error) {
    console.error('Failed to get test status:', error);
    res.status(500).json({
      success: false,
      error: 'Failed to get test status',
      message: error instanceof Error ? error.message : 'Unknown error'
    });
  }
});

export default router;
