---
description: Greenfield Service/API Development (from .bmad-core/workflows/greenfield-service.yaml)
---

Build a new backend service/API from concept to development readiness.

1. __Create project brief (Analyst)__ → `docs/project-brief.md`
   - Optional: brainstorming and market research.

2. __Create PRD (PM)__ → `docs/prd.md`
   - Input: project brief. Focus on service/API requirements.

3. __Create service architecture (Architect)__ → `docs/architecture.md`
   - Input: PRD. Optional technical research.

4. __Update PRD if architecture changes stories (PM)__
   - If needed, update and re-export PRD to `docs/prd.md`.

5. __Validate artifacts (PO)__
   - Use master checklist across PRD and Architecture.

6. __Shard documents (PO)__
   - Produce `docs/prd/` and `docs/architecture/` folders for story creation.

7. __Create next story (SM)__
   - Input: sharded docs. Output: Draft story.

8. __(Optional) Review and approve story (Analyst/PM)__
   - Draft → Approved.

9. __Implement story (Dev)__
   - Code + tests; update File List; mark Ready for Review.

10. __(Optional) QA review (QA)__
    - Refactor small issues; checklist for remaining.

11. __Address QA feedback (Dev)__
    - Resolve unchecked items.

12. __Repeat for all stories__

13. __(Optional) Epic retrospective (PO)__

14. __Complete__ → Service development phase complete.
