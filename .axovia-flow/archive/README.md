# Axovia Flow Archive

Purpose
- Holds deprecated or legacy coordination files that should not be part of the active standard or default workflow.
- Contents are git-ignored by default; only this README and the local .gitignore are tracked so the folder is present.

Usage Rules
- Do not rely on archived files as a source of truth. The canonical standard is `.axovia-flow/company-standards/multi-agent-collaboration.md` and the operational prompt is `docs/planning/multi-agent-sync.md`.
- Only use archived files when explicitly instructed by a project lead or when performing forensics/comparisons.

Populate Legacy multi-agent-sync.md Locally (if needed)
1) Retrieve the current operational prompt from the repo:
   - URL: https://github.com/Axovia-AI/axovia-flow/blob/main/docs/planning/multi-agent-sync.md
   - Download it to `.axovia-flow/archive/multi-agent-sync.md` on your machine.
2) Keep the file untracked (this folder is git-ignored); do not commit archived copies unless explicitly directed.
3) When finished, delete your local copy or leave it here for your local reference.

Authoritative References
- Standard: `.axovia-flow/company-standards/multi-agent-collaboration.md`
- Operational Prompt: `docs/planning/multi-agent-sync.md`

