# Axovia Flow Prompts

This directory contains reusable prompts for initializing and aligning AI coding agents with this repository’s standards and configuration.

## Files

- `AGENT-INIT.md` — Dynamic initialization prompt that instructs any agent to:
  - Load `.bmad-core/core-config.yaml` (core) and `.axovia-flow/flow-config.yaml` (extension) in that order
  - Follow `AGENTS.md` and `.axovia-flow/company-standards/` (multi-agent standards, quality gates, testing requirements)
  - Use config-driven paths and fail safely if required inputs are missing
  - Adhere to repository slash command conventions

-  `AGENTS-GLOBAL.md` - Global AI Agent Coding instructions meant to be adhered to across all Company projects that a utilizing the Axovia Flow™ Agentic Coding Framework.  These Agent Coding Guidelines should be appended/moved to the project specific `AGENTS.md` file for ease of access and adherence.  This process should happen during the package installation process when a project is initializing the Axovia Flow™ framework. 

## Usage

- Print the prompt in your terminal:
  ```sh
  bash scripts/codex /agent-init
  ```

- Copy to clipboard (macOS):
  ```sh
  bash scripts/codex /agent-init --copy
  ```

- Save to a file for sharing:
  ```sh
  bash scripts/codex /agent-init --out /tmp/AGENT-INIT.md
  ```

- Paste the prompt into any Auggie AI Coding agent as session bootstrap text. The prompt is dynamic — it instructs the agent to read configs/guidelines each session rather than relying on hardcoded values.

## Notes

- If `.bmad-core/core-config.yaml` or `.axovia-flow/flow-config.yaml` are missing, agents should HALT and request remediation.
- When agents present choices, they should use numbered lists to allow quick selection.
- Safe-by-default: agents may lint/test/build locally, but must not install dependencies or deploy without explicit permission.

