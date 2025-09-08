---
description: Agile Planning workflow - Generate feature specifications and user stories from an implementation plan and PRD
---

# Agile Planning Workflow (/agile-planning)

```yaml
mode: turbo
```

This workflow guides the AI Agentic Multi Agent Agile planning process across four phases: Knowledge Gathering, Understanding, Feature Specifications, and User Stories. It is designed to be interactive, allowing the user to provide input and receive feedback during each phase.

## Inputs

- Implementation Plan path (default): docs/planning/implementation-plan.md
- PRD path (default): docs/requirements/PRD-v1.0.0.md
- Project name: <your-project-name>

## Phase 1 — Knowledge Gathering

1. Open and review:
   - docs/planning/implementation-plan.md
   - docs/requirements/PRD-v1.0.0.md

2. Confirm sources:
   - Ask: “I think we have everything we need to move to Phase 2 of the Planning Process. Are the two files I have `<implementation-plan-file-path>` and `<PRD-plan-file-path>` the correct, up to date, official documents for the `<project-name>` project?”

3. Branching logic:
   - If YES → Move to Phase 2
   - If NO → Restart Phase 1 and acquire correct inputs
   - Else → Ask user to answer Yes or No

## Phase 2 — Analyze and Align on Features

1. Converse to fully understand PRD and plan:
   - Ensure clarity on features, dependencies, boundaries, and scope.

2. Produce numbered feature list with brief descriptions.
3. Validate with user per feature:
   - Ask: “I think I have a clear understanding of the project and the features to be implemented (list + brief descriptions). Is my understanding correct?”

4. Branching logic (per feature):
   - If YES:
     - If last feature → Move to Phase 3
     - Else → Continue to next feature
   - If NO → Continue conversing until aligned, then re-ask
   - Else → Ask user to answer Yes or No

## Phase 3 — Generate Feature Specifications

1. For each approved feature, create a file:
   - Path: docs/agile/features/<feature_num>-<feature_name>.md
2. Use the following template:

   ```markdown
   # <Feature N — Feature Name>

   ## Overview
   <Clear and concise description of the feature.>

   ## Scope
   - In Scope:
     - <item>
   - Out of Scope:
     - <item>

   ## Functional Requirements
   - Given <precondition>, When <action>, Then <outcome>.
   - Given <...>, When <...>, Then <...>.

   ## Non-Functional Requirements
   - <Performance/SLA/UX/Accessibility/Security/Scalability/etc.>

   ## Public API and Interfaces
   - <Endpoints/Interfaces/Events/Schemas>
   - <Parameters/Responses/Contracts>

   ## Dependencies
   - <Internal/External/Teams/Services/Feature dependencies>

   ## Risks & Mitigations
   - Risk: <description>
     - Mitigation: <strategy>

   ## Acceptance Criteria
   - <High-level success conditions indicating completion>
   ```

3. Phase 3 exit question (per feature):
   - “I have generated the Feature Specifications for each Feature agreed upon in Phase #2. Are these accurate, complete, and ready to move into the backlog for sprint planning?”

4. Branching logic:
   - If YES:
     - If last feature → Move to Phase 4
     - Else → Continue to the next feature
   - If NO → Revise that feature spec until approved

## Phase 4 — Generate User Stories

1. For each approved feature, generate stories (recommended path):
   - Path: docs/user-stories/<feature_num>-<feature_name>.md

2. Story template (repeat per story):

   ```markdown
   ## User Story <ID or short name>
   As a <role>, I want <capability/feature> so that <benefit/value>.

   ### Acceptance Criteria
   - Given <context>, When <action>, Then <result>.
   - Given <...>, When <...>, Then <...>.

   ### Notes/Dependencies (optional)
   - <Dependencies or sequencing from PRD or implementation plan>
   ```

3. Phase 4 exit question (per feature):
   - “I have generated the User Stories for each Feature Specification. Are these accurate, complete, and ready to move into the backlog for sprint planning?”

4. Branching logic:
   - If YES:
     - If last feature → End workflow
     - Else → Continue to the next feature
   - If NO → Revise stories for that feature until approved

## Tips
- Maintain strict alignment with `docs/planning/implementation-plan.md` and the PRD.
- Ensure every requirement is testable and unambiguous.
- Avoid adding scope beyond what’s explicitly required.
