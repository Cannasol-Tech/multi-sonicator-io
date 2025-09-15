---
__metadata:
  title: Update Project Board
  author: Stephen Boyett
  company: Axovia AI
  version: 1.0
  brief: Task to update the project board with current statuses and sprint planning decisions.
  date: 2025-09-14
  tags: 
    - "agile"
    - "project-board"
    - "update"
    - "sm"
    - "project-state"
---
# Update Project Board Task

Purpose: Keep the single source of truth in `docs/agile/project-board.md` in sync with current epic/feature/story statuses and sprint planning decisions.

Operator: Scrum Master (sm)

Inputs
- Items to update: list of `{id, status, assignee?, notes?}`
- Include QA gates and assessments in the board 

---
**Project-Board Template**:

QA Gate: Choose One: ⚠️ CONCERNS / ⚠️ READY FOR REVIEW / ⚠️ BLOCKED / ✅ PASS

```md
| Story | Title | Status | Progress | QA Gate | QA Gate Timestamp | Assessment | Blocking Issues |
|-------|-------|--------|----------|---------|-------------------|------------|-----------------|
| {story_id} | {title} | {status} | {progress} | {qa_gate} | {qa_gate_timestamp} | {assessment} | {blocking_issues} |
| {story_id} | {title} | {status} | {progress} | {qa_gate} | {qa_gate_timestamp} | {assessment} | {blocking_issues} |
| {story_id} | {title} | {status} | {progress} | {qa_gate} | {qa_gate_timestamp} | {assessment} | {blocking_issues} |
| ... | ... | ... | ... | ... | ... | ... | ... |

```
---

## Purpose

- Provide a safe, guided workflow for synchronizing a GitHub Project board with repository Issues and PRs.
- Support dry-run preview, explicit user confirmation, and then apply updates via the `githubRepo` tool.
- Honor Agile Flow and Source-of-Truth hierarchy, minimizing risk and avoiding unintended bulk changes.