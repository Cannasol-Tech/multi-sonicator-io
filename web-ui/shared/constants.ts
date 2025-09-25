/**
 * Shared constants for Multi-Sonicator-IO Web UI
 * 
 * This file defines all port numbers, URLs, and other constants used across
 * the frontend, backend, and configuration files to avoid hardcoding values
 * in multiple places.
 * 
 * @fileoverview Centralized constants for web-ui components
 * @author Multi-Sonicator-IO Team
 * @version 1.0.0
 */

// ============================================================================
// SERVER PORTS
// ============================================================================

/**
 * Backend server port (Node.js/Express + WebSocket)
 * Used by: backend/src/server.ts, Makefile, docker-compose.yml
 */
export const BACKEND_PORT = 3001;

/**
 * Frontend development server port (Vite)
 * Used by: frontend/vite.config.ts, Makefile
 */
export const FRONTEND_DEV_PORT = 3102;

/**
 * Frontend production port (when serving built assets)
 * Used by: production deployments, nginx configurations
 */
export const FRONTEND_PROD_PORT = 3101;

// ============================================================================
// API ENDPOINTS
// ============================================================================

/**
 * Backend API base URL for development
 */
export const API_BASE_URL = `http://localhost:${BACKEND_PORT}/api`;

/**
 * WebSocket endpoint URL for development
 */
export const WEBSOCKET_URL = `ws://localhost:${BACKEND_PORT}/ws`;

/**
 * Health check endpoint
 */
export const HEALTH_ENDPOINT = `${API_BASE_URL}/health`;

// ============================================================================
// WEBSOCKET CONFIGURATION
// ============================================================================

/**
 * WebSocket reconnection settings
 */
export const WEBSOCKET_CONFIG = {
  /** Maximum number of reconnection attempts */
  MAX_RECONNECT_ATTEMPTS: 5,
  
  /** Initial reconnection delay in milliseconds */
  RECONNECT_DELAY: 1000,
  
  /** Maximum reconnection delay in milliseconds */
  MAX_RECONNECT_DELAY: 30000,
  
  /** Multiplier for exponential backoff */
  RECONNECT_BACKOFF_MULTIPLIER: 1.5,
  
  /** Ping interval to keep connection alive (milliseconds) */
  PING_INTERVAL: 30000,
  
  /** Timeout for ping responses (milliseconds) */
  PING_TIMEOUT: 5000,
} as const;

// ============================================================================
// HARDWARE COMMUNICATION
// ============================================================================

/**
 * Hardware interface timeouts and limits
 */
export const HARDWARE_CONFIG = {
  /** Default command timeout in milliseconds */
  COMMAND_TIMEOUT: 5000,
  
  /** Hardware connection timeout in milliseconds */
  CONNECTION_TIMEOUT: 10000,
  
  /** Maximum number of retry attempts for hardware commands */
  MAX_COMMAND_RETRIES: 3,
  
  /** Delay between command retries in milliseconds */
  RETRY_DELAY: 1000,
  
  /** Polling interval for hardware status updates (milliseconds) */
  STATUS_POLL_INTERVAL: 1000,
} as const;

// ============================================================================
// TEST AUTOMATION
// ============================================================================

/**
 * Test automation configuration
 */
export const TEST_CONFIG = {
  /** Maximum test execution time in milliseconds */
  MAX_EXECUTION_TIME: 300000, // 5 minutes
  
  /** Test step timeout in milliseconds */
  STEP_TIMEOUT: 30000, // 30 seconds
  
  /** Delay between test steps in milliseconds */
  STEP_DELAY: 500,
  
  /** Maximum number of test retries */
  MAX_TEST_RETRIES: 2,
} as const;

// ============================================================================
// UI CONFIGURATION
// ============================================================================

/**
 * User interface settings
 */
export const UI_CONFIG = {
  /** Default auto-refresh interval for live data (milliseconds) */
  DEFAULT_REFRESH_INTERVAL: 1000,
  
  /** Maximum number of log entries to display */
  MAX_LOG_ENTRIES: 1000,
  
  /** Default timeout for notifications (milliseconds) */
  NOTIFICATION_TIMEOUT: 5000,
  
  /** Animation duration for UI transitions (milliseconds) */
  ANIMATION_DURATION: 300,
} as const;

// ============================================================================
// ENVIRONMENT DETECTION
// ============================================================================

/**
 * Detect current environment and return appropriate configuration
 */
export const getEnvironmentConfig = () => {
  const isDevelopment = process.env.NODE_ENV === 'development';
  const isProduction = process.env.NODE_ENV === 'production';
  const isTest = process.env.NODE_ENV === 'test';
  
  return {
    isDevelopment,
    isProduction,
    isTest,
    
    // Use environment-specific ports if available
    backendPort: parseInt(process.env.BACKEND_PORT || String(BACKEND_PORT)),
    frontendPort: parseInt(process.env.FRONTEND_PORT || String(isDevelopment ? FRONTEND_DEV_PORT : FRONTEND_PROD_PORT)),
    
    // Environment-specific API URLs
    apiBaseUrl: process.env.API_BASE_URL || `http://localhost:${BACKEND_PORT}/api`,
    websocketUrl: process.env.WEBSOCKET_URL || `ws://localhost:${BACKEND_PORT}/ws`,
  };
};

// ============================================================================
// VALIDATION HELPERS
// ============================================================================

/**
 * Validate that required ports are available
 */
export const validatePorts = () => {
  const config = getEnvironmentConfig();
  
  if (config.backendPort === config.frontendPort) {
    throw new Error(`Port conflict: Backend and frontend cannot use the same port (${config.backendPort})`);
  }
  
  const validPortRange = (port: number) => port >= 1024 && port <= 65535;
  
  if (!validPortRange(config.backendPort)) {
    throw new Error(`Invalid backend port: ${config.backendPort}. Must be between 1024-65535`);
  }
  
  if (!validPortRange(config.frontendPort)) {
    throw new Error(`Invalid frontend port: ${config.frontendPort}. Must be between 1024-65535`);
  }
  
  return true;
};

// ============================================================================
// EXPORTS
// ============================================================================

/**
 * Default export with all constants grouped by category
 */
export default {
  PORTS: {
    BACKEND: BACKEND_PORT,
    FRONTEND_DEV: FRONTEND_DEV_PORT,
    FRONTEND_PROD: FRONTEND_PROD_PORT,
  },
  
  URLS: {
    API_BASE: API_BASE_URL,
    WEBSOCKET: WEBSOCKET_URL,
    HEALTH: HEALTH_ENDPOINT,
  },
  
  WEBSOCKET: WEBSOCKET_CONFIG,
  HARDWARE: HARDWARE_CONFIG,
  TEST: TEST_CONFIG,
  UI: UI_CONFIG,
  
  // Helper functions
  getEnvironmentConfig,
  validatePorts,
} as const;
