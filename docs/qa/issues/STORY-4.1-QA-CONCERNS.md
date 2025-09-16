---
title: "QA Concerns: Story 4.1 — Multi‑Unit State Management"
labels: ["qa", "concerns", "story:4.1", "testing", "embedded"]
assignees: []
---

Summary
- Gate: CONCERNS (HIL temporarily waived) — see docs/qa/gates/4.1-multi-unit-state-management.yml
- TODO checklist: docs/qa/gates/4.1-multi-unit-state-management.TODO.md
- Story: docs/agile/stories/4.1.multi-unit-state-management.md

Scope
- Finish all non‑hardware items. Keep HIL scenarios @pending with TODO notes until hardware is available.

Checklist
- [ ] AC‑5/6 — Implement EEPROM persistence + safe recovery; add unit + integration tests; recovery BDD active
- [ ] AC‑7/10 — Implement E‑STOP and safety interlocks; add unit + integration tests (HIL deferred)
- [ ] AC‑13 — Activate and pass partial‑success BDD scenarios (remove @pending)
- [ ] AC‑8/9 — Add transition coverage and ≤10ms timing assertions; publish CI artifacts
- [ ] AC‑11 — Verify previous‑state RO mapping via integration tests; ensure register addresses match include/register_map.h
- [ ] CI & Coverage — Ensure make test-unit and make test-acceptance pass; ≥85% coverage; reports per SOP

References
- Gate file: docs/qa/gates/4.1-multi-unit-state-management.yml
- TODO file: docs/qa/gates/4.1-multi-unit-state-management.TODO.md
- Feature: test/acceptance/features/multi_unit_state_management.feature
- Unit: test/unit/control/test_multi_sonicator.cpp, test/unit/test_prev_state_registers.cpp
- Impl: src/modules/control/multi_sonicator.cpp
- SOPs: docs/sop/sw-testing.md, docs/sop/release-format.md

Create on GitHub
- Option A (CLI):
  gh issue create \
    --title "QA Concerns: Story 4.1 — Multi‑Unit State Management" \
    --label qa,concerns,story:4.1,testing,embedded \
    --body-file docs/qa/issues/STORY-4.1-QA-CONCERNS.md

- Option B (UI): Copy this file’s content into a new GitHub issue and apply labels.

