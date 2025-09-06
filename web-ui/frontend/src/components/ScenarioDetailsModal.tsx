import React from 'react'
import { TestScenario } from '../types'

interface ScenarioDetailsModalProps {
  scenario: TestScenario | null
  visible: boolean
  onClose: () => void
}

export const ScenarioDetailsModal: React.FC<ScenarioDetailsModalProps> = ({
  scenario,
  visible,
  onClose
}) => {
  if (!visible || !scenario) return null

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content scenario-details-modal" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h2 style={{ 
            color: 'var(--text-primary)', 
            margin: 0, 
            fontSize: '20px', 
            fontWeight: '600' 
          }}>
            📋 Test Scenario Details
          </h2>
          <button
            className="modal-close-btn"
            onClick={onClose}
            style={{
              background: 'none',
              border: 'none',
              fontSize: '24px',
              cursor: 'pointer',
              color: 'var(--text-secondary)',
              padding: '0',
              width: '32px',
              height: '32px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              borderRadius: '4px'
            }}
          >
            ×
          </button>
        </div>

        <div className="modal-body">
          {/* Scenario Name */}
          <div className="detail-section">
            <h3 style={{ 
              color: 'var(--text-primary)', 
              fontSize: '18px', 
              fontWeight: '600',
              marginBottom: '8px'
            }}>
              {scenario.name}
            </h3>
          </div>

          {/* Description */}
          <div className="detail-section">
            <h4 style={{ 
              color: 'var(--text-secondary)', 
              fontSize: '14px', 
              fontWeight: '600',
              marginBottom: '8px'
            }}>
              Description
            </h4>
            <p style={{ 
              color: 'var(--text-primary)', 
              fontSize: '14px',
              lineHeight: '1.5',
              margin: 0
            }}>
              {scenario.description}
            </p>
          </div>

          {/* Feature File */}
          <div className="detail-section">
            <h4 style={{ 
              color: 'var(--text-secondary)', 
              fontSize: '14px', 
              fontWeight: '600',
              marginBottom: '8px'
            }}>
              Feature File
            </h4>
            <p style={{ 
              color: 'var(--text-primary)', 
              fontSize: '14px',
              fontFamily: 'monospace',
              background: 'var(--bg-tertiary)',
              padding: '8px',
              borderRadius: '4px',
              margin: 0
            }}>
              📄 {scenario.feature_file}
            </p>
          </div>

          {/* Tags */}
          {scenario.tags.length > 0 && (
            <div className="detail-section">
              <h4 style={{ 
                color: 'var(--text-secondary)', 
                fontSize: '14px', 
                fontWeight: '600',
                marginBottom: '8px'
              }}>
                Tags
              </h4>
              <div className="tags-container">
                {scenario.tags.map((tag, index) => (
                  <span
                    key={index}
                    className="tag-badge"
                    style={{
                      background: 'var(--color-primary)',
                      color: 'white',
                      padding: '4px 8px',
                      borderRadius: '4px',
                      fontSize: '12px',
                      fontWeight: '500',
                      marginRight: '8px',
                      marginBottom: '4px',
                      display: 'inline-block'
                    }}
                  >
                    @{tag}
                  </span>
                ))}
              </div>
            </div>
          )}

          {/* Test Steps */}
          <div className="detail-section">
            <h4 style={{ 
              color: 'var(--text-secondary)', 
              fontSize: '14px', 
              fontWeight: '600',
              marginBottom: '8px'
            }}>
              Test Steps ({scenario.steps.length})
            </h4>
            <div className="steps-container">
              {scenario.steps.map((step, index) => (
                <div
                  key={index}
                  className="step-item"
                  style={{
                    background: 'var(--bg-secondary)',
                    border: '1px solid var(--border-color)',
                    borderRadius: '6px',
                    padding: '12px',
                    marginBottom: '8px'
                  }}
                >
                  <div className="step-header" style={{ marginBottom: '8px' }}>
                    <span style={{
                      background: 'var(--color-primary)',
                      color: 'white',
                      padding: '2px 6px',
                      borderRadius: '3px',
                      fontSize: '11px',
                      fontWeight: '600',
                      marginRight: '8px'
                    }}>
                      {step.step_type}
                    </span>
                    <span style={{ 
                      color: 'var(--text-primary)', 
                      fontSize: '14px',
                      fontWeight: '500'
                    }}>
                      {step.description}
                    </span>
                  </div>
                  
                  {step.pin_interactions && step.pin_interactions.length > 0 && (
                    <div className="step-pins" style={{ marginTop: '8px' }}>
                      <span style={{ 
                        color: 'var(--text-secondary)', 
                        fontSize: '12px',
                        marginRight: '8px'
                      }}>
                        Pin Interactions:
                      </span>
                      {step.pin_interactions.map((pin, pinIndex) => (
                        <span
                          key={pinIndex}
                          style={{
                            background: 'var(--bg-tertiary)',
                            color: 'var(--text-primary)',
                            padding: '2px 6px',
                            borderRadius: '3px',
                            fontSize: '11px',
                            fontFamily: 'monospace',
                            marginRight: '4px',
                            border: '1px solid var(--border-color)'
                          }}
                        >
                          {pin}
                        </span>
                      ))}
                    </div>
                  )}
                </div>
              ))}
            </div>
          </div>

          {/* Status Information */}
          <div className="detail-section">
            <h4 style={{ 
              color: 'var(--text-secondary)', 
              fontSize: '14px', 
              fontWeight: '600',
              marginBottom: '8px'
            }}>
              Status Information
            </h4>
            <div style={{
              background: 'var(--bg-secondary)',
              border: '1px solid var(--border-color)',
              borderRadius: '6px',
              padding: '12px'
            }}>
              <div style={{ marginBottom: '8px' }}>
                <span style={{ color: 'var(--text-secondary)', fontSize: '12px' }}>
                  Current Status:
                </span>
                <span style={{ 
                  color: 'var(--text-primary)', 
                  fontSize: '14px',
                  fontWeight: '600',
                  marginLeft: '8px'
                }}>
                  {scenario.status || 'Not Run'}
                </span>
              </div>
              <div>
                <span style={{ color: 'var(--text-secondary)', fontSize: '12px' }}>
                  Total Steps:
                </span>
                <span style={{ 
                  color: 'var(--text-primary)', 
                  fontSize: '14px',
                  fontWeight: '600',
                  marginLeft: '8px'
                }}>
                  {scenario.steps.length}
                </span>
              </div>
            </div>
          </div>
        </div>

        <div className="modal-footer">
          <button
            onClick={onClose}
            style={{
              background: 'var(--color-primary)',
              color: 'white',
              border: 'none',
              borderRadius: '6px',
              padding: '8px 16px',
              fontSize: '14px',
              fontWeight: '500',
              cursor: 'pointer'
            }}
          >
            Close
          </button>
        </div>
      </div>
    </div>
  )
}
