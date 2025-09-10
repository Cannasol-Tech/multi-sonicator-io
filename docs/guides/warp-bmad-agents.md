# Warp AI — BMAD Agents (Global Rules)

This guide adds company-standard “BMAD” agents to Warp AI as reusable system prompts, and shows how to access them globally in Warp.

## What You Get

- Global base rules for Warp AI (BMAD Global Rules)
- Specialized agents: Standards, Testing, HIL, Web UI, Release, Compliance, Repo Navigator
- Quick-open workflows to view/copy prompts

## Add Global Rules to Warp AI

Warp → Settings → AI → Custom Instructions
- Copy the content of `.warp/agents/bmad-global-rules.txt` and paste into Custom Instructions.
- These rules will apply to all Warp AI interactions.

## Add Specialized Agents

Warp → AI (Command-K → “AI Command” or Agents UI)
- Create a new Agent for each of the following and paste the corresponding prompt content:
  - Standards Agent → `.warp/agents/standards-agent.txt`
  - Testing Agent → `.warp/agents/testing-agent.txt`
  - HIL Agent → `.warp/agents/hil-agent.txt`
  - Web UI Agent → `.warp/agents/webui-agent.txt`
  - Release Agent → `.warp/agents/release-agent.txt`
  - Compliance Agent → `.warp/agents/compliance-agent.txt`
  - Repo Navigator Agent → `.warp/agents/repo-navigator-agent.txt`

Name and save each agent. They’ll be available across all tabs.

## Make Them Easy to Access

- Use the Command Palette: Command-K → type the agent name, or start with “AI Command”, then pick the agent.
- Pin favorites in Warp’s AI UI if supported in your version.

## Keep Agents Up To Date

- If company standards change or `docs/standards/` updates, update the files in `.warp/agents/` and re-paste into Warp’s settings.
- Use the Compliance Agent to check for divergence: `make check-standards`.

## Related Workflows

From this repo, quick-open the prompts:
- Command-K → Workflows → Docs — BMAD Agents Guide
- Or open any individual prompt from the “Agents — Open …” entries.

## Notes

- Warp does not currently auto-import agent prompts from files; manual copy/paste into Warp’s settings is required.
- These prompts reflect the single source of truth hierarchy:
  - `docs/standards/*` (canonical)
  - `docs/architecture/*` and PRDs
  - `AGENTS.md`, `.axovia-flow/flow-config.yaml`, `.bmad-core/core-config.yaml`
- For pin assignments and wiring, always reference `docs/planning/pin-matrix.md`.

