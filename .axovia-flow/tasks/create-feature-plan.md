<!-- 
-----------------
# Author: Axovia AI
# Company: Axovia AI
# Date: 2025-09-07
# Brief: Executable workflow to produce an implementation plan from a Feature Specification.
# Version: 1.0
-----------------
 -->

# Create Feature Implementation Plan Task

Purpose
- Create a practical, phased implementation plan derived from a Feature Specification, aligning scope to milestones, dependencies, and risks.

Pre‑requisites
- Target Feature Specification present in `docs/agile/features/`.
- Access to implementation plan template: `docs/planning/implementation-plan-tmpl.yaml` (or fallback: `.axovia-flow/templates/implementation-plan-tmpl.yaml`).
- Review standards in `.axovia-flow/company-standards/` for handoffs, testing, and quality gates.

Execution Rules
- Keep one source of truth for plan details; avoid duplication across docs.
- Clearly mark assumptions, constraints, and open questions.
- Reference all upstream requirements and dependencies.

1) Ingest Feature Specification
- Extract: Feature ID, scope (in/out), acceptance criteria, dependencies.
- Identify cross‑team impacts and integration points.

2) Decompose into Milestones
- Define 2–5 milestones delivering incremental value.
- For each milestone: objectives, entry/exit criteria, acceptance checks.

3) Work Breakdown Structure (WBS)
- Break milestones into tasks by domain (backend, frontend, infra, QA, docs).
- Provide size/complexity placeholders for estimation.
- Identify parallelizable vs. blocking tasks.

4) Risks, Assumptions, Dependencies
- List top risks with mitigation/contingency.
- Explicit assumptions and how to validate them.
- Concrete dependencies (internal/external) with owners.

5) Testing & Quality Gates
- Map acceptance criteria to test plans (unit, integration, acceptance).
- Add quality gates per company testing standard.

6) Handoffs & Coordination
- Note where handoffs are required (Dev→QA, etc.) and artifacts needed.
- Align with the Multi‑Agent Collaboration Standard handoff protocol.

7) Produce Plan Using Template
- Populate `implementation-plan-tmpl.yaml` with the above sections.
- Output to `docs/planning/implementation-plan.md` (or feature‑specific variant if needed).

8) Review & Finalize
- Sanity‑check for completeness and clarity.
- Share for review and incorporate feedback.

Deliverables
- Updated `docs/planning/implementation-plan.md` reflecting this feature’s plan.
- Links back to the Feature Specification and created user stories.

Test Plan
- Run docs checks (if configured) and validate links/anchors.
- Ensure gates and test mapping are present and actionable.

