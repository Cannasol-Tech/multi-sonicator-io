import { describe, it, expect } from 'vitest'

// Simple validation utilities for testing
export function validateEmail(email: string): boolean {
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/
  return emailRegex.test(email)
}

export function validateUrl(url: string): boolean {
  try {
    new URL(url)
    return true
  } catch {
    return false
  }
}

export function validatePinName(pinName: string): boolean {
  const pinRegex = /^(D([0-9]|1[0-3])|A[0-7]|RESET_\d+|PWM_\d+)$/
  return pinRegex.test(pinName)
}

export function validateFrequency(frequency: number): boolean {
  return frequency > 0 && frequency <= 1000000 // 1MHz max
}

export function validatePwmValue(value: number): boolean {
  return value >= 0 && value <= 255
}

export function sanitizeHtml(input: string): string {
  return input
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#x27;')
    .replace(/\//g, '&#x2F;')
}

export function isValidJson(str: string): boolean {
  try {
    JSON.parse(str)
    return true
  } catch {
    return false
  }
}

export function validateTimeout(timeout: number): boolean {
  return timeout > 0 && timeout <= 300000 // 5 minutes max
}

describe('Validation Utilities', () => {
  describe('validateEmail', () => {
    it('validates correct email addresses', () => {
      expect(validateEmail('test@example.com')).toBe(true)
      expect(validateEmail('user.name@domain.co.uk')).toBe(true)
      expect(validateEmail('test+tag@example.org')).toBe(true)
    })

    it('rejects invalid email addresses', () => {
      expect(validateEmail('invalid-email')).toBe(false)
      expect(validateEmail('@example.com')).toBe(false)
      expect(validateEmail('test@')).toBe(false)
      expect(validateEmail('test@.com')).toBe(false)
    })
  })

  describe('validateUrl', () => {
    it('validates correct URLs', () => {
      expect(validateUrl('http://example.com')).toBe(true)
      expect(validateUrl('https://example.com')).toBe(true)
      expect(validateUrl('ws://localhost:3000')).toBe(true)
      expect(validateUrl('wss://secure.example.com')).toBe(true)
    })

    it('rejects invalid URLs', () => {
      expect(validateUrl('not-a-url')).toBe(false)
      expect(validateUrl('http://')).toBe(false)
      expect(validateUrl('://example.com')).toBe(false)
    })
  })

  describe('validatePinName', () => {
    it('validates correct pin names', () => {
      expect(validatePinName('D8')).toBe(true)
      expect(validatePinName('D13')).toBe(true)
      expect(validatePinName('A0')).toBe(true)
      expect(validatePinName('A7')).toBe(true)
      expect(validatePinName('RESET_4')).toBe(true)
      expect(validatePinName('PWM_9')).toBe(true)
    })

    it('rejects invalid pin names', () => {
      expect(validatePinName('D')).toBe(false)
      expect(validatePinName('D99')).toBe(false)
      expect(validatePinName('B8')).toBe(false)
      expect(validatePinName('invalid')).toBe(false)
    })
  })

  describe('validateFrequency', () => {
    it('validates correct frequencies', () => {
      expect(validateFrequency(1)).toBe(true)
      expect(validateFrequency(1000)).toBe(true)
      expect(validateFrequency(50000)).toBe(true)
      expect(validateFrequency(1000000)).toBe(true)
    })

    it('rejects invalid frequencies', () => {
      expect(validateFrequency(0)).toBe(false)
      expect(validateFrequency(-1)).toBe(false)
      expect(validateFrequency(1000001)).toBe(false)
    })
  })

  describe('validatePwmValue', () => {
    it('validates correct PWM values', () => {
      expect(validatePwmValue(0)).toBe(true)
      expect(validatePwmValue(128)).toBe(true)
      expect(validatePwmValue(255)).toBe(true)
    })

    it('rejects invalid PWM values', () => {
      expect(validatePwmValue(-1)).toBe(false)
      expect(validatePwmValue(256)).toBe(false)
      expect(validatePwmValue(1000)).toBe(false)
    })
  })

  describe('sanitizeHtml', () => {
    it('sanitizes HTML characters', () => {
      expect(sanitizeHtml('<script>alert("xss")</script>')).toBe('&lt;script&gt;alert(&quot;xss&quot;)&lt;&#x2F;script&gt;')
      expect(sanitizeHtml('Hello <b>World</b>')).toBe('Hello &lt;b&gt;World&lt;&#x2F;b&gt;')
      expect(sanitizeHtml("It's a 'test' & \"demo\"")).toBe('It&#x27;s a &#x27;test&#x27; &amp; &quot;demo&quot;')
    })

    it('handles empty and normal strings', () => {
      expect(sanitizeHtml('')).toBe('')
      expect(sanitizeHtml('Normal text')).toBe('Normal text')
    })
  })

  describe('isValidJson', () => {
    it('validates correct JSON', () => {
      expect(isValidJson('{"key": "value"}')).toBe(true)
      expect(isValidJson('[]')).toBe(true)
      expect(isValidJson('null')).toBe(true)
      expect(isValidJson('123')).toBe(true)
      expect(isValidJson('"string"')).toBe(true)
    })

    it('rejects invalid JSON', () => {
      expect(isValidJson('invalid json')).toBe(false)
      expect(isValidJson('{key: value}')).toBe(false)
      expect(isValidJson("{'key': 'value'}")).toBe(false)
      expect(isValidJson('')).toBe(false)
    })
  })

  describe('validateTimeout', () => {
    it('validates correct timeout values', () => {
      expect(validateTimeout(1000)).toBe(true)
      expect(validateTimeout(30000)).toBe(true)
      expect(validateTimeout(300000)).toBe(true)
    })

    it('rejects invalid timeout values', () => {
      expect(validateTimeout(0)).toBe(false)
      expect(validateTimeout(-1000)).toBe(false)
      expect(validateTimeout(300001)).toBe(false)
    })
  })
})
