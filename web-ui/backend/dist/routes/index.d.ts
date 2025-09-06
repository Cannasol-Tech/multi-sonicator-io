import { Express } from 'express';
import { HardwareInterface } from '../adapters/HardwareInterface.js';
import { TestAutomationService } from '../services/TestAutomationService.js';
export declare function setupRoutes(app: Express, hardwareInterface: HardwareInterface, testAutomationService?: TestAutomationService): void;
