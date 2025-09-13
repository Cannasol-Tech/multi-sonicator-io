---
description: Greenfield UI/Frontend Development (from .bmad-core/workflows/greenfield-ui.yaml)
---

Build a new frontend application from concept to development readiness.

1. __Create project brief (Analyst)__ → `docs/project-brief.md`
   - Optional: brainstorming, market research.

2. __Create PRD (PM)__ → `docs/prd.md`
   - Input: project brief. Focus on UI/frontend requirements.

3. __Create Frontend Spec (UX)__ → `docs/front-end-spec.md`
   - Input: PRD. Optional: user research.

4. __(Optional) Generate AI UI prompt (UX)__
   - Input: Frontend Spec. Output: prompt for v0/Lovable; download UI project if used.

5. __Create Frontend Architecture (Architect)__ → `docs/front-end-architecture.md`
   - Inputs: Frontend Spec (+ generated UI structure if any). Optional technical research.

6. __Update PRD if architecture changes stories (PM)__

7. __Validate artifacts (PO)__

8. __Shard documents (PO)__ → `docs/prd/`, `docs/architecture/`

9. __Create next story (SM)__ → Draft story from sharded docs

10. __(Optional) Review and approve story (Analyst/PM)__ → Draft → Approved

11. __Implement story (Dev)__ → Code + tests; mark Ready for Review

12. __(Optional) QA review (QA)__ → Review; checklist

13. __Address QA feedback (Dev)__

14. __Repeat for all stories__

15. __(Optional) Epic retrospective (PO)__

16. __Complete__ → Project development phase complete.
