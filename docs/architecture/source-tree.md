# Multi-Sonicator I/O Source Tree

Purpose: Canonical map of the repository layout, responsibilities, and key entrypoints. Use this as the authoritative overview when navigating or extending the project. For pin assignments and hardware I/O, the single source of truth is `config/hardware-config.yaml`.

## Top-Level Layout

```
multi-sonicator-io/
├─ .agile-flow/                 # Agile Flow™ extension config (flow-config, SOP wiring)
├─ .bmad-core/                  # BMAD Core configs, reusable tasks/checklists/templates
├─ .github/                     # CI, chatmodes, and GitHub metadata
├─ acceptance-junit/            # Acceptance test JUnit reports (artifacts)
├─ config/                      # Project configuration (SOLE SOURCE for hardware pins)
├─ coverage/                    # Coverage artifacts and HTML reports
├─ docs/                        # Project documentation (architecture, sop, agile, reports)
├─ include/                     # Public headers shared across firmware modules
├─ integration-junit/           # Integration test JUnit reports (artifacts)
├─ scripts/                     # Automation: CI helpers, validation, reporting, HIL tools
├─ src/                         # Firmware source (PlatformIO/Arduino for ATmega32A)
├─ test/                        # Tests: unit (Unity C++), HIL, integration, acceptance
├─ web-ui/                      # Simple web UI stack (frontend/backend) for DUT testing
├─ Makefile                     # Standard entrypoints: build/test/docs/report
├─ platformio.ini               # PlatformIO environment configuration
└─ requirements.txt             # Python tooling dependencies for scripts/tests
```

Key references:
- Hardware pins and signal mapping: `config/hardware-config.yaml` (SOLE SOURCE OF TRUTH)
- SOP index: `docs/sop/README.md`
- Core config: `.bmad-core/core-config.yaml`

## `/src` — Firmware (PlatformIO / Arduino)

Entry point and modules for the ATmega32A firmware.

- `src/main.cpp` — Setup/loop, wires HAL, Modbus, register map, and controllers.
- `src/modules/` — Layered components:
  - `hal/` — GPIO, ADC, PWM, UART, timer abstractions and convenience helpers.
  - `communication/` — Modbus RTU, register/mapping, CRC, timeouts.
  - `control/` — Sonicator domain logic and orchestration.
- `src/compat/` — Build-target compatibility shims and native test Arduino stubs.

Headers shared externally live in `include/` and should be documented with doxygen-style comments per SOP.

## `/test` — Testing Layout

Organized to support fast unit tests and higher-level acceptance/HIL flows.

- `test/unit/` — C/C++ unit tests (Unity). Aim for ≥85% statement coverage for new code.
- `test/hil/`, `test/integration/`, `test/acceptance/` — Higher levels as applicable.
- `test/test_comprehensive_coverage.cpp` — Coverage harness aggregation.
- `acceptance-junit/`, `integration-junit/` — JUnit XML outputs (report artifacts).

Run tests exclusively via Make targets as defined in `docs/sop/sw-testing.md` and the `Makefile`.

## `/scripts` — Automation & Tooling

Python utilities for CI, reporting, validation, HIL orchestration, QA, and compliance checks.

- Examples: `generate_coverage_report.py`, `unity_coverage_runner.py`, `validate_project_structure.py`, `hil_cli.py`, `qa_cli.py`.
- Do not edit lockfiles or package configs by hand; use the package manager (`pip`, `poetry`, etc.) per SOP.

## `/docs` — Documentation

Structured documentation with clear indices per SOP.

- `docs/architecture/` — Architecture overview, tech stack, and this source tree.
- `docs/sop/` — Canonical company standards; treat these as the single source for process.
- `docs/agile/` — Epics/stories/boards for development workflow.
- `docs/agent-reports/` — Generated or curated reports for stakeholders.

Authoritative indices:
- Human index: `docs/sop/README.md`
- Machine index: `.agile-flow/flow-config.yaml` → `standard_operating_procedures`

## `/config` — Configuration

- `hardware-config.yaml` — Canonical pin/connector/signal matrix and test capabilities.
  - Any change here must be reflected in `include/system_config.h` and HIL wrapper constants (see actions_on_update in the YAML).

## `/web-ui` — Test UI for DUT

Lightweight web stack to drive/observe the ATmega32A Sonicator Multiplexer for HIL/acceptance testing.

- `web-ui/backend/`, `web-ui/frontend/` — Service and UI code.
- `web-ui/docker-compose*.yml` — Local orchestration for the web UI stack.

## Build & Tooling Entry Points

- `Makefile` — Primary interface for build, unit tests, acceptance tests, coverage, docs.
- `platformio.ini` — PlatformIO environments and board/framework configuration.

## Conventions & Notes

- Single Responsibility: Keep modules focused; share logic via `modules/` and `include/`.
- DRY: Search for existing helpers before adding new ones (see `scripts/`, `modules/`).
- Documentation: Keep doxygen comments up to date on public headers; update relevant docs when structure changes.
- Quality Gates: Never deliver untested functionality; enforce ≥85% statement coverage for new features; run tests via Make targets only.
- Hardware Mapping: Always reference `config/hardware-config.yaml` for any pin/connector logic.

