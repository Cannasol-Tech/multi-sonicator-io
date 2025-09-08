# Axovia Flow AI Coding Agent Initialization Checklist

Purpose: Verify an AI agent has correctly loaded and is adhering to project configurations and standards.

## Preconditions
- [ ] `.bmad-core/core-config.yaml` exists
- [ ] `.axovia-flow/flow-config.yaml` exists
- [ ] `AGENTS.md` exists
- [ ] `.axovia-flow/company-standards/` accessible

## Load Order and Merge
- [ ] Agent read `.bmad-core/core-config.yaml` first
- [ ] Agent read `.axovia-flow/flow-config.yaml` second
- [ ] Extension values override core unless core marks non-overridable

## Resolved Settings Confirmed to User
- [ ] story_root/devStoryLocation
- [ ] qa_root/qa.qaLocation
- [ ] architecture mode (sharded vs monolithic) and locations
- [ ] any always-load paths (e.g., devLoadAlwaysFiles or context_requirements.always_load)
- [ ] VITE_API_BASE (if relevant)

## Behavior and Safety
- [ ] Agent uses config-driven discovery for stories/PRD/QA/architecture
- [ ] Agent halts and reports precisely if required inputs are missing
- [ ] Agent uses package managers for dependency changes
- [ ] Agent avoids deployments/external writes without explicit permission
- [ ] Agent presents choices as numbered lists

## Slash Commands and Contracts
- [ ] Agent is aware of AGENTS.md slash commands
- [ ] Agent respects UI→API contract `{ images: [{ url }] }` unless configs override

## Evidence Collected (paste or link)
- [ ] Loaded file list
- [ ] Extracted key settings
- [ ] Any inconsistencies and next steps

## Result
- [ ] PASS — Agent adheres to configuration and standards
- [ ] CONCERNS — Some gaps; remediation planned
- [ ] FAIL — Agent not aligned; must fix before proceeding

