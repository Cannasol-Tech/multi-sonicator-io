# Planning Definition of Done (Standard)

Authoritative Standard — Single Source of Truth

- This document governs the Planning Definition of Done (DoD) for all Axovia Flow repositories.
- Implementation work MUST NOT begin until all mandatory criteria are satisfied.

## Mandatory Phase Separation (Policy)

- No implementation may start until:
  - A HUMAN reviewer has explicitly marked the Planning DoD as COMPLETE for the story/epic
  - All required BMad agent checklists have PASSED (e.g., Architect, PM/PO, QA, as applicable)

## A) Foundation & Alignment

- [ ] Links to Business Goals and PRD shard(s) included
- [ ] Problem statement and intended outcome are clear (success criteria stated)
- [ ] Scope boundaries and out‑of‑scope notes documented

## B) Requirements & Acceptance (BDD‑ready)

- [ ] PRD requirement IDs enumerated (coverage is explicit)
- [ ] Acceptance criteria drafted in BDD style (Given/When/Then) and traceable to PRD IDs
- [ ] Non‑functional requirements (security, performance, reliability) considered where relevant

## C) Test Strategy & Traceability

- [ ] Unit test target stated: ≥ 90% line coverage
- [ ] Integration/Acceptance approach identified (E2E/API/Emulation/HIL as applicable)
- [ ] Data/fixtures plan captured (deterministic, idempotent where possible)
- [ ] Traceability plan: how BDD scenarios map to PRD IDs and where traces will be stored (docs/qa/traces)

## D) Risks, Dependencies, Assumptions

- [ ] Key risks called out with mitigations (probability × impact)
- [ ] External dependencies identified (teams, services, data, credentials)
- [ ] Assumptions explicitly noted

## E) Story Decomposition for Parallel Work

- [ ] Story is small enough or sharded into sub‑stories for parallel agents
- [ ] Ownership per shard is clear; interfaces/contracts between shards defined
- [ ] Handoffs and sequencing noted where ordering matters

## F) Repository & Standards Readiness

- [ ] Relevant standards referenced (sw‑testing‑standard.md, release‑format.md, repository‑auditing.md)
- [ ] Required templates/stubs added (e.g., test/acceptance, docs/qa/traces)
- [ ] Labels and links prepared for Issue/PR templates (epic/story checklist, PR template expectations)

## G) Exit Criteria (Mark “Ready for Dev”)

- [ ] HUMAN sign‑off recorded: Planning DoD marked COMPLETE
- [ ] All required BMad agent checklists PASSED (Architect, PM/PO, QA, etc.)
- [ ] Planning DoD satisfied and recorded in the story/epic file or planning issue
- [ ] Links gathered: PRD shards, acceptance draft, traces location, risk log
- [ ] “Ready for Dev” label applied and communicated to Dev/QA agents

## References

- .bmad-core/checklists/architect-checklist.md
- .bmad-core/checklists/pm-checklist.md
- .bmad-core/checklists/po-master-checklist.md
- .bmad-core/checklists/story-draft-checklist.md
- docs/standards/sw‑testing‑standard.md
- docs/standards/release‑format.md
- docs/standards/repository‑auditing.md
- docs/qa/README.md
