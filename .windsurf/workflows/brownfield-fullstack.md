---
description: Brownfield Full-Stack Enhancement (from .bmad-core/workflows/brownfield-fullstack.yaml)
---

Plan and execute enhancements to an existing full‑stack system.

1. __Classify enhancement (Analyst)__
   - Single story (<4h) → create single story and exit
   - Small feature (1–3 stories) → create epic and exit
   - Major enhancement → continue

2. __Check documentation (Analyst)__
   - If inadequate, run project analysis first.

3. __Project analysis (Architect)__ → `brownfield-architecture.md` (or multiple docs)

4. __Create PRD (PM)__ → `docs/prd.md` (use brownfield PRD template)

5. __Decide on architecture (PM/Architect)__
   - If needed, create architecture doc.

6. __Create architecture (Architect)__ → `docs/architecture.md`

7. __Validate artifacts (PO)__ → use checklist

8. __Fix issues if any (Various)__ → update flagged docs

9. __Shard documents (PO)__ → `docs/prd/`, `docs/architecture/`

10. __Create story (SM)__ → from sharded PRD or brownfield docs, Draft status

11. __(Optional) Review/approve story (Analyst/PM)__ → Draft → Approved

12. __Implement story (Dev)__ → implement; update File List; mark Ready for Review

13. __(Optional) QA review (QA)__ → review and checklist

14. __Address QA feedback (Dev)__

15. __Repeat for all stories__

16. __(Optional) Epic retrospective (PO)__

17. __Complete__ → Development phase complete.
