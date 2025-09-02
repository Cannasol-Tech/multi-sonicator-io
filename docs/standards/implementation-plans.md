# Standard: Implementation Plans

## 1. Purpose

An Implementation Plan translates a user story's requirements into a concrete, actionable checklist for developers. It serves as a bridge between the *what* (the story's acceptance criteria) and the *how* (the specific development tasks).

Its primary goals are:

* **Clarity**: To ensure a shared understanding of the work required before coding begins.
* **Visibility**: To provide a visual representation of progress (via checkboxes).
* **Traceability**: To link development tasks directly to stories and architectural decisions.

## 2. Process

### When to Create an Implementation Plan

An implementation plan should be created for any user story that involves more than a few hours of work or touches multiple parts of the codebase. It should be drafted **after** the story has been groomed and **before** development work starts.

### How to Create and Use an Implementation Plan

1. **Create the File**: Copy the template from `docs/planning/implementation-plan.md` into a new file, typically within the relevant feature or epic directory (e.g., `docs/agile/epics/my-feature/implementation-plan.md`).

2. **Break Down the Work**: Under the `Work Breakdown` section, list the specific, small, and testable tasks required to complete the story. Use checkboxes for each task.

    ```markdown
    - [ ] Task 1: Create the Pydantic model for user state.
    - [ ] Task 2: Add the new state to the LangGraph workflow.
    - [ ] Task 3: Write unit tests for the new model.
    ```

3. **Update as You Go**: As you complete each task, check the box in the implementation plan. This provides a real-time, visual status of your progress.

## 3. Implementation Plans vs. Story Checklists

While a simple checklist in a story is good for trivial tasks, a dedicated implementation plan is better for complex stories because it provides more context, including links to architecture, risks, and testing strategies. The visual nature of the checkboxes in a dedicated Markdown file is also a key benefit for tracking progress.
