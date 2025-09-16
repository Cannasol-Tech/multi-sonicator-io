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

Purpose: Update the markdown tables in `docs/agile/project-board.md` to reflect current Epic/Story statuses, progress, QA gates, and related notes. This task operates only on the repository markdown file (no GitHub API sync).

Operator: Scrum Master (sm)

Source of Truth: `docs/agile/project-board.md` (authoritative project dashboard)

---
Scope
- Update rows for specific stories by ID across Epic sections.
- Preserve existing table structure and column order in each section.
- Add or update: Status, Progress, QA Gate, QA Gate Timestamp, Assessment, Blocking Issues/Dependencies, and optionally Assignee in notes.
- Do not add/remove stories or restructure epics without explicit user approval.

Inputs (Required)
- Primary data source: Story files under `docs/agile/stories/` (alias: `@docs/stories/`).
- Optional overrides: You may still provide a YAML list (schema below) to override auto-extracted values per story.

Overrides Schema (optional)
- Use when you need to override or supplement what’s parsed from stories:
  - `id` (string, e.g., `0.2`, `3.1`)
  - `status` (enum): `NOT STARTED` | `DRAFT` | `IN PROGRESS` | `REVIEW` | `READY FOR REVIEW` | `COMPLETED` | `BLOCKED`
  - `progress` (percentage string, e.g., `75%`)
  - `qa_gate` (enum): `CONCERNS` | `READY FOR REVIEW` | `BLOCKED` | `PASS` | `-`
  - `assessment` (short text)
  - `blocking_issues` or `dependencies` (short text)
  - `assignee` (optional)

Input Example (pasteable list):

```yaml
- id: 0.2
  status: IN PROGRESS
  progress: 75%
  qa_gate: CONCERNS
  assessment: Unit test coverage improving; target ≥85% by Wed
  blocking_issues: -
  assignee: @dev1
- id: 3.1
  status: REVIEW
  progress: 30%
  qa_gate: READY FOR REVIEW
  assessment: Core layout done; awaiting API stubs
  blocking_issues: Depends on 3.0 QA
  assignee: @dev2
```

---
Table Conventions
- Story ID cell may be bolded (e.g., `**0.2**`); match by ID ignoring markdown formatting and whitespace.
- Progress values end with `%` (e.g., `70%`).
- QA Gate column displays one of: `⚠️ CONCERNS`, `🟢 READY FOR REVIEW`, `🔒 BLOCKED`, `✅ PASS`, or `-` if not set. When provided in input as plain words, map to icons:
  - `CONCERNS` → `⚠️ CONCERNS`
  - `READY FOR REVIEW` → `🟢 READY FOR REVIEW`
  - `BLOCKED` → `🔒 BLOCKED`
  - `PASS` → `✅ PASS`
- Set `QA Gate Timestamp` to current ISO timestamp if `qa_gate` is provided and not `-`.
- Some Epic sections use `Blocking Issues`; others use `Dependencies`. Update the one that exists for that section; if neither exists, append brief note to `Assessment`.

---
Automatic Extraction Rules
1) Story discovery
   - Scan `docs/agile/stories/` for `*.md` files excluding dashboards/indices.
   - Derive story id from filename pattern `<n>.<n>` (e.g., `0.2`), or from frontmatter `id: S-<n>.<n>` if present.

2) Title
   - Prefer first H1 matching `# Story <id>: <Title>` → capture `<Title>`.
   - Fallback: frontmatter `title:`.

3) Status (normalized)
   - Priority order:
     a) Line starting with `STORY STATUS:` (e.g., `STORY STATUS: Ready for Review — CONCERNS IDENTIFIED`).
     b) The first non-empty line under a `## Status` (or `## Statusq`) section.
     c) Frontmatter `status:` value.
   - Normalize to board status: map case-insensitively:
     - `backlog|draft` → `DRAFT`
     - `in progress|wip|doing|testing` → `IN PROGRESS`
     - `review` → `REVIEW`
     - `ready for review|rfr` → `READY FOR REVIEW`
     - `done|complete|completed` → `COMPLETED`
     - `blocked` → `BLOCKED`
     - otherwise → `NOT STARTED`.

4) Progress
   - Look for `Overall Completion: ~<num>%` or `<num>%` near a `Progress` or `Completion` indicator.
   - If status is `COMPLETED` set to `100%`.
   - If no signal found and status is `DRAFT/NOT STARTED` set `0%`.
   - Otherwise keep existing board value if present; fallback `0%`.

5) QA Gate
   - Priority order:
     a) Line starting with `Gate:` (e.g., `Gate: CONCERNS`).
     b) From `STORY STATUS: ... — <QA qualifier>` if contains `CONCERNS|BLOCKED|READY FOR REVIEW|PASS`.
   - Map to icons per Table Conventions; `-` if absent.

6) QA Gate Timestamp
   - Prefer a `Review Date:` ISO-like date if present.
   - Else use frontmatter `updated_at:`.
   - Else set to current ISO timestamp at update time if `qa_gate` is set.

7) Blocking Issues / Dependencies
   - Extract lines beginning with `Depends on` or similar markers.
   - If none, set to `-`.

8) Assessment
   - Prefer a concise summary from any `### Current Status Assessment` or nearby bullet points.
   - If not found, set to `N/A`.

Workflow
1) Build update set from stories
   - Parse all stories using the rules above to produce a computed list `{id, title, status, progress, qa_gate, qa_gate_timestamp, assessment, blocking_issues/dependencies}`.
   - Apply optional YAML overrides by matching on `id` and replacing provided fields.

2) Dry-run preview
   - Prepare proposed row edits per story ID across the relevant Epic tables.
   - Preserve all untouched content and formatting.
   - Present a unified diff of `docs/agile/project-board.md` for review.

3) Apply changes
   - Only after explicit user approval of the diff, write updates to `docs/agile/project-board.md`.

4) Verify rendering
   - Re-open the file, confirm tables are well-formed (header + delimiter rows intact).
   - Spot-check updated rows for correct emoji/status mapping and timestamps.

---
Acceptance Criteria
- The specified story rows are updated in `docs/agile/project-board.md` with correct columns for their section.
- QA Gate icons and timestamps are accurate per inputs.
- Tables render correctly as GitHub-flavored markdown.
- No unrelated content changes.

Safety & Notes
- Always show a diff before writing. Require explicit user approval.
- Never reorder or remove rows unless explicitly requested.
- If both `blocking_issues` and `dependencies` are provided, prefer the section’s column; include the other in `Assessment` as a note.
