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

## Preconditions

- Confirm the repo and authentication context used by the host agent supports `githubRepo` operations.
- Read `.bmad-core/core-config.yaml` and `.agile-flow/flow-config.yaml` for any board naming conventions or labels.
- Do not make network calls until after the user approves the plan from the dry-run.

## Interaction Mode

- Default to incremental with explicit elicitation steps and confirmations.
- Offer YOLO mode (batch) if the user requests it; still perform a dry-run preview and require confirmation before applying.

## Elicitation

Prompt the user for:
1. Target project board
   - Org/user: e.g., `cannasol-technologies`
   - Project type: Classic or Projects (v2)
   - Identifier: URL or name; if ambiguous, list available boards via `githubRepo` and let the user choose.
2. Column mapping (by exact column names):
   - Backlog/Triage: new Issues without `wip` label
   - In Progress: open PRs or Issues with `in-progress`/`wip`
   - Review: open PRs with `needs-review`
   - Done: closed Issues/PRs
   - Allow custom mappings and extra columns (e.g., Blocked, QA)
3. Scope filters (optional):
   - Labels to include/exclude (e.g., `ui`, `backend`, `wontfix`)
   - Milestone or iteration selection
   - Whether to include Draft PRs

If the user is unsure, present sensible defaults and ask for confirmation.

## Plan and Dry-Run

1. Discover current state via `githubRepo`:
   - List project boards and resolve the selected board
   - Fetch columns and existing cards
   - Fetch candidate Issues/PRs per scope filters
2. Compute proposed actions:
   - New cards to create (missing on board)
   - Card moves between columns per mapping rules
   - Optional label syncs (if enabled by user)
   - No destructive deletions; avoid closing Issues/PRs from this task
3. Present a dry-run summary to the user:
   - Counts per action type and a compact table of first N items
   - Note any conflicts (e.g., unmapped column, missing permissions)
4. Ask for explicit approval: "Proceed to apply these changes? (yes/no)"

## Apply Changes (only after explicit approval)

- Execute actions using `githubRepo` operations appropriate to the project type:
  - Create missing cards for Issues/PRs
  - Move cards to target columns
  - Optionally apply labels per mapping rules
- Respect API limits and apply in small batches
- Stop and report if an operation fails; do not continue blindly

## Reporting

Generate a concise report under `docs/agent-reports/`:
- Filename: `project-board-sync-YYYYMMDD-HHMM.md`
- Include:
  - Board identifier and column mapping used
  - Scope filters
  - Dry-run summary
  - Applied actions summary (counts and sample items)
  - Any warnings, conflicts, or follow-ups

---
s
## Safety & Rollback

- This task only creates/moves cards and updates labels; it does not delete cards or close Issues/PRs.
- If the outcome is undesired, re-run with corrected mappings or move cards back via a targeted follow-up run.

---

## Notes for the Agent

- Use the `githubRepo` tool; do not implement raw API calls.
- If the board type or APIs differ, adapt using capability discovery (list boards → choose → list columns → operate).
- Adhere to the Source-of-Truth hierarchy; defer to project SOPs where relevant labels/columns are standardized.

---

Acceptance Notes
- After update, the board file remains valid YAML and keeps prior items intact.
- Changes include a timestamped `last_updated` field.
