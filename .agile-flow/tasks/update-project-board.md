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
# Task: Update Project Board

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

**Process**
1) Load `config/planning-board.yaml`.
2) Ensure sprint exists; create it if missing with default columns.
3) Upsert items, preserving history and timestamps.
4) Write back the file and emit a short summary.

Acceptance Notes
- After update, the board file remains valid YAML and keeps prior items intact.
- Changes include a timestamped `last_updated` field.

