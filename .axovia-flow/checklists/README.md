# Axovia Flow Checklists

A comprehensive checklist system for the Axovia Flow Agentic coding framework, designed to ensure quality, consistency, and completeness across all development workflows.

## Overview

The Axovia Flow Checklists provide structured quality gates and verification steps for:
- Code development and review processes
- Multi-agent collaboration workflows
- Testing and quality assurance
- Deployment and release management
- Architecture and design validation

## Structure

```
.axovia-flow/checklists/
├── README.md                    # This file
├── development/                 # Development workflow checklists
│   ├── code-review.md          # Code review checklist
│   ├── feature-completion.md   # Feature completion verification
│   └── refactoring.md          # Refactoring safety checklist
├── testing/                    # Testing and QA checklists
│   ├── unit-testing.md         # Unit test coverage checklist
│   ├── integration-testing.md  # Integration test checklist
│   └── acceptance-testing.md   # User acceptance test checklist
├── deployment/                 # Deployment and release checklists
│   ├── pre-deployment.md       # Pre-deployment verification
│   ├── production-ready.md     # Production readiness checklist
│   └── rollback.md            # Rollback procedure checklist
├── architecture/               # Architecture and design checklists
│   ├── design-review.md        # Architecture design review
│   ├── security-review.md      # Security assessment checklist
│   └── performance-review.md   # Performance validation checklist
└── collaboration/              # Multi-agent workflow checklists
    ├── handoff.md             # Agent-to-agent handoff checklist
    ├── task-completion.md     # Task completion verification
    └── knowledge-transfer.md   # Knowledge transfer checklist

## How to Follow a Checklist

This section defines the standard operating procedure (SOP) for using any checklist under `.axovia-flow/checklists/`.

### 1) Identify and Select the Correct Checklist(s)
- Map your current activity to one or more checklist categories: `development/`, `testing/`, `deployment/`, `architecture/`, `collaboration/`.
- For multi-phase tasks, select all applicable checklists (e.g., `development/feature-completion.md` + `testing/integration-testing.md`).
- If no checklist fits, propose a new one via PR using the template in this README (see Conventions below).

### 2) Initialize the Checklist Execution
- Create a work item in your PR description or issue that references the specific checklist files.
- Record metadata at the top: owner(s), date started, linked PR/issue, environment/branch, and definition of done.
- Optionally copy the checklist into the PR description using collapsible sections for visibility during review.

### 3) Execute Each Checklist Item
- Work top-to-bottom. For each item:
  - Mark status as Pending/In Progress/Completed.
  - Add evidence: links to commits, test runs, artifacts, logs, screenshots, or architecture docs.
  - Note blockers and decisions with rationale.
- Keep items granular—if an item is too broad, split it and reference the sub-items.

### 4) Quality Gates and Fail-Fast Rules
- Any checklist item marked as a Gate must pass before proceeding (e.g., security approvals, test coverage thresholds).
- If a gate fails:
  - Stop work on dependent steps.
  - Open a remediation sub-task.
  - Document the failure mode and proposed fix.

### 5) Verification and Peer Review
- Request peer review specifically against the referenced checklist(s).
- Reviewers verify evidence and leave explicit sign-off comments per section.
- For multi-agent workflows, each responsible role must sign off their segment (see Roles below).

### 6) Finalize and Archive
- Upon completion:
  - Ensure all items are Checked/Completed and evidence is linked.
  - Summarize deviations and waivers (if any) with justification and approver.
  - Link the completed checklist state in the PR body; merge only after approvals.
- Archive by leaving the completed checklist within the PR description and tagging the release notes if relevant.

---

## Checklist Lifecycle
- Draft: Proposed or in-progress checklist content.
- Adopted: Approved and in use for projects.
- Versioned: Breaking changes increment major; minor for additions; patch for wording fixes.
- Deprecated: Replaced by a newer checklist; keep for audits.
- Retired: No longer applicable; keep a link with rationale.

Version each checklist with a `Version:` header and `Changelog:` section at the bottom of the file.

---

## Roles and Responsibilities
- Author/Implementer: Executes items, attaches evidence.
- Reviewer: Verifies evidence, requests changes, signs off.
- Security/Architecture SME: Approves gated sections in `security-review.md` and `design-review.md`.
- Release Manager: Confirms deployment gates and rollback readiness.
- Orchestrator (Multi-Agent): Coordinates handoffs using `collaboration/handoff.md` and ensures `task-completion.md` is satisfied before next agent starts.

---

## Evidence and Artifacts (What to Attach)
- Code references: commit hashes, PR links, diffs.
- Test proof: CI run links, coverage reports, screenshots of passing suites.
- Architecture: diagrams, ADRs, sequence diagrams.
- Security: dependency scan results, SAST/DAST summaries, threat model notes.
- Performance: benchmark charts, profiling outputs.
- Deployment: release checklist, environment configs, canary results, rollback plans.

Store large artifacts in approved storage and link; do not embed binaries in the repo.

---

## Automation Hooks
- CI can enforce presence of required checklist references in PRs.
- Lint rules can ensure evidence links for gated items.
- Pre-merge checks can parse checklists to verify all gates are complete.
- Templates can auto-insert relevant checklist sections into new PRs.

---

## Conventions and Templates
- Each checklist file should include:
  - Title, Purpose, Scope, Version, Maintainers
  - Prerequisites
  - Step-by-step items with clear pass criteria and gates
  - Evidence examples
  - Waiver policy
  - Changelog
- Use consistent statuses: `Pending`, `In Progress`, `Completed`, `Waived (with approval)`.

---

## Example: Development Feature Flow
1. Select `development/feature-completion.md` and `testing/unit-testing.md`.
2. Initialize in the PR body with owner, scope, DoD, and links.
3. Implement feature; attach commit links and unit test evidence.
4. Run CI; ensure coverage threshold met; attach coverage report.
5. Peer review against checklist; address feedback.
6. Final sign-offs; merge.

## Example: Multi-Agent Handoff
1. Agent A completes implementation and satisfies `collaboration/task-completion.md`.
2. Agent A uses `collaboration/handoff.md` to transfer context, decisions, and artifacts to Agent B.
3. Agent B acknowledges receipt and continues with `testing/integration-testing.md`.

---

## FAQ
- What if an item is not applicable? Mark as `N/A` with justification and reviewer approval.
- What if a checklist is missing an item? Propose an update via PR and reference it in your current work as a temporary note until merged.
- Can I skip a gate? No; use the waiver policy with explicit approval from the responsible SME.
