# Story 1.4: Web UI for Testing & Monitoring

**Story ID**: S-1.4
**Feature**: F-002: HIL Testing Framework & Hardware Verification
**Priority**: High
**Owner**: Product Owner (Sarah)
**Created**: 2025-09-10
**Updated**: 2025-09-10

## Status

Draft

## Story

**As a** developer and test engineer,
**I want** a Web UI to visualize and control hardware testing and MODBUS registers in real-time,
**so that** I can monitor system state, drive tests manually, and validate HIL interactions quickly.

## Feature Context

**Feature F-002** establishes the HIL framework and hardware validation. This story provides a web-based interface to the HIL controller for observability and manual test workflows.

## Story Context

- Real-time display of MODBUS registers and key system signals
- Controls for starting/stopping sonicators and triggering resets
- Integration with HIL controller backend
- Support for developer workflows (dev ports 3001/3101)

## Acceptance Criteria

1. Web UI displays real-time MODBUS register values with <1s latency
2. Web UI provides start/stop/reset controls for all four sonicators (1–4)
3. UI displays status indicators for: FREQ_LOCK_1-4, OVERLOAD_1-4, and PWR_MONITOR_1-4
4. UI shows amplitude control (PB3 PWM) current value and allows safe adjustments (bounded)
5. Backend integrates with HIL controller to execute actions and stream updates
6. Dev server supports start/stop targets and uses ports 3001 (frontend) and 3101 (backend) by default
7. Errors and disconnections are surfaced clearly with retry/backoff strategy
8. UI state updates are resilient under concurrent interactions

## Technical Implementation Details

### Architecture

- Frontend: React/Vite (or existing stack) with websocket/SSE for live updates
- Backend: Node/Python adapter bridging Web UI and HIL controller
- Protocol: REST for commands, websocket/SSE for telemetry

### API Sketch

- GET /api/registers → full register snapshot
- POST /api/sonicator/{n}/start | /stop | /reset
- POST /api/amplitude → { value: <0..100> }
- GET /api/stream → SSE for register diffs and signal events

### Make Targets (dev convenience)

```bash
make web-ui-start     # start backend (3101) and frontend (3001)
make web-ui-stop      # stop any processes on :3001 and :3101
make web-ui-build     # build frontend
```

## Definition of Done

- [ ] Real-time MODBUS register dashboard (<1s latency)
- [ ] Controls for start/stop/reset for all four sonicators
- [ ] Status indicators: FREQ_LOCK, OVERLOAD, and PWR_MONITOR per channel
- [ ] Amplitude control with safe bounds and validation
- [ ] Backend bridge to HIL controller with REST+SSE
- [ ] Dev servers use ports 3001 and 3101 with start/stop targets
- [ ] Robust error handling and reconnection logic implemented
- [ ] Documented in README with quick-start instructions

## Tasks / Subtasks

- [ ] Backend bridge
  - [ ] Implement REST commands (start/stop/reset, amplitude)
  - [ ] Implement SSE/websocket telemetry stream
  - [ ] Integrate with HIL controller API

- [ ] Frontend dashboard
  - [ ] Register table with live updates and highlighting on change
  - [ ] Sonicator controls and status indicators
  - [ ] Amplitude control widget with bounds and confirmation

- [ ] DevOps
  - [ ] Implement make targets to start/stop/build web UI
  - [ ] Ensure processes bind to 3001 (frontend) and 3101 (backend)
  - [ ] Add docs and troubleshooting section

## Dev Notes

- Ensure command paths trigger actual HIL actions and the UI reflects results deterministically
- Keep UI responsive under slower hardware update intervals; debounce updates where needed
- Log client/server disconnects and provide user feedback

## Dependencies

- Story 1.2: HIL Test Harness Implementation
- Story 1.3: BDD Acceptance Testing Framework (for register map and scenarios)

## Change Log

| Date       | Version | Description                         | Author               |
|------------|---------|-------------------------------------|----------------------|
| 2025-09-10 | 1.0     | Initial story for Web UI monitoring | Product Owner (Sarah) |
