---
description: Greenfield Full-Stack Application Development (from .bmad-core/workflows/greenfield-fullstack.yaml)
---

This workflow helps build a new full‑stack application from concept through planning to development readiness, mapping the BMad sequence into Windsurf steps.

1. __Create project brief (Analyst)__
   - Produce `docs/project-brief.md` capturing goals, scope, constraints.
   - Optional: run brainstorming and market research first.

2. __Create PRD (PM)__
   - Input: `docs/project-brief.md`
   - Output: `docs/prd.md` using your PRD template.

3. __Create Frontend Spec (UX)__
   - Input: `docs/prd.md`
   - Output: `docs/front-end-spec.md` with UI/UX requirements and flows.

4. __(Optional) Generate AI UI prompt (UX)__
   - Input: `docs/front-end-spec.md`
   - Output: Prompt for v0/Lovable/etc. If used, download UI project structure.

5. __Create Fullstack Architecture (Architect)__
   - Inputs: `docs/prd.md`, `docs/front-end-spec.md` (and generated UI structure if any)
   - Output: `docs/fullstack-architecture.md`

6. __Update PRD if architecture changes stories (PM)__
   - If the architecture suggests PRD changes, update and re-export `docs/prd.md`.

7. __Validate artifacts (PO)__
   - Use a master checklist. Ensure consistency across PRD, Frontend Spec, Architecture.

8. __Shard documents for IDE development (PO)__
   - Create `docs/prd/` and `docs/architecture/` from the consolidated docs to support story creation.

9. __Create next story (SM)__
   - Input: sharded docs
   - Output: story in `docs/agile/stories/` (or your chosen location), status Draft.

10. __(Optional) Review and approve story (Analyst/PM)__
   - Update status: Draft → Approved.

11. __Implement story (Dev)__
   - Build feature and tests; update File List; mark Ready for Review when complete.

12. __(Optional) QA review (QA)__
   - Senior dev-style review; small fixes inline; checklist for remaining items.

13. __Address QA feedback (Dev)__
   - Fix remaining items and return for final approval as needed.

14. __Repeat for all stories__
   - Continue SM → Dev → QA loop until epic completion.

15. __(Optional) Epic retrospective (PO)__
   - Document learnings and improvements.

16. __Complete__
   - All stories implemented and reviewed; development phase complete.
