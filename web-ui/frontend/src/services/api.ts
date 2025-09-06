// API service for communicating with the backend
const API_BASE = 'http://localhost:3001/api'

export interface ApiResponse<T = any> {
  success?: boolean
  data?: T
  error?: string
  message?: string
  timestamp: number
}

class ApiService {
  private async request<T = any>(
    endpoint: string, 
    options: RequestInit = {}
  ): Promise<ApiResponse<T>> {
    try {
      const response = await fetch(`${API_BASE}${endpoint}`, {
        headers: {
          'Content-Type': 'application/json',
          ...options.headers,
        },
        ...options,
      })

      const data = await response.json()
      
      if (!response.ok) {
        throw new Error(data.error || `HTTP ${response.status}`)
      }

      return data
    } catch (error) {
      console.error(`API request failed: ${endpoint}`, error)
      throw error
    }
  }

  // Health check
  async getHealth() {
    return this.request('/health')
  }

  // Pin operations
  async getPins() {
    return this.request('/pins')
  }

  async getPin(signal: string) {
    return this.request(`/pins/${signal}`)
  }

  // Hardware commands
  async sendCommand(command: string, args?: string[]) {
    return this.request('/command', {
      method: 'POST',
      body: JSON.stringify({ command, args })
    })
  }

  // Ping hardware
  async ping() {
    return this.request('/ping', {
      method: 'POST'
    })
  }

  // Connection management
  async getConnectionStatus() {
    return this.request('/connection')
  }

  async reconnect() {
    return this.request('/connection/reconnect', {
      method: 'POST'
    })
  }

  async retryConnection(maxAttempts: number = 3, delayMs: number = 2000) {
    return this.request('/connection/retry', {
      method: 'POST',
      body: JSON.stringify({ maxAttempts, delayMs })
    })
  }
}

export const api = new ApiService()
