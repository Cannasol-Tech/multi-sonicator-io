# Warp Terminal — Efficiency Guide (macOS)

This guide helps you get productive fast with Warp on macOS, including VS Code integration, keybindings, recommended settings, and practical tips.

## Overview

- External terminal for VS Code: Warp
- Integrated shell in VS Code: zsh
- Quick open external terminal from VS Code: Ctrl+Alt+T
- Default shell on macOS (outside VS Code): zsh, loading `~/.zshrc`

## Repository Workflows (Warp)

This repo includes Warp Workflows at `.warp/workflows.yaml`, aligned with company standards.

Add them in Warp:
- Open Warp → Command Palette (Command-K) → “Workflows” → “Add Repository Workflows…”, select this repository root, then browse/run workflows by name.

Highlights:
- Tests: Full suite, Unit, Acceptance (BDD), Integration (HIL), per Makefile
- CI flows: `ci`, `ci-test`, and local simulation
- Reports: Generate executive and coverage reports, auto-open outputs
- Firmware: Build, upload to device, serial monitor
- Web UI: Dev servers, Sandbox, Sandbox (Auto), Tests
- Standards: Sync and Check to keep `docs/standards/` canonical

## Agent Suite (tmuxp)

We provide a reproducible multi-pane “agent suite” using tmuxp, mapped to company-standard Make targets.

- Prereqs: tmux and tmuxp (installed via Homebrew)
- Config: `.tmuxp/sonicator-dev.yaml` (checked into this repo)
- Launch from Warp: Command-K → Workflows → "Dev — Agent Suite (tmuxp)"
- Re-attach later: Command-K → Workflows → "Dev — Attach Agent Suite"

Windows included
- standards: `make check-standards`, `make sync-standards`
- tests: `make test-unit`, `make test-acceptance`, `make test-integration`
- reports: `make generate-executive-report`, `make generate-coverage-report`
- firmware: `make build`, `make upload-to-device`, `make upload-harness`
- web-ui: `make web-ui-dev`, `make web-ui-test`
- sandbox: `make web-ui-sandbox`, `make web-ui-sandbox-auto`

Tips
- Use Option-Command-Arrow keys to move focus between panes.
- Close a pane or window with Command-W; re-launch via workflows if needed.
- If a pane exits immediately (command completes), re-run it with Up → Enter or type another command.

## VS Code Integration

- User settings applied:
  - `terminal.external.osxExec`: `Warp.app`
  - `terminal.integrated.defaultProfile.osx`: `zsh`
- Keybinding added (User):
  - `Ctrl+Alt+T` → `workbench.action.terminal.openNativeConsole` (opens Warp)
- Notes:
  - VS Code’s integrated terminal remains VS Code’s built‑in terminal; Warp cannot embed inside it.
  - Use the shortcut above to pop open Warp alongside VS Code when you need a full terminal.

## Keybindings — Must Know

General
- Command Palette: Command-K (Warp palette) / Command-Shift-P (macOS global/VS Code)
- Find in output: Command-F
- Zoom: Command-+ / Command-- / Command-0

Command Entry & Blocks
- Run: Enter
- Newline without run: Shift-Enter
- Rerun previous: Up/Down arrows navigate history; Enter to run
- Clear screen: Control-L

Tabs & Windows
- New tab: Command-T
- Close tab: Command-W
- Next/Previous tab: Shift-Command-] / Shift-Command-[
- New window: Command-N

Panes (Splits)
- Split right (vertical): Command-D
- Split down (horizontal): Shift-Command-D
- Focus next pane: Option-Command-Right (or arrow keys)
- Focus previous pane: Option-Command-Left
- Close focused pane: Command-W

Navigation & Editing
- Beginning/End of line: Command-Left / Command-Right
- Move by word: Option-Left / Option-Right
- Delete previous word: Option-Delete
- Cursor to start/end of block: Fn-Left / Fn-Right (or Home/End if mapped)

Selection & Copy
- Select all: Command-A
- Copy: Command-C (copies selected or last block output when selected)
- Paste: Command-V

Search & History
- Search output: Command-F
- Filter command history: Command-K → type “history”

AI & Workflows
- Open Command Palette: Command-K
- Trigger AI commands: Command-K → type “AI Command”
- Run a saved Workflow: Command-K → search workflow name

## Recommended Settings

Appearance
- Theme: pick a high-contrast theme (e.g., Tokyo Night, Dracula, or Warp Dark High Contrast)
- Font: JetBrains Mono or SF Mono; size 13–14; enable ligatures if preferred
- Cursor: Block or Beam; enable blinking for visibility

Blocks & Output
- Show timestamps and exit codes
- Enable block separators
- Enable “Copy block output”
- Optionally “Collapse successful blocks” for long sessions
- Increase scrollback for large logs (e.g., 100k lines)

Splits & Tabs
- Show pane borders and tab bar
- Enable “Reopen last session on launch”

Shell & Prompt
- Default shell: zsh (ensure `~/.zshrc` loads as usual)
- Consider Starship prompt for a compact, informative prompt (optional)

AI & Privacy
- Enable Command Palette and AI Command; review data/privacy opts
- Create Workflows for frequent tasks with variables (e.g., `git`, `docker`, `kubectl`)

## Productivity Tips

- Use blocks: Treat each command + output as a unit. Click a block to copy output quickly.
- Command Palette: Command-K to discover actions, workflows, and AI commands via fuzzy search.
- Workflows: Save common commands with placeholders (e.g., “git checkout <branch>”).
- Panes: Use vertical/horizontal splits to pin logs while working in another pane.
- History: Arrow keys navigate commands; use search to find older commands quickly.
- Session restore: Keep “Reopen last session on launch” on for continuity.

## Troubleshooting

- Warp not launching from VS Code:
  1) Check VS Code setting `terminal.external.osxExec` is `Warp.app`
  2) Try the command palette: “Open New External Terminal”
  3) Reinstall Warp cask: `brew reinstall --cask warp`

- zsh config not loading:
  - Confirm default shell: `echo $SHELL` → `/bin/zsh`
  - Ensure `~/.zshrc` exists and contains your setup
  - If using frameworks (oh-my-zsh, zinit), confirm they’re sourced in `~/.zshrc`

## What We Configured For You

- Set Warp as external terminal in VS Code
- Set zsh as the integrated default profile in VS Code
- Added VS Code keybinding: Ctrl+Alt+T → opens Warp
- Confirmed macOS default shell is zsh and `~/.zshrc` exists

---

For updates or additional customizations (themes, fonts, keybindings), open Warp → Settings, or ask to automate specific profiles.
