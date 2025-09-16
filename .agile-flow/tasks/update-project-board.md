---
__metadata__:
   title: Update Project Board
   author: Stephen Boyett
   company: Axovia AI
   version: 1.0
   brief: Update the Agile Project Board (docs/agile/planning-board.yaml) with latest epic/feature/story statuses
   date: 2025-09-14
   tags:
   - agile
   - planning
   - board
   - update
   - task
---

# Task: Update Project Board
> *~ Axovia AI ~•Axovia•ƒløw™•~ Agentic Agile Standard Operating Procedure ~*

Purpose: Keep the single source of truth in `docs/agile/planning-board.yaml` in sync with current epic/feature/story statuses and sprint planning decisions. 

Operator: Scrum Master (sm)

## Project Board Format
> *~ Axovia AI ~•Axovia•ƒløw™•~ Agentic Agile Standard Operating Procedure ~*

## Inputs
> *~ Axovia AI ~•Axovia•ƒløw™•~ Agentic Agile Standard Operating Procedure ~*
- None: If no inputs are supplied, check all stories in `docs/agile/stories` and update the board to be consistent with the stories latest update. 

## Process
> *~ Axovia AI ~•Axovia•ƒløw™•~ Agentic Agile Standard Operating Procedure ~*
1) Load `docs/agile/planning-board.yaml`.
2) Note the last updated time
3) Check all stories in `docs/agile/stories` and sort by last updated time.
4) For each story, check if the story has been updated since the last update.  If so, update the board to be consistent with the story.
5) Write back the file and emit a short summary.

## Acceptance Notes
> *~ Axovia AI ~•Axovia•ƒløw™•~ Agentic Agile Standard Operating Procedure ~*

- After update, the board file remains consistent with it's format and keeps prior items intact.
- All Stories are included that are in `docs/agile/stories`.
- All Stories are aligned between the project board and the stories for all information displayed on the project board.
- Changes include a timestamped `last_updated` field. 
- The `last_updated` field is in the format of `YYYY-MM-DD HH:MM:SS`.
- You have double checked your work.
