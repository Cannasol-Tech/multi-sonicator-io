# GEMINI.md: AI-Powered Project Assistant Context

This document provides the necessary context for an AI assistant to understand and effectively contribute to the Multi-Sonicator I/O Controller project.

## Project Overview

This repository contains two main components:

1.  **Multi-Sonicator I/O Controller (Firmware):** An industrial automation solution for controlling up to four CT2000 sonicators simultaneously. It is built on an ATmega32A microcontroller and uses the Arduino framework. The controller communicates with a primary PLC/HMI system via MODBUS RTU.
2.  **Web UI (HIL Interface):** A sophisticated web-based Hardware-in-the-Loop (HIL) testing interface built with a React frontend and a Node.js backend. This allows for interactive testing and validation of the firmware.

---

## 1. Multi-Sonicator I/O Controller (Firmware)

### Technologies
*   **Language:** C++
*   **Framework:** Arduino
*   **Build System:** PlatformIO
*   **Microcontroller:** ATmega32A
*   **Communication:** MODBUS RTU

### Building and Running
The project uses a comprehensive `Makefile` to streamline all development tasks.

*   **Build Firmware:** `make build`
*   **Upload to Device:** `make upload`
*   **Clean Build Artifacts:** `make clean`

### Testing
The firmware has a dedicated testing suite:

*   **Unit Tests:** Written in C++ using the **Unity** framework. Run with `make test-unit`.
*   **Acceptance & Integration Tests:** Written in Python using the **Behave (BDD)** and **Pytest** frameworks. These tests interact with the hardware (or a simulation) to validate requirements. Run with `make test-acceptance` and `make test-integration`.

---

## 2. Web UI (HIL Interface)

### Technologies
*   **Frontend:** React, TypeScript, Vite
*   **Backend:** Node.js, Express, WebSocket
*   **Build System:** npm/vite (managed via `Makefile`)

### Building and Running
*   **Install Dependencies:** `make web-ui-install`
*   **Start Development Servers:** `make web-ui-dev`
*   **Build for Production:** `make web-ui-build`
*   **Run in Sandbox Mode (Recommended):** `make web-ui-sandbox`
    *   This all-in-one command builds and uploads the firmware, then starts the web UI and opens it in a browser.

### Testing
The Web UI has its own set of tests written in Python.

*   **Run Tests:** `make web-ui-test`
    *   These tests use **Pytest** to validate the backend API and its integration with the HIL framework.

---

## General Development Conventions

*   **Source of Truth:**
    *   **Hardware Configuration:** `config/hardware-config.yaml` is the sole source of truth for all pin assignments and hardware configurations.
    *   **Product Requirements:** `docs/prd.md` is the definitive source for all functional and non-functional requirements.
*   **Code Style:** The project enforces company-wide coding standards, which are automatically synced. Use `make check-standards` to verify compliance.
*   **CI/CD:** All code is automatically tested and built using the GitHub Actions workflow defined in `.github/workflows/ci.yml`.
*   **Documentation:** The project uses Doxygen for firmware documentation and TypeDoc for the web UI. Use `make docs-all` to generate all documentation.