---
title: "QA Concerns: Story 0.4 — Hardware Connection Verification"
labels: ["qa", "concerns", "story:0.4", "hardware", "hil"]
assignees: []
---

Summary
- Gate: DEPENDENCY_BLOCKED (HIL waived) — see docs/qa/gates/0.4-hardware-connection-verification.yml
- TODO checklist: docs/qa/gates/0.4-hardware-connection-verification.TODO.md
- Story: docs/agile/stories/0.4.hardware-connection-verification.md

Scope
- Finish non‑hardware deliverables and CI preparation while hardware is unavailable.

Checklist (see TODO for details)
- [ ] Pin matrix validator in CI; report artifact generated
- [ ] Pre‑HIL scripts/docs scaffolded; Make targets added
- [ ] Interface test outlines with mocks/sim where possible
- [ ] Power/safety test plans documented
- [ ] Traceability to `config/hardware-config.yaml` ensured; report templates prepared

References
- Gate: docs/qa/gates/0.4-hardware-connection-verification.yml
- TODO: docs/qa/gates/0.4-hardware-connection-verification.TODO.md
- SOPs: docs/sop/sw-testing.md, docs/sop/release-format.md

Create on GitHub
- gh issue create \
    --title "QA Concerns: Story 0.4 — Hardware Connection Verification" \
    --label qa,concerns,story:0.4,hardware,hil \
    --body-file docs/qa/issues/STORY-0.4-QA-CONCERNS.md

